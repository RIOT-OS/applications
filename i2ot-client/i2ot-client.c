/*
 * Copyright (C) 2015 Lennart Dührsen <lennart.duehrsen@fu-berlin.de>
 */

/**
 * @{
 *
 * @file
 * @brief       i2ot client demonstration
 *
 * @author      Lennart Dührsen <lennart.duehrsen@fu-berlin.de>
 *
 * @}
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#include "debug.h"
#include "thread.h"
#include "random.h"
#include "coap.h"

#ifdef MODULE_XTIMER
#include "xtimer.h"
#endif

#define SERVER_MSG_QUEUE_SIZE   (8)
#define SERVER_BUFFER_SIZE      (64)

static int server_socket = -1;

static char server_buffer[SERVER_BUFFER_SIZE];

static char server_stack[THREAD_STACKSIZE_DEFAULT];

static int sender_socket = -1;

static char sender_stack[THREAD_STACKSIZE_DEFAULT];

static msg_t server_msg_queue[SERVER_MSG_QUEUE_SIZE];


// buffers for resource discovery
struct sockaddr_in6 server_ip;
socklen_t server_ip_len = sizeof(server_ip);
char recv_buf[1024];
char   *seg_p[8];
size_t seg_len[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
size_t num_segs = 0;

coap_option_t dest_uri_opts[8];


//  buffers for CoAP server
uint8_t pkt_buf[256];
uint8_t scratch_buf[256];
coap_rw_buffer_t scratch = { .p = scratch_buf, .len = 256 };
char res_type[128];

coap_endpoint_path_t path_well_known_core = { .count = 2, .elems = { ".well-known", "core" } };

coap_endpoint_path_t sensor_path = { .count = 2, .elems = { "sensors", NULL } };

static int handle_get_well_known_core(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt,
                                      coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo);

static int handle_sensor_get(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt,
                             coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo);

static int i2ot_client_init(char *rt);

static int read_sensor(void);

static int read_sensor(void)
{
    return 42;
}

const coap_endpoint_t endpoints[] = {
    { COAP_METHOD_GET,  handle_get_well_known_core, &path_well_known_core, "ct=40"},
    { COAP_METHOD_GET,  handle_sensor_get, &sensor_path, "ct=0"},
    { (coap_method_t)0, NULL, NULL, NULL }
};


static int handle_get_well_known_core(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt,
                                      coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    puts("handle_get_well_known_core got called");
    char rsp[1024];
    rsp[0] = '\0';
    strcat(rsp, "</sensors/");
    strcat(rsp, res_type);
    strcat(rsp, ">;rt=");
    strcat(rsp, res_type);

    return coap_make_pb_response(scratch, outpkt, (const uint8_t *)rsp, strlen(rsp),
                                 id_hi, id_lo, &inpkt->token, COAP_RSPCODE_CONTENT,
                                 COAP_CONTENTTYPE_APPLICATION_LINKFORMAT);
}


static int handle_sensor_get(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt,
                             coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    int val = read_sensor();

    return coap_make_pb_response(scratch, outpkt, (const uint8_t *)&val, sizeof(int),
                                 id_hi, id_lo, &inpkt->token, COAP_RSPCODE_CONTENT,
                                 COAP_CONTENTTYPE_APPLICATION_LINKFORMAT);
}


static void *coap_server_thread(void *args)
{
    struct sockaddr_in6 server_addr;
    uint16_t port;
    coap_packet_t inpkt;
    coap_packet_t outpkt;
    int bad_packet;
    size_t rsplen;

    msg_init_queue(server_msg_queue, SERVER_MSG_QUEUE_SIZE);

    server_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

    /* parse port */
    port = (uint16_t)atoi((char *)args);

    if (port == 0) {
        puts("ERROR: invalid port specified");
        return NULL;
    }

    server_addr.sin6_family = AF_INET6;

    memset(&server_addr.sin6_addr, 0, sizeof(server_addr.sin6_addr));

    server_addr.sin6_port = htons(port);

    if (server_socket < 0) {
        puts("ERROR initializing socket");
        server_socket = 0;
        return NULL;
    }

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        server_socket = -1;
        puts("ERROR binding socket");
        return NULL;
    }

    printf("Success: started UDP server on port %" PRIu16 "\n", port);

    while (1) {
        int res;
        struct sockaddr_in6 src;
        socklen_t src_len = sizeof(struct sockaddr_in6);

        res = recvfrom(server_socket, server_buffer, sizeof(server_buffer), 0,
                       (struct sockaddr *)&src, &src_len);

        if (res < 0) {
            puts("ERROR on receive");
            continue;
        }

        if (res == 0) {
            puts("Peer shut down");
            continue;
        }

        printf("Received data: ");
        puts(server_buffer);
        bad_packet = coap_parse(&inpkt, (uint8_t *)server_buffer, res);

        if (bad_packet) {
            puts("ERROR: malformed CoAP packet");
            continue;
        }

        // coap_dump_packet(&inpkt);

        bad_packet = coap_handle_req(&scratch, &inpkt, &outpkt, true, false);

        if (bad_packet) {
            puts("ERROR: coap_handle_req failed");
        }

        bad_packet = coap_build(pkt_buf, &rsplen, &outpkt);

        if (bad_packet) {
            printf("ERROR: could not build CoAP packet: %d\n", bad_packet);
            continue;
        }

        res = sendto(server_socket, (void *)&pkt_buf, rsplen, 0,
                     (struct sockaddr *)&src, src_len);

        if (res == -1) {
            puts("ERROR: could not send reply");
        }
    }

    return NULL;
}


