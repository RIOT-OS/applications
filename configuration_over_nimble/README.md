## About

This application provides a simple Bluetooth LE service (via RIOT's
NimBLE port) for device configuration purposes. The included bluetooth
characteristics are taken from an
[application](https://github.com/HendrikVE/smarthome2/tree/master/ESP32/window_alert_riot) for
a sensor node, which is placed in a specific room and transmits data to
a MQTT-SN gateway.
The sent values ​​are stored persistently in MCU's internal flash via
RIOT's flashpage API.

There is an Android App specifically written for this application.
Either build the app yourself
[from source](https://github.com/HendrikVE/smarthome2/tree/master/AndroidApp/WindowAlarmConfig),
download it from
[here](https://play.google.com/store/apps/details?id=de.vanappsteer.windowalarmconfig)
or use Nordics "nRF Connect"-App, available for
[Android](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp)
and
[iOS](https://itunes.apple.com/us/app/nrf-connect/id1054362403).

In the case you want to use Nordics app, you will need to write an empty string to the
characteristic "890f7b6f-cecc-4e3e-ade2-5f2907867f4b" in the end of the configuration process
to store the data permanently and to restart the device afterwards.

Currently the application is only running on *nrf52dk* and *nrf52840dk* boards.

## Clean install

Add `JLINK_PRE_FLASH=erase` to your make command to erase the flash memory.
That way you avoid data left in the storage, especially on the flash page
used for configuration.
