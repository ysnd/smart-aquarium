# Smart Aquarium

This project is an IoT based smart Aquarium using telegram bot as a notification and control interface.

## Features
- Reads tank temperature, TDS Value.
- Send notification if temperature/TDS reach threshold value.
- Automatic heater control based on threshold value.
- Send data to google sheet.
- Control feed servo, LED, etc. using button in telegram bot. 

## Hardware Component
- ESP32
- DS18B20 temperature sensor
- LED
- Heater
- 2 Channel relays
- Motor Servo

## Wiring Diagram
<img src="images/wiring.jpg" width="800" />

## Prototype
<p align="center">
  <img src="images/1.jpg" width="80%" />
  <img src="images/2.jpg" width="50%" />
  <img src="images/3.png" width="120%" />
</p>

## Screenshot Telegram Bot
<p align="center">
  <img src="images/ssbot.png" width="500" />
  <img src="images/ssalert.png" width="500" />
  <img src="images/ssstoredata.png" width="500" />
</p>

