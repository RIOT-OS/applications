Purpose
=======

This project is used to provide a baseline for the power consumption of the
platform when no software is interfering with the low power modes. The
application is designed to be used as a test subject for measuring power
consumption and timing of low power modes.

Hardware requirements
=====================

The data collection relies on external measurement equipment. An oscilloscope,
or logic analyzer and a multimeter is required for any meaningful measurements.

Test outputs
============

The test uses the board's LED0, LED1 pins as feedback from the application to
the logic analyzer to signal when a low power mode is exited.
The wake up request is triggered by UART RX activity, or a configurable GPIO pin.

Set up
======

Connect the logic analyzer to trigger on the chosen GPIO wake pin (or UART RX
pin, if not using the GPIO), connect the LED0 pin to the logic analyzer as well.

The LED0 pin will be asserted immediately when control returns to the main thread.
The LED1 pin is asserted by the ISR.

Tested hardware
===============

The application is designed to run on the FRDM-KW41Z (or a custom R41Z board),
but some care has been taken to allow building for different platforms without
any major changes.

Future work
===========

The logic analyzer requirement could be eliminated by creating a measurement
application which can run on a different board for driving the test and taking
measurements.
