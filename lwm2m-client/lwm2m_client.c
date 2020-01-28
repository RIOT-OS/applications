/*
 * Copyright (c) 2020 Ken Bannister. All rights reserved.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 * @ingroup     lwm2m_client
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
#include "lwm2m_client.h"

#define _REG_PAYLOAD  "</>;rt=\"oma.lwm2m\",</1/0>,</3/0>"

typedef struct {
    unsigned state;
    sock_udp_ep_t server;
} lwm2m_client_t;

static lwm2m_client_t _client = {
    .state = LWM2M_STATE_INIT,
};

/*
 * Initialize server address/endpoint.
 *
 * @return 0 on success
 * @return <0 if can't create address
 */
static int _init(void) {
    ipv6_addr_t addr;

    _client.server.family = AF_INET6;

    /* parse for interface */
    char *iface = ipv6_addr_split_iface(LWM2M_SERVER_ADDR);
    if (!iface) {
        if (gnrc_netif_numof() == 1) {
            /* assign the single interface found in gnrc_netif_numof() */
            _client.server.netif = (uint16_t)gnrc_netif_iter(NULL)->pid;
        }
        else {
            _client.server.netif = SOCK_ADDR_ANY_NETIF;
        }
    }
    else {
        int pid = atoi(iface);
        if (gnrc_netif_get_by_pid(pid) == NULL) {
            DEBUG("lwm2m: interface not valid\n");
            return -1;
        }
        _client.server.netif = pid;
    }
    /* parse destination address */
    if (ipv6_addr_from_str(&addr, LWM2M_SERVER_ADDR) == NULL) {
        DEBUG("lwm2m: unable to parse destination address\n");
        return -1;
    }
    if ((_client.server.netif == SOCK_ADDR_ANY_NETIF)
            && ipv6_addr_is_link_local(&addr)) {
        DEBUG("lwm2m: must specify interface for link local target\n");
        return -1;
    }
    memcpy(&_client.server.addr.ipv6[0], &addr.u8[0], sizeof(addr.u8));

    _client.server.port = GCOAP_PORT;
    return 0;
}

/*
 * Registration callback.
 */
static void _reg_handler(const gcoap_request_memo_t *memo, coap_pkt_t* pdu,
                         const sock_udp_ep_t *remote)
{
    (void)pdu;
    (void)remote;

    if (memo->state == GCOAP_MEMO_TIMEOUT) {
        DEBUG("lwm2m: registration timed out\n");
        _client.state = LWM2M_STATE_REG_FAIL;
    }
    else if (memo->state == GCOAP_MEMO_ERR) {
        DEBUG("lwm2m: registration error\n");
        _client.state = LWM2M_STATE_REG_FAIL;
    }
    _client.state = LWM2M_STATE_REG_OK;
}


int lwm2m_client_start(void) {
    if (_client.state != LWM2M_STATE_INIT
            && _client.state != LWM2M_STATE_REG_FAIL) {
        return -EALREADY;
    }
    if (_init() < 0) {
        return -LWM2M_ERROR;
    }

    coap_pkt_t pdu;
    uint8_t buf[GCOAP_PDU_BUF_SIZE];

    gcoap_req_init(&pdu, buf, GCOAP_PDU_BUF_SIZE, COAP_METHOD_POST, "/rd");
    coap_hdr_set_type(pdu.hdr, COAP_TYPE_CON);
    coap_opt_add_format(&pdu, COAP_FORMAT_LINK);
    gcoap_add_qstring(&pdu, "lwm2m", "1.0");
    gcoap_add_qstring(&pdu, "ep", "RIOTclient");
    gcoap_add_qstring(&pdu, "lt", "300");
    int len = coap_opt_finish(&pdu, COAP_OPT_FINISH_PAYLOAD);
    if (len + strlen(_REG_PAYLOAD) < GCOAP_PDU_BUF_SIZE) {
        memcpy(pdu.payload, _REG_PAYLOAD, strlen(_REG_PAYLOAD));
        len += strlen(_REG_PAYLOAD);
    }
    else {
        return -ECOMM;
    }

    if (!gcoap_req_send(buf, len, &_client.server, _reg_handler, NULL)) {
        return -ECOMM;
    }
    _client.state = LWM2M_STATE_REG_SENT;
    return 0;
}

int lwm2m_client_state(void) {
    return _client.state;
}
