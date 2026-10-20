# SmartFrame

## Summary

- [Overview](#overview)
- [The project](#the-project)
- [Characteristics](#characteristics)
- [Used technologies](#used-technologies)
- [My code](#my-code)
- [Difficulties](#difficulties)
- [Future improvements](#future-improvements)
- [Build the firmware](#build-the-firmware)

## Overview

SmartFrame is an IoT-connected 64×64 LED matrix built with an Adafruit Portal S3. It cycles through screens showing the current time, local weather, and live Spotify playback, including downsampled album art when a connected account is active. The device hosts a local web dashboard where users can manage screens and upload custom images.

> [!NOTE]
> The core project is functional, but I am still working on smoother animated screens and adding configuration options to the dashboard. I made SmartFrame as a student project to learn about embedded programming, APIs, and connecting hardware to online services.

For more information and a demonstration, see [SmartFrame on my portfolio](https://dawid-sac.dev/smartframe).

## The project

SmartFrame connects to Wi-Fi and gets information from online services. The firmware chooses which screen to show and draws it on the LED matrix. A small web dashboard is hosted by the device, so it can be accessed from the local network.

The main screens and features are:

- **Clock:** shows the current time, synchronized using an NTP time server.
- **Weather:** retrieves local weather information from OpenWeatherMap.
- **Spotify:** checks current playback and displays track information and resized album art.
- **Custom image:** lets an image be uploaded through the dashboard and displayed on the matrix.
- **Screen controls:** the dashboard can be used to manage the available screens and their display.

The screens are managed by a scheduler. The firmware uses separate FreeRTOS tasks for the display and network services, so the display can keep running while information is being fetched.

## Characteristics

### General

- 64×64 LED matrix
- Adafruit Portal S3 microcontroller
- Wi-Fi-connected
- Displays clock, weather, Spotify playback, and custom images
- Local web dashboard hosted by the device

### Firmware

- ESP-IDF framework
- FreeRTOS tasks for the UI and network services
- HTTPS web server
- Persistent storage for uploaded images and application data

## Used technologies

- [ESP-IDF](https://github.com/espressif/esp-idf) for the firmware and hardware support
- FreeRTOS, included with ESP-IDF, for task scheduling
- Spotify Web API for playback information and album art
- OpenWeatherMap API for weather information
- NTP for synchronizing the clock
- ESP-IDF component manager dependencies, including the HUB75 display component

## My code

I organized the firmware into ESP-IDF components. The display, screen scheduler, storage, Wi-Fi, web server, and API integrations are kept in separate parts of the project.

One of the more interesting parts was the screen scheduler. It rotates through screens and handles transitions, while allowing Spotify to take priority when music is playing. The clock and weather screens can continue to be shown when Spotify is not active.

The Spotify integration has to authenticate with Spotify, request playback data, and download and decode the album cover. The weather integration fetches and parses data separately. Both services use their own tasks, so waiting for a network request does not stop the display from updating.

## Difficulties

### Memory use

The ESP32 has limited memory, and parsing API responses while decoding album art can use a lot of it. The project includes custom memory allocators and shared data structures to help manage memory during these operations.

### Spotify authentication

Spotify authentication required more steps than I first expected. It made the integration more complicated than simply requesting the current track, but it was important to make the playback feature work.

### Keeping the display responsive

The device needs to update the screen while network requests are happening. I separated the display loop and API work into different FreeRTOS tasks to avoid making the interface wait for those requests.

## Future improvements

The core project is working, but there are still things I would like to improve. I am working on making some animations smoother and adding the configuration options that are still missing from the web dashboard.

## Build the firmware

The project targets the ESP32-S3 and uses ESP-IDF 6.1.0. Build it from the repository root in an ESP-IDF environment:

```sh
idf.py set-target esp32s3
idf.py build
```

To flash the firmware and open the serial monitor, replace `/dev/ttyACM0` with the serial port for your board:

```sh
idf.py -p /dev/ttyACM0 flash monitor
```

The project uses a local `secrets.h` for Wi-Fi credentials and location values. This file is not included in the repository, so configure your own values before building. Do not publish your private credentials.
