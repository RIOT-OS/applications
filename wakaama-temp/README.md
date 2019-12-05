# LwM2M/SAUL temperature app

This application extends the RIOT LwM2M [Wakaama example](https://github.com/RIOT-OS/RIOT/tree/master/examples/wakaama) to periodically sample a SAUL temperature sensor and make the value available as a [measured temperature](http://www.openmobilealliance.org/tech/profiles/lwm2m/3303.xml) (/3303) object.

See the RIOT [JC42 driver](https://github.com/RIOT-OS/RIOT/tree/master/drivers/jc42) for a suitable SAUL temperature reader. The Microchip MCP9808 sensor supports this standard, and for example Adafruit provides a convenient [breakout board](https://www.adafruit.com/product/1782) for development.

The app also can be run with a "native" board without a SAUL sensor. In this case the temperature value simply increases with each measurement.


## Setup
The app reads the temperature in a separate thread from LwM2M. The measurement interval is defined in the app as `TEMP_READER_INTERVAL`, and defaults to 60 seconds.

The SAUL driver is defined in the Makefile as `SAUL_DRIVER`, and defaults to "jc42".

See the Wakaama example for details on LwM2M client and server setup.


## Compile and run

Review and modify `Makefile` as required. Then compile and run the client with:

```shell
    BOARD=<board> make clean all flash term
```

## Shell commands
- `lwm2m start`: Registers with the LwM2M server and starts taking temperature measurements
- `lwm2m mem`: Debugging tool to show memory use by Wakaama
