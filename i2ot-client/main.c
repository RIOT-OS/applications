/*
 * Copyright (C) 2015 Lennart Dührsen <lennart.duehrsen@fu-berlin.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @brief       An example client for the i2ot gateway
 *
 * @author      Lennart Dührsen <lennart.duehrsen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "shell.h"

#include "coap.h"

#define MAIN_MSG_QUEUE_SIZE (1)

extern int i2ot_cmd(int argc, char **argv);

static const shell_command_t shell_commands[] = {
    { "i2ot-client", "send sensor data to an i2ot gateway", i2ot_cmd },
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("i2ot client demonstration");

    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;
}
