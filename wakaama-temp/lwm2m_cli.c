/*
 * Copyright (C) 2019 HAW Hamburg
 * Copyright (c) 2019 Ken Bannister. All rights reserved.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     app_wakaama_temp
 * @{
 *
 * @file
 * @brief       CLI support for Wakaama app to read SAUL temperature sensor
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @}
 */

#include "kernel_defines.h"
#include "lwm2m_client.h"
#include "lwm2m_client_objects.h"
#include "lwm2m_platform.h"
#include "saul_reg.h"

#include "measured_temp.h"
#include "temp_reader.h"

#define OBJ_COUNT (4)

uint8_t connected = 0;
lwm2m_object_t *obj_list[OBJ_COUNT];
lwm2m_client_data_t client_data;

void lwm2m_cli_init(void)
{
    /* this call is needed before creating any objects */
    memset(&client_data, 0, sizeof(client_data));
    lwm2m_client_init(&client_data);

    /* add objects that will be registered */
    obj_list[0] = lwm2m_client_get_security_object(&client_data);
    obj_list[1] = lwm2m_client_get_server_object(&client_data);
    obj_list[2] = lwm2m_client_get_device_object(&client_data);
    obj_list[3] = lwm2m_object_measured_temp_get(1);

    if (!obj_list[0] || !obj_list[1] || !obj_list[2]) {
        puts("Could not create mandatory objects");
    }
}

int lwm2m_cli_cmd(int argc, char **argv)
{
    if (argc == 1) {
        goto help_error;
    }

    if (!strcmp(argv[1], "start")) {
        /* run the LwM2M client */
        if (!connected && lwm2m_client_run(&client_data, obj_list, OBJ_COUNT)) {
            connected = 1;
            // start the SAUL reader thread
            lwm2m_measured_temp_instance_t *instance = (lwm2m_measured_temp_instance_t *)
                                                       lwm2m_list_find(obj_list[3]->instanceList, 0);
            temp_reader_start(SAUL_DRIVER, client_data.lwm2m_ctx, instance);
        }
        return 0;
    }

    if (IS_ACTIVE(DEVELHELP) && !strcmp(argv[1],"mem")) {
        lwm2m_tlsf_status();
        return 0;
    }

help_error:
    if (IS_ACTIVE(DEVELHELP)) {
        printf("usage: %s <start|mem>\n", argv[0]);
    }
    else {
        printf("usage: %s <start>\n", argv[0]);
    }

    return 1;
}