static int coap_start_server(char *port_str)
{
    /* check if server is already running */
    if (server_socket >= 0) {
        puts("ERROR: server already running");
        return 1;
    }

    /* start server (which means registering pktdump for the chosen port) */
    if (thread_create(server_stack, sizeof(server_stack), THREAD_PRIORITY_MAIN - 1,
                      THREAD_CREATE_STACKTEST, coap_server_thread, port_str, "UDP server") <= KERNEL_PID_UNDEF) {
        server_socket = -1;
        puts("ERROR initializing thread");
        return 1;
    }

    return 0;
}


static void *i2ot_server_thread(void *args)
{
    (void)args;

    sender_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

    uint16_t mid_rand;
    uint32_t token_rand;
    int err;
    uint8_t pkt_buf[256];
    uint8_t rcv_buf[256];
    uint8_t payload[sizeof(int)];
    size_t pkt_buf_len = sizeof(pkt_buf);
    size_t rcv_buf_len = sizeof(rcv_buf);
    int value;

    coap_packet_t rcv_pkt;

    coap_header_t req_hdr = {
        .version        = 1,
        .type           = COAP_TYPE_NONCON,
        .tkllen         = 4,
        .code           = COAP_METHOD_POST,
        .mid            = { 0, 0 }      // will be overwritten in loop
    };

    coap_packet_t req_pkt = {
        .header         = req_hdr,
        .token          = { NULL, 0 }, // will be overwritten in loop
        .numopts        = num_segs,
        .opts           = { dest_uri_opts[0],dest_uri_opts[1], dest_uri_opts[2], dest_uri_opts[3] },
        .payload        = { .p = payload,.len = sizeof(int) }
    };

    int initval;

#ifdef MODULE_XTIMER
    initval = xtimer_now();
#else
    initval = 54321;
#endif

    random_init(initval);

    while (1) {
        pkt_buf_len = sizeof(pkt_buf);
        rcv_buf_len = sizeof(rcv_buf);

        mid_rand = (uint16_t)random_uint32();
        token_rand = random_uint32();

        req_hdr.mid[0] = ((uint8_t *)&mid_rand)[0];
        req_hdr.mid[1] = ((uint8_t *)&mid_rand)[1];
        req_pkt.token.p = (uint8_t *)&token_rand;
        req_pkt.token.len = 4;

        value = read_sensor();
        memcpy((void *)&payload, (void *)&value, sizeof(int));

        printf("sensor value: %d\n", value);

        err = coap_build(pkt_buf, &pkt_buf_len, &req_pkt);

        if (err != 0) {
            puts("ERROR: coap_build failed");
            goto sleep;
        }

        err = sendto(sender_socket, (void *)pkt_buf, pkt_buf_len, 0,
                     (struct sockaddr *)&server_ip, server_ip_len);

        if (err < 0) {
            puts("ERROR: sendto failed");
            goto sleep;
        }

        err = recvfrom(sender_socket, (void *)rcv_buf, rcv_buf_len, 0, NULL, 0);

        if (err < 0) {
            puts("ERROR: recvfrom failed");
            goto sleep;
        }

        err = coap_parse(&rcv_pkt, rcv_buf, err);

        if (err != 0) {
            puts("ERROR: malformed CoAP packet");
            goto sleep;
        }

        // coap_dump_packet(&rcv_pkt);

sleep:
        sleep(1);
    }

    return NULL;
}


static int i2ot_client_start(void)
{
    if (thread_create(sender_stack, sizeof(sender_stack), THREAD_PRIORITY_MAIN - 1,
                      THREAD_CREATE_STACKTEST, i2ot_server_thread, NULL, "UDP server") <= KERNEL_PID_UNDEF) {
        sender_socket = -1;
        puts("error initializing thread");
        return 1;
    }

    return 0;
}


