/*
 * Copyright (c) 2020 Ken Bannister. All rights reserved.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup     app_lwm2m_client
 * @{
 *
 * @file
 * @brief       RIOT native LwM2M client
 *
 * @author      Ken Bannister <kb2ma@runbox.com>
 * @}
 */

#include <stdio.h>
#include <string.h>

#include "msg.h"
#include "shell.h"

#define SHELL_QUEUE_SIZE (4)
static msg_t _shell_queue[SHELL_QUEUE_SIZE];


static int _cli_cmd(int argc, char **argv)
{
    if (argc == 1) {
        goto help_error;
    }

    if (!strcmp(argv[1], "start")) {
        printf("nothing to see here\n");
        return 0;
    }

help_error:
    printf("usage: %s <start>\n", argv[0]);

    return 1;
}

static const shell_command_t my_commands[] = {
    { "lwm2m", "Start LwM2M client", _cli_cmd },
    { NULL, NULL, NULL }
};

int main(void)
{
    msg_init_queue(_shell_queue, SHELL_QUEUE_SIZE);
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(my_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
