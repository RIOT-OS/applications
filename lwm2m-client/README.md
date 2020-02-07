# RIOT native LwM2M/SAUL temperature app

This application is a RIOT native LwM2M client to read a temperature sensor. It uses [gcoap](http://doc.riot-os.org/group__net__gcoap.html) for CoAP messaging and [SAUL](http://doc.riot-os.org/group__sys__saul__reg.html) to read a temperature sensor. The temperature value is available as a [measured temperature](http://www.openmobilealliance.org/tech/profiles/lwm2m/3303.xml) (/3303) object.

See the RIOT [JC42 driver](https://github.com/RIOT-OS/RIOT/tree/master/drivers/jc42) for a suitable SAUL temperature reader. The Microchip MCP9808 sensor supports this standard, and for example Adafruit provides a convenient [breakout board](https://www.adafruit.com/product/1782) for development.

The app also can be run with a "native" board without a SAUL sensor. In this case the temperature value simply increases with each measurement.


## Setup
The server address is defined in the Makefile as 'SERVER_ADDR'. The SAUL driver is defined as `SAUL_DRIVER`, and defaults to "jc42".

The measurement interval is defined in `saul_info_reporter.h` as `SAUL_INFO_INTERVAL`, and defaults to 60 seconds. 


## Compile and run

Review and modify `Makefile` as required. Then compile and run the client with:

```shell
    BOARD=<board> make clean all flash term
```

## Shell commands, etc.
- `lwm2m start`: Registers with the LwM2M server and starts taking temperature measurements
- `lwm2m state`: Debugging tool to show current application state

The temperature value resource `/3303/0/5700` may be read or observed by the LwM2M server.

## Debugging
Turn on DEBUG in `lwm2m_client.c` to follow state changes.
