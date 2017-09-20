# SAM-M8Q GNSS FeatherWing

A clone of the [Adafruit Ultimate GPS FeatherWing](https://www.adafruit.com/product/3133) but with the u-blox SAM-M8Q replacing the GlobalTop FGPMMOPA6H. The SAM-M8Q can receive signals from GPS, GLONASS and Galileo concurrently and supports both SBAS and QZSS.

![SAM-M8Q_FeatherWing](https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing/blob/master/img/SAM-M8Q_FeatherWing.jpg)

See [SAM-M8Q_GPS_FeatherWing.pdf](https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing/blob/master/SAM-M8Q_GPS_FeatherWing.pdf) for the schematic, layout and Bill Of Materials.

The [Eagle](https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing/tree/master/Eagle) directory contains the schematic and pcb design files.

The [Arduino](https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing/tree/master/Arduino) directory contains code for the [Adafruit Feather M0 Adalogger](https://www.adafruit.com/products/2796) which will log your route to SD card in GPX and CSV format:
- https://www.adafruit.com/products/2796
- https://learn.adafruit.com/adafruit-feather-m0-adalogger

![SAM-M8Q_and_Adalogger_1](https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing/blob/master/img/SAM-M8Q_and_Adalogger_1.jpg)

![SAM-M8Q_and_Adalogger_2](https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing/blob/master/img/SAM-M8Q_and_Adalogger_2.jpg)

This guide concentrates on the differences between the SAM-M8Q FeatherWing and the Adafruit Ultimate GPS FeatherWing. Please refer to Lady Ada's excellent documentation to get started with the Feather family and GPS:
- https://www.adafruit.com/product/3133
- https://learn.adafruit.com/adafruit-ultimate-gps-featherwing

## Power Pins

Like the Ultimate GPS FeatherWing, the SAM-M8Q FeatherWing runs from +3.3V power and uses 3V logic. The SAM-M8Q is powered directly from the 3V and GND pins on the bottom left of the Feather. Each Adafruit Feather has a regulator to provide clean 3V power from USB or battery power.

![SAM-M8Q_FeatherWing_PowerTop](https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing/blob/master/img/SAM-M8Q_FeatherWing_PowerTop.jpg)

![SAM-M8Q_FeatherWing_PowerBottom](https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing/blob/master/img/SAM-M8Q_FeatherWing_PowerBottom.jpg)

If you want to completely power down the Adalogger _and_ the SAM-M8Q, you can do this by pulling the Feather EN pin low (e.g. by connecting it to GND via a small slide switch).

![SAM-M8Q_FeatherWing_EN](https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing/blob/master/img/SAM-M8Q_FeatherWing_EN.jpg)

## Serial Data Pins

![SAM-M8Q_FeatherWing_TxRxTop](https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing/blob/master/img/SAM-M8Q_FeatherWing_TxRxTop.jpg)

The SAM-M8Q FeatherWing, like the Ultimate GPS FeatherWing, communicates over UART serial. It sends ASCII NMEA sentences from the TX pin to the microcontroller RX pin and can be controlled to change its data output from the microcontroller TX pin. Logic level is 3V for both.

The u-box M8 chipset also supports much more comprehensive UBX binary format messages (see below).

The baud rate by default is 9600 baud, but you can configure the module to use a different baud rate if desired.

The SAM-M8Q Tx and Rx pins are wired directly to the Serial pins at the bottom right of the Feather.
If you need to connect to a different set of pins, you can cut the RX and TX jumpers on the bottom of the board and connect instead to the TX and RX pads near the TP LED.

![SAM-M8Q_FeatherWing_TxRxBottom](https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing/blob/master/img/SAM-M8Q_FeatherWing_TxRxBottom.jpg)

The ATSAMD21G18 ARM Cortex M0 chip on the Adalogger has multiple Sercom channels which can be used to provide additional serial ports, which is handy if you're already using Serial1 for something else:
- Adafruit: https://learn.adafruit.com/using-atsamd21-sercom-to-add-more-spi-i2c-serial-ports/creating-a-new-serial
- MartinL: https://forum.arduino.cc/index.php?topic=341054.msg2443086#msg2443086

## Reset Button

![SAM-M8Q_FeatherWing_ResetTop](https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing/blob/master/img/SAM-M8Q_FeatherWing_ResetTop.jpg)

![SAM-M8Q_FeatherWing_ResetBottom](https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing/blob/master/img/SAM-M8Q_FeatherWing_ResetBottom.jpg)

There is a small button that will connect the microcontroller RESET pin to ground for resetting it. Handy to restart the Feather. Note that this is not connected to SAM-M8Q reset unless you short the GRESET split pad on the rear of the PCB (see below).

## Breakout Pins

![SAM-M8Q_FeatherWing_BreakoutPins](https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing/blob/master/img/SAM-M8Q_FeatherWing_BreakoutPins.jpg)

**GPS Reset** is connected to the SAM-M8Q reset pin. You can reset the SAM-M8Q by pulling this pin low. If you want to reset both the Feather _and_ the SAM-M8Q via the reset button, short the GRESET split pad on the rear of the PCB.
Note that pulling GPS Reset low does not put the SAM-M8Q into a low power state, you'll want to disable it instead (see En below).

![SAM-M8Q_FeatherWing_GReset](https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing/blob/master/img/SAM-M8Q_FeatherWing_GReset.jpg)

**INT** is connected to the SAM-M8Q EXTINT external interrupt pin. It can be used for control of the receiver or for aiding. See the u-blox documentation links below for further details.

**TP** is connected to the SAM-M8Q TP time pulse pin. It can be used to output pulse trains synchronized with GPS or UTC time grid with intervals configurable over a wide frequency range.
Thus it may be used as a low frequency time synchronization pulse or as a high frequency reference signal (up to 10 MHz). By default, the time pulse signal is configured to 1 pulse per second.

TP is also connected to an LED via a buffer transistor. By default it will flash once per second once the receiver is synchronised to GNSS time.

![SAM-M8Q_FeatherWing_TPLED](https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing/blob/master/img/SAM-M8Q_FeatherWing_TPLED.jpg)

**En** is a true 'power disable' control line you can use to completely cut power to the SAM-M8Q. This is good if you need to run at ultra-low-power modes. By default this is pulled low (enabled). So pull high (to 3V) to disable the SAM-M8Q.

## SAFEBOOT

![SAM-M8Q_FeatherWing_SAFEBOOT](https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing/blob/master/img/SAM-M8Q_FeatherWing_SAFEBOOT.jpg)

A small, square, unlabelled via now provides access to the SAFEBOOT pin. You will need access to this to update the M8 firmware.

## Battery Backup

Like the Ultimate GPS FeatherWing, the SAM-M8Q FeatherWing includes a holder for a CR1220 back-up battery which will keep the SAM-M8Q's internal clock going if the power is removed or disabled, providing a much quicker "warm start" when the power is reconnected.

If you don't want to use the backup battery, you can instead draw backup power from the 3V pin by shorting the 3V BACKUP split pad. But be careful! Only do this if you **won't** be using the backup battery.
If you install the battery _and_ have the split pad shorted **BAD THINGS WILL HAPPEN!** You might want to put some tape over the battery holder if you have shorted the split pad.

![SAM-M8Q_FeatherWing_Backup](https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing/blob/master/img/SAM-M8Q_FeatherWing_Backup.jpg)

## Antenna

The SAM-M8Q comes with a built-in patch antenna and, unlike the Ultimate GPS, it isn't possible to connect an external antenna. If you do want to use an enternal antenna, check out the MAX-M8Q FeatherWing:
- https://github.com/PaulZC/MAX-M8Q_GPS_FeatherWing

![SAM-M8Q_FeatherWing_Antenna](https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing/blob/master/img/SAM-M8Q_FeatherWing_Antenna.jpg)

## Serial Protocol

The SAM-M8Q is based on u-blox's sophisticated M8 chipset. By default it will output standard NMEA format navigation messages, but also supports much more comprehensive UBX binary format messages.
As the SAM-M8Q is able to provide location information from GPS, GLONASS and Galileo, you will find that the prefix of the NMEA messages changes to: $GP for GPS, SBAS and QZSS; $GL for GLONASS; $GA for Galileo; or $GN if it is using a mix of satellites (which is usually the case).
Having the messages prefixed with $GN will confuse the Adafruit GPS Library and Mikal Hart's TinyGPS. To get round this, you can force the "Talker ID" to GP by sending the following UBX binary format message:

- 0xb5, 0x62, 0x06, 0x17, 0x14, 0x00, 0x20, 0x40, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x96, 0xd9

The M8 chipset supports several different navigation modes including: portable, pedestrian, automotive, sea and "Airborne <1G" (which is really useful for high altitude ballooning!).

In normal power mode, the SAM-M8Q draws approx. 29mA. You can reduce this to approx. 9.5mA by putting the M8 into power save mode. But be careful. If you put the M8 into power save mode too early, before the chip has established a fix, the chip can perform a full restart.

By default, the SAM-M8Q will receive signals from GPS, SBAS, QZSS and GLONASS. If you want to enable Galileo reception, you'll need to do this with another UBX message (see below).

You can find the messages to change the navigation mode, power mode and GNSS configuration in the [GPX_and_CSV_Logger](https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing/tree/master/Arduino) Arduino code.

## GPX_and_CSV_Logger

The [Arduino](https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing/tree/master/Arduino) directory contains code for the [Adafruit Feather M0 Adalogger](https://www.adafruit.com/products/2796) which will log your route to SD card in GPX and CSV format.

By default, the code updates the GPX and CSV files once every five seconds. You can alter the code to log (e.g.) every second should you want to.

Be careful when you disconnect the power as you may end up with a corrupt file if you remove the power while the SD card is being updated.
You can wait for the Adalogger red LED to flash (indicating an SD write) then quickly disconnect the power once the LED has gone out.
Or connect a normally-open push-to-make button between swPin (by default this is digital pin 15 - 0.2" away from the GND pin on the Adalogger) and GND. Pressing it will stop the logger writing to SD card, leaving it ready for the power to be removed.

To enable Galileo reception in addition to GPS, GLONASS and SBAS, uncomment the line which says _#define GALILEO_

To enable power saving mode, uncomment the line which says _#define LOWPOWER_

GPX_and_CSV_Logger can also be used with the Adafruit Ultimate GPS FeatherWing. Comment out the line which says _#define UBLOX_ to use the Ultimate GPS.

## Datasheets

Useful documentation about the SAM-M8Q and its protocol specification can be found at:
- https://www.u-blox.com/en/files/sam-m8q-data-sheet
- https://www.u-blox.com/en/files/sam-m8q-hardware-integration-manual
- https://www.u-blox.com/en/files/u-bloxm8receiverdescrprotspecubx-13003221publicpdf

## Adafruit GPS Library for Arduino

- https://github.com/adafruit/Adafruit-GPS-Library/

## Mikal Hart's TinyGPS

- https://github.com/mikalhart/TinyGPS

## Acknowledgements

The SAM-M8Q GNSS FeatherWing is based extensively on an original design by Adafruit Industries:
- https://github.com/adafruit/Adafruit-Ultimate-GPS-FeatherWing-PCB

Distributed under a Creative Commons Attribution, Share-Alike license

Adafruit invests time and resources providing this open source design, please support Adafruit and open-source hardware by purchasing products from Adafruit!

Enjoy!

### _Paul_