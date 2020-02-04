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
 * @brief       CLI for RIOT native LwM2M client
 *
 * @author      Ken Bannister <kb2ma@runbox.com>
 * @}
 */

#include <stdio.h>
#include <string.h>

#include "errno.h"
#include "msg.h"
#include "shell.h"
#include "thread.h"
#include "lwm2m_cli.h"
#include "lwm2m_client.h"

#define SHELL_QUEUE_SIZE (4)
static msg_t _msg_queue[SHELL_QUEUE_SIZE];
static char _stack[THREAD_STACKSIZE_DEFAULT];
static kernel_pid_t _pid = KERNEL_PID_UNDEF;
static kernel_pid_t _lwm2m_pid = KERNEL_PID_UNDEF;


static int _cli_cmd(int argc, char **argv)
{
    if (argc == 1) {
        goto help_error;
    }

    if (!strcmp(argv[1], "start")) {
        int ret = -1;
        if (lwm2m_client_state() == LWM2M_STATE_INIT) {
            ret = thread_wakeup(_lwm2m_pid);
        }
        if (ret != 1) {
            printf("Failed to start: %d\n", ret);
        }
        return 0;
    }
    else if (!strcmp(argv[1], "state")) {
        printf("Client state: %d\n", lwm2m_client_state());
        return 0;
    }

help_error:
    printf("usage: %s <start|state>\n", argv[0]);

    return 1;
}

static const shell_command_t my_commands[] = {
    { "lwm2m", "LwM2M client commands", _cli_cmd },
    { NULL, NULL, NULL }
};

static void *_run_shell(void *arg)
{
    (void)arg;

    msg_init_queue(_msg_queue, SHELL_QUEUE_SIZE);
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    /* loops forever */
    shell_run_once(my_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    _pid = KERNEL_PID_UNDEF;
    return 0;
}

int lwm2m_cli_start(kernel_pid_t lwm2m_pid)
{
    if (_pid != KERNEL_PID_UNDEF) {
        return -EALREADY;
    }
    _lwm2m_pid = lwm2m_pid;

    _pid = thread_create(_stack, sizeof(_stack),
                         THREAD_PRIORITY_MAIN+1, THREAD_CREATE_STACKTEST,
                         _run_shell, NULL, "shell");
    return 0;
}
