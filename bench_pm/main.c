/*
 * Copyright (C) 2017-2018 SKF AB
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup tests
 * @{
 *
 * @file
 * @brief       Test for low power modes and wake up timings
 *
 * This test will switch to different low power modes and wait for wake events.
 *
 * @author      Joakim Nohlgård <joakim.nohlgard@eistec.se>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>

#include "cpu.h"
#include "board.h"
#include "periph_conf.h"
#include "periph/rtt.h"
#ifdef MODULE_PM_LAYERED
#include "pm_layered.h"
#endif
#ifdef MODULE_PERIPH_GPIO_IRQ
#include "periph/gpio.h"
#endif
#ifdef MODULE_PERIPH_LLWU
/* Kinetis specific low leakage mode handling */
#include "llwu.h"
#endif

#ifndef ENABLE_DEBUG
/* Enabling debug prints will affect timing measurements */
#define ENABLE_DEBUG        (0)
#endif
#include "debug.h"

#if COMA_MODE
/* For testing deepest low power modes without any interference */
#undef MODULE_PERIPH_GPIO_IRQ
#undef MODULE_PERIPH_RTT
#undef MODULE_PERIPH_LLWU
#undef LED0_ON
#undef LED1_ON
#endif

#ifdef MODULE_PERIPH_RTT
#define PRINT_RTT() (printf("RTT: %" PRIu32 "\n", rtt_get_counter()))
#else
#define PRINT_RTT()
#endif

#ifndef GPIO_WAKE_PIN
#ifdef BTN0_PIN
#define GPIO_WAKE_PIN       BTN0_PIN
#else
#error Missing GPIO_WAKE_PIN configuration
#endif
#endif

#ifdef MODULE_PERIPH_LLWU
/* Platform specific configuration for testing Kinetis low leakage wake up module (LLWU) */
#ifndef LLWU_WAKE_PIN
/* This macro should correspond to the pin used in the GPIO_WAKE_PIN macro */
#if defined(BOARD_FRDM_KW41Z)
#define LLWU_WAKE_PIN       LLWU_WAKEUP_PIN_PTC4
#elif defined(BOARD_FRDM_K22F)
#define LLWU_WAKE_PIN       LLWU_WAKEUP_PIN_PTC1
#elif defined(BOARD_FRDM_K64F)
#define LLWU_WAKE_PIN       LLWU_WAKEUP_PIN_PTC6
#endif
#endif /* LLWU_WAKE_PIN */
#endif /* MODULE_PERIPH_LLWU */

#ifndef TEST_PIN_ON
#ifdef LED0_ON
#define TEST_PIN_ON         LED0_ON
#define TEST_PIN_OFF        LED0_OFF
#else
#define TEST_PIN_ON
#define TEST_PIN_OFF
#endif
#endif

#ifndef TEST_ISR_PIN_ON
#ifdef LED1_ON
#define TEST_ISR_PIN_ON     LED1_ON
#define TEST_ISR_PIN_OFF    LED1_OFF
#else
#define TEST_ISR_PIN_ON
#define TEST_ISR_PIN_OFF
#endif
#endif

#ifdef MODULE_PERIPH_RTT
#define TICKS_TO_WAIT       (10 * RTT_FREQUENCY)
#endif /* MODULE_PERIPH_RTT */

static volatile unsigned busy = 0;

#ifdef MODULE_PERIPH_RTT
static void cb_rtt(void *arg)
{
    (void)arg;
    TEST_ISR_PIN_ON;
    DEBUG("RTT IRQ\n");

    busy = 0;
}
#endif /* MODULE_PERIPH_RTT */

#ifdef MODULE_PERIPH_LLWU
static void cb_llwu(void *arg)
{
    (void)arg;
    TEST_ISR_PIN_ON;
    DEBUG("llwu pin\n");

    busy = 0;
}
#endif /* MODULE_PERIPH_LLWU */

#ifdef MODULE_PERIPH_GPIO_IRQ
static void cb_gpio(void *arg)
{
    (void)arg;
    TEST_ISR_PIN_ON;
    DEBUG("gpio pin\n");

    busy = 0;
}
#endif /* MODULE_PERIPH_GPIO_IRQ */

int main(void)
{
    TEST_PIN_ON;
    puts("\nRIOT power consumption and wake timing test application");

#ifdef MODULE_PERIPH_LLWU
    /* Kinetis specific */
    puts("Enable LLWU wake up from RTC");
    llwu_wakeup_module_enable(LLWU_WAKEUP_MODULE_RTC_ALARM);
    puts("Enable LLWU IRQ on PTC4 (SW4) falling");
    gpio_init(GPIO_WAKE_PIN, GPIO_IN);
    llwu_wakeup_pin_set(LLWU_WAKE_PIN, LLWU_WAKEUP_EDGE_FALLING, cb_llwu, NULL);
#endif /* MODULE_PERIPH_LLWU */
#ifdef MODULE_PERIPH_GPIO_IRQ
    puts("Enable GPIO IRQ on PTC4 (SW4) falling");
    int res = gpio_init_int(GPIO_WAKE_PIN, GPIO_IN_PU, GPIO_FALLING, cb_gpio, NULL);
    if (res != 0) {
        printf("!! gpio_init_int: %d\n", res);
    }
#endif /* MODULE_PERIPH_GPIO_IRQ */

#ifdef MODULE_PERIPH_RTT
    puts("Initializing the RTT driver");
    rtt_init();
#endif /* MODULE_PERIPH_RTT */

#if ENABLE_CLKOUT
    /* Kinetis specific, clock monitor via CLKOUT on pin PTB3 */
    puts("Enable CLKOUT on PTB3");
    PORTB->PCR[3] = PORT_PCR_MUX(4);
    /* Select which clock to output */
    SIM->SOPT2 = (SIM->SOPT2 & ~SIM_SOPT2_CLKOUTSEL_MASK) | SIM_SOPT2_CLKOUTSEL(2);
    /* Use a logic analyzer or oscilloscope to look at the signal */
#endif

#if COMA_MODE
    puts("Coma mode, going to pm_set(0), not coming back");
    while (1) {
        pm_set(0);
        puts("woke up unexpectedly, going to pm_set(0)");
    }
#endif

    while (1) {
        PRINT_RTT();
        puts("Busy spin");
        {
#ifdef MODULE_PERIPH_RTT
            uint32_t rtt_target = rtt_get_counter() + TICKS_TO_WAIT;
            rtt_target &= RTT_MAX_VALUE;
            rtt_set_alarm(rtt_target, cb_rtt, 0);
#endif /* MODULE_PERIPH_RTT */
            busy = 1;
            TEST_PIN_OFF;
            TEST_ISR_PIN_OFF;
            while (busy) {
                __asm__ volatile ("" ::: "memory");
            }
            TEST_PIN_ON;
            PRINT_RTT();
        }
#ifdef MODULE_PM_LAYERED
        for (int k = PM_NUM_MODES; 0 <= k; --k) {
#ifdef MODULE_PERIPH_RTT
            uint32_t rtt_target = rtt_get_counter() + TICKS_TO_WAIT;
            rtt_target &= RTT_MAX_VALUE;
            rtt_set_alarm(rtt_target, cb_rtt, 0);
#endif /* MODULE_PERIPH_RTT */
            PRINT_RTT();
            printf("pm_set(%d)\n", k);
            TEST_PIN_OFF;
            TEST_ISR_PIN_OFF;
            pm_set((unsigned)k);
            TEST_PIN_ON;
            printf("wake from %d\n", k);
        }
#endif
    }
    return 0;
}