static int i2ot_client_init(char *rt)
{
    int init_sock;
    char coap_mc_addr_str[] = "ff02::fd";
    struct sockaddr_in6 dst;
    struct sockaddr_in6 my_addr;
    socklen_t dst_len = sizeof(dst);
    char uri_query[strlen(rt) + 4];
    uint16_t mid;
    int err;
    coap_packet_t recv_pkt;
    size_t pkt_size;
    uint8_t pkt_buf[256];
    char link[512];

    init_sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

    if (init_sock < 0) {
        puts("ERROR: could not initialize socket");
        return 1;
    }

    memset((void *)&my_addr, 0, sizeof(my_addr));

    my_addr.sin6_family = AF_INET6;
    my_addr.sin6_port   = htons(11222);

    err = bind(init_sock, (struct sockaddr *)&my_addr, dst_len);

    if (err != 0) {
        puts("ERROR: bind failed");
        return 1;
    }

    strcpy(uri_query, "rt=");
    strcpy(&uri_query[3], rt);

    int initval;

#ifdef MODULE_XTIMER
    initval = xtimer_now();
#else
    initval = 61524;
#endif

    random_init(initval);

    mid = (uint16_t)random_uint32();

    coap_option_t opt_uri_path_01 = {
        .val    = { .p = (uint8_t *)".well-known", .len = strlen(".well-known") },
        .num    = COAP_OPTION_URI_PATH
    };

    coap_option_t opt_uri_path_02 = {
        .val    = { (uint8_t *)"core", strlen("core") },
        .num    = COAP_OPTION_URI_PATH
    };

    coap_option_t opt_uri_query = {
        .val    = { (uint8_t *)uri_query, strlen(uri_query) },
        .num    = COAP_OPTION_URI_QUERY
    };

    coap_header_t req_hdr = {
        .version        = 1,
        .type           = COAP_TYPE_NONCON,
        .tkllen         = 0,
        .code           = COAP_METHOD_GET,
        .mid            = { ((uint8_t *)&mid)[1],((uint8_t *)&mid)[0]    }
    };

    coap_packet_t req_pkt = {
        .header         = req_hdr,
        .token          = { NULL, 0 },
        .numopts        = 3,
        .opts           = { opt_uri_path_01,opt_uri_path_02, opt_uri_query },
        .payload        = { NULL, 0 }
    };

    pkt_size = sizeof(pkt_buf);

    err = coap_build(pkt_buf, &pkt_size, &req_pkt);

    if (err != 0) {
        puts("ERROR: coap_build failed");
        return 1;
    }

    memset((void *)&dst, 0, sizeof(dst));

    dst.sin6_family = AF_INET6;
    dst.sin6_port   = htons(5683);

    err = inet_pton(AF_INET6, coap_mc_addr_str, &dst.sin6_addr);

    if (err != 1) {
        puts("ERROR: inet_pton failed");
    }

    err = sendto(init_sock, (void *)pkt_buf, pkt_size, 0, (struct sockaddr *)&dst, dst_len);

    if (err < 0) {
        puts("ERROR: sendto failed");
        return 1;
    }

    puts("request for /.well-known/core sent, waiting for reply...");

    err = recvfrom(init_sock, (void *)recv_buf, 1023, 0,
                   (struct sockaddr *)&server_ip, &server_ip_len);

    if (err < 1) {
        puts("ERROR: recvfrom failed");
        printf("%s\n", strerror(errno));
        return 1;
    }

    err = coap_parse(&recv_pkt, (uint8_t *)recv_buf, err);

    if (err != 0) {
        puts("ERROR: coap_parse failed");
        return 1;
    }

    char   *payload     = (char *) &(recv_pkt.payload.p);
    size_t payload_len = recv_pkt.payload.len;

    char   *link_start;
    size_t link_size;

    char *p = payload;

    char *curr = (char *)(payload - p);

    unsigned int i = 0;

find_link:
    while (*p != '<') {
        if (curr >= (payload + payload_len)) {
            goto fail;
        }
        p++;
    }

    link_start = ++p;

    while (*p != '>') {
        if (curr >= (payload + payload_len)) {
            goto fail;
        }
        p++;
    }

    link_size = p - link_start;

    p += 2;

    if (curr >= (payload + payload_len)) {
        goto fail;
    }

    if (strncmp(p, rt, strlen(rt)) == 0) {
        strncpy(link, link_start, link_size);
        link[link_size] = '\0';
        return 0;
    }
    else {
        while (*p != ',') {
            if (curr >= (payload + payload_len)) {
                goto fail;
            }
            p++;
        }

        goto find_link;
    }

    while (i < link_size) {
        if (payload[i] == '/') {
            seg_p[num_segs++] = &payload[i + 1];
        }
        else if (payload[i] == '>') {
            break;
        }
        else {
            seg_len[num_segs - 1]++;
        }

        i++;
    }

    for (unsigned int j = 0; j < num_segs; j++) {
        dest_uri_opts[j].val.p   = (uint8_t *)seg_p[j];
        dest_uri_opts[j].val.len = seg_len[j];
        dest_uri_opts[j].num     = COAP_OPTION_URI_PATH;
    }

    puts("found link: ");

    for (unsigned int k = 0; k < link_size; k++)
        putchar(link[k]);

    return 0;

fail:
    link[0] = '\0';
    puts("ERROR: no suitable link found");

    return 1;
}


int i2ot_cmd(int argc, char **argv)
{
    if (argc < 3) {
        printf("usage: %s [run [resourcetype]]\n", argv[0]);
        return 1;
    }

    if (argc == 3 && strcmp(argv[1], "run") == 0) {
        strcpy(res_type, argv[2]);
        sensor_path.elems[1] = res_type;

        if (i2ot_client_init(res_type) != 0) {
            puts("ERROR: i2ot_client_init failed");
            return 1;
        }

        i2ot_client_start();

        coap_start_server("5683");
        return 0;
    }
    else {
        puts("ERROR: invalid command");
        return 1;
    }

    return 0;
}

/** @} */
