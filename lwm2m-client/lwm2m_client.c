/*
 * Copyright (c) 2020 Ken Bannister. All rights reserved.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     app_lwm2m_client
 * @{
 *
 * @file
 * @brief       RIOT native LwM2M client
 *
 * @author      Ken Bannister <kb2ma@runbox.com>
 * @}
 */

#define ENABLE_DEBUG    (0)
#include "debug.h"

#include "net/gcoap.h"
#include "fmt.h"
#include "lwm2m_cli.h"
#include "lwm2m_client.h"
#include "phydat.h"
#include "saul_info_reporter.h"
#include "thread.h"
#include "timex.h"

#define _REG_PAYLOAD  "</>;rt=\"oma.lwm2m\",</1/0>,</3/0>,</3303/0>"

static ssize_t _temp_req_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len,
                                 void *ctx);

/*
 * Client internal variables
 */
typedef struct {
    int state;                    /* current state, a LWM2M_STATE_* macro */
    uint32_t next_reg_time;       /* time for next registration, in seconds */
    uint32_t next_info_time;      /* time for next SAUL information report,
                                   * in seconds */
    sock_udp_ep_t server_ep;      /* device management server endpoint */
    char server_location[LWM2M_REG_LOCATION_MAXLEN];
                                  /* registration location provided by server,
                                   * as a null terminated string */
} lwm2m_client_t;

static lwm2m_client_t _client = {
    .state = LWM2M_STATE_INIT,
    .next_reg_time = 0,
    .next_info_time = 0,
    .server_location = { '\0' }
};

/* CoAP resources. Must be sorted by path (ASCII order). */
static const coap_resource_t _resources[] = {
    { "/3303", COAP_GET | COAP_MATCH_SUBTREE, _temp_req_handler, NULL },
};

static gcoap_listener_t _listener = {
    &_resources[0],
    ARRAY_SIZE(_resources),
    NULL, NULL
};

/*
 * Registration response callback
 */
static void _reg_handler(const gcoap_request_memo_t *memo, coap_pkt_t* pdu,
                         const sock_udp_ep_t *remote)
{
    (void)pdu;
    (void)remote;

    if (memo->state != GCOAP_MEMO_RESP) {
        DEBUG("lwm2m: registration failed: %d\n", memo->state);
        _client.state = LWM2M_STATE_REG_FAIL;
        return;
    }

    /* record initial registration location */
    if (!strlen(_client.server_location)) {
        ssize_t len = coap_get_location_path(pdu, (uint8_t*)_client.server_location,
                                             LWM2M_REG_LOCATION_MAXLEN);
        if (len == -ENOSPC) {
            DEBUG("lwmwm: location path too long\n");
            _client.state = LWM2M_STATE_REG_FAIL;
            return;
        }
        else {
            DEBUG("lwm2m: reg location %s\n", _client.server_location);
        }
    }

    timex_t time;
    xtimer_now_timex(&time);
    /* re-registration interval less confirmable msg lifetime */
    _client.next_reg_time = time.seconds + (LWM2M_REG_INTERVAL - 90);
    _client.state = LWM2M_STATE_REG_OK;
}

/*
 * Temperature (/3303) request callback
 */
static ssize_t _temp_req_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len,
                                 void *ctx)
{
    (void)ctx;
    coap_optpos_t opt = {0, 0};
    uint8_t *opt_val;
    unsigned path_i = 0;
    bool is_value_req = false;

    /* verify request is for /3303/0/5700 */
    ssize_t optlen;
    while ((optlen = coap_opt_get_next(pdu, &opt, &opt_val, !opt.opt_num)) >= 0) {
        if (opt.opt_num == COAP_OPT_URI_PATH) {
            if (path_i == 2 && optlen == 4 && !strncmp((char *)opt_val, "5700", 4)) {
                is_value_req = true;
                break;
            }
            path_i++;
        }
    }

    /* write response */
    if (is_value_req) {
        gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
        coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
        size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

        if (pdu->payload_len >= 10) {
            phydat_t temp_dat;
            saul_info_value(&temp_dat);
            ssize_t len = saul_info_print(temp_dat.val[0], temp_dat.scale,
                                          (char *)pdu->payload, 10);
            if (len > 0) {
                resp_len += len;
            }
            else {
                DEBUG("lwm2m: buffer too small\n");
                return -1;
            }
            return resp_len;
        }
        else {
            DEBUG("lwm2m: msg buffer too small\n");
            return -1;
        }
    }
    else {
        DEBUG("lwm2m: request not for 5700\n");
        return -1;
    }
}

int lwm2m_client_state(void)
{
    return _client.state;
}

/*
 * Initialize server address/endpoint from LWM2M_SERVER_ADDR.
 *
 * @return 0 on success
 * @return <0 if can't create address
 */
