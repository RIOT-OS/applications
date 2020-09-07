/*
 * Copyright (C) 2015-19 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    app_sniffer
 * @brief       Sniffer application based on the new network stack
 * @{
 *
 * @file
 * @brief       Sniffer application for RIOT
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Martine Lenders <m.lenders@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "byteorder.h"
#include "isrpipe.h"
#include "net/gnrc.h"
#include "net/netopt.h"
#include "periph/uart.h"
#include "stdio_uart.h"
#include "xtimer.h"

/**
 * @brief   Priority of the RAW dump thread
 */
#define RAWDUMP_PRIO            (THREAD_PRIORITY_MAIN - 1)

/**
 * @brief   Message queue size of the RAW dump thread
 */
#define RAWDUMP_MSG_Q_SIZE      (32U)

#define SET_CHAN_MSG_TYPE       (0xdd4a)

#define END_BYTE                (0xc0U)
#define ESC_BYTE                (0xdbU)
#define END_ESC_BYTE            (0xdcU)
#define ESC_ESC_BYTE            (0xddU)

#ifndef SNIFFER_DEV
#define SNIFFER_DEV             (UART_DEV(0))
#endif

#ifndef SNIFFER_BAUDRATE
#define SNIFFER_BAUDRATE        (STDIO_UART_BAUDRATE)
#endif

static msg_t msg_q[RAWDUMP_MSG_Q_SIZE];
static kernel_pid_t _main_pid = KERNEL_PID_UNDEF;

static void _rx_uart(void *arg, uint8_t data)
{
    /* XXX: read one byte channel. More sync is required for larger channel
     * ranges */
    msg_t msg = { .type = SET_CHAN_MSG_TYPE, .content = { .value = data } };

    (void)arg;
    msg_send_int(&msg, _main_pid);
}

static void _tx_byte(uint8_t data)
{
    uart_write(SNIFFER_DEV, &data, sizeof(data));
}

static void _tx_bytes(uint8_t *data, size_t len)
{
    for (unsigned i = 0; i < len; i++) {
        switch (*data) {
            case END_BYTE:
                _tx_byte(ESC_BYTE);
                _tx_byte(END_ESC_BYTE);
                break;
            case ESC_BYTE:
                _tx_byte(ESC_BYTE);
                _tx_byte(ESC_ESC_BYTE);
                break;
            default:
                _tx_byte(*data);
                break;
        }
        data++;
    }
}

/**
 * @brief   Make a raw dump of the given packet contents
 */
static void _dump_pkt(gnrc_pktsnip_t *pkt)
{
    network_uint64_t now_us = byteorder_htonll(xtimer_now_usec64());
    gnrc_pktsnip_t *snip = pkt;
    network_uint32_t len;
    uint8_t lqi = 0;
    uint8_t padding[3] = { 0, 0, 0 };

    if (pkt->next) {
        if (pkt->next->type == GNRC_NETTYPE_NETIF) {
            gnrc_netif_hdr_t *netif_hdr = pkt->next->data;
            lqi = netif_hdr->lqi;
            pkt = gnrc_pktbuf_remove_snip(pkt, pkt->next);
        }
    }

    len = byteorder_htonl((uint32_t)gnrc_pkt_len(pkt));
    /*
     * write packet "header" in network byte order:
     *
     *  0                   1                   2                   3
     *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |                         packet length                         |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |      LQI      |                    padding                    |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |                                                               |
     * +                       Timestamp (in us)                       +
     * |                                                               |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     */
    _tx_bytes(len.u8, sizeof(len));
    _tx_bytes(&lqi, sizeof(lqi));
    _tx_bytes(padding, sizeof(padding));
    _tx_bytes(now_us.u8, sizeof(now_us));
    while (snip) {
        _tx_bytes(snip->data, snip->size);
        snip = snip->next;
    }
    _tx_byte(END_BYTE);

    gnrc_pktbuf_release(pkt);
}

static void _init_uart(void)
{
    int res;

    (void)res;  /* in case assert is not compiled in */
    uart_init(SNIFFER_DEV, SNIFFER_BAUDRATE, _rx_uart, NULL);
    assert(res == UART_OK);
}

static void _init_netif(gnrc_netif_t *netif)
{
    int res;
    netopt_enable_t enable = NETOPT_ENABLE;

    (void)res;  /* in case assert is not compiled in */
    res = gnrc_netapi_set(netif->pid, NETOPT_RAWMODE, 0,
                          &enable, sizeof(enable));
    assert(res >= 0);
    res = gnrc_netapi_set(netif->pid, NETOPT_PROMISCUOUSMODE, 0,
                          &enable, sizeof(enable));
    assert(res >= 0);
}

/**
 * @brief   Maybe you are a golfer?!
 */
int main(void)
{
    gnrc_netif_t *netif = gnrc_netif_iter(NULL);
    gnrc_netreg_entry_t dump;

    assert(netif != NULL);
    _main_pid = sched_active_pid;
    msg_init_queue(msg_q, RAWDUMP_MSG_Q_SIZE);
    _init_uart();

    /* start and register main thread for dumping */
    dump.target.pid = _main_pid;
    dump.demux_ctx = GNRC_NETREG_DEMUX_CTX_ALL;
    gnrc_netreg_register(GNRC_NETTYPE_UNDEF, &dump);

    _init_netif(netif);

    /* write empty packet to signal start */
    _tx_byte(END_BYTE);
    while (1) {
        msg_t msg;

        msg_receive(&msg);
        switch (msg.type) {
            case GNRC_NETAPI_MSG_TYPE_RCV:
                _dump_pkt((gnrc_pktsnip_t *)msg.content.ptr);
                break;
            case SET_CHAN_MSG_TYPE: {
                uint8_t chan = msg.content.value;
                gnrc_netapi_set(netif->pid, NETOPT_CHANNEL, 0, &chan,
                                sizeof(chan));
                break;
            }
            default:
                /* do nothing */
                break;
        }
    }

    return 0;
}