static int _init_server_addr(void)
{
    _client.server_ep.family = AF_INET6;

    /* parse for interface specifier, like %1 */
    char *iface = ipv6_addr_split_iface(LWM2M_SERVER_ADDR);
    if (!iface) {
        if (gnrc_netif_numof() == 1) {
            /* assign the single interface found in gnrc_netif_numof() */
            _client.server_ep.netif = (uint16_t)gnrc_netif_iter(NULL)->pid;
        }
        else {
            _client.server_ep.netif = SOCK_ADDR_ANY_NETIF;
        }
    }
    else {
        int pid = atoi(iface);
        if (gnrc_netif_get_by_pid(pid) == NULL) {
            DEBUG("lwm2m: interface not valid\n");
            return -1;
        }
        _client.server_ep.netif = pid;
    }

    /* build address */
    ipv6_addr_t addr;
    if (ipv6_addr_from_str(&addr, LWM2M_SERVER_ADDR) == NULL) {
        DEBUG("lwm2m: unable to parse destination address\n");
        return -1;
    }
    if ((_client.server_ep.netif == SOCK_ADDR_ANY_NETIF)
            && ipv6_addr_is_link_local(&addr)) {
        DEBUG("lwm2m: must specify interface for link local target\n");
        return -1;
    }
    memcpy(&_client.server_ep.addr.ipv6[0], &addr.u8[0], sizeof(addr.u8));

    _client.server_ep.port = GCOAP_PORT;

    return 0;
}

/*
 * Register with LwM2M device management server.
 */
static void _register(void)
{
    coap_pkt_t pdu;
    uint8_t buf[GCOAP_PDU_BUF_SIZE];
    int len;

    if (_client.state == LWM2M_STATE_REG_RENEW) {
        gcoap_req_init(&pdu, buf, GCOAP_PDU_BUF_SIZE, COAP_METHOD_POST,
                      &_client.server_location[0]);
        coap_hdr_set_type(pdu.hdr, COAP_TYPE_CON);
        len = coap_opt_finish(&pdu, COAP_OPT_FINISH_NONE);
    }
    else {
        /* initial registration */
        char interval[8];
        memset(interval, 0, 8);
        fmt_u32_dec(interval, LWM2M_REG_INTERVAL);
        
        gcoap_req_init(&pdu, buf, GCOAP_PDU_BUF_SIZE, COAP_METHOD_POST,
                      "/rd");
        coap_hdr_set_type(pdu.hdr, COAP_TYPE_CON);
        coap_opt_add_format(&pdu, COAP_FORMAT_LINK);
        gcoap_add_qstring(&pdu, "lwm2m", "1.0");
        gcoap_add_qstring(&pdu, "ep", "RIOTclient");
        gcoap_add_qstring(&pdu, "lt", interval);
        len = coap_opt_finish(&pdu, COAP_OPT_FINISH_PAYLOAD);
        if (len + strlen(_REG_PAYLOAD) < GCOAP_PDU_BUF_SIZE) {
            memcpy(pdu.payload, _REG_PAYLOAD, strlen(_REG_PAYLOAD));
            len += strlen(_REG_PAYLOAD);
        }
        else {
            _client.state = LWM2M_STATE_REG_FAIL;
            return;
        }
    }

    if (!gcoap_req_send(buf, len, &_client.server_ep, _reg_handler, NULL)) {
        _client.state = LWM2M_STATE_REG_FAIL;
        return;
    }
    _client.state = LWM2M_STATE_REG_SENT;
}

/*
 * Initialize and maintain application state loop.
 */
int main(void)
{
    timex_t time;

    gcoap_register_listener(&_listener);
    lwm2m_cli_start(thread_getpid());

    DEBUG("lwm2m: waiting for start from CLI\n");
    thread_sleep();

    if (_init_server_addr() < 0) {
        _client.state = LWM2M_STATE_INIT_FAIL;
    }

    while (1) {
        xtimer_now_timex(&time);
        DEBUG("lwm2m: state %d @ %u\n", _client.state, (unsigned)time.seconds);

        switch (_client.state) {
        case LWM2M_STATE_INIT:
        case LWM2M_STATE_REG_RENEW:
            _register();
            break;

        case LWM2M_STATE_INFO_RENEW:
            saul_info_send();
            /* assume we don't wait for confirmation */
            _client.next_info_time = time.seconds + SAUL_INFO_INTERVAL;
            _client.state = LWM2M_STATE_INFO_OK;
            break;

        case LWM2M_STATE_REG_OK:
        case LWM2M_STATE_INFO_OK:
            if (_client.next_info_time == 0) {
                saul_info_init(SAUL_INFO_DRIVER, &_resources[0]);
                _client.next_info_time = time.seconds + SAUL_INFO_INTERVAL;
            }
            if (_client.next_info_time < _client.next_reg_time) {
                DEBUG("lwm2m: sleeping for %u\n",
                      (unsigned) (_client.next_info_time - time.seconds));
                xtimer_sleep(_client.next_info_time - time.seconds);
                _client.state = LWM2M_STATE_INFO_RENEW;
            }
            else {
                DEBUG("lwm2m: sleeping for %u\n",
                      (unsigned) (_client.next_reg_time - time.seconds));
                xtimer_sleep(_client.next_reg_time - time.seconds);
                _client.state = LWM2M_STATE_REG_RENEW;
            }
            break;

        default:
            /* expecting an event, or has a terminal error */
            xtimer_sleep(1);
        }
    }
    return 0;
}
