# ESP32 Web Server with FreeRTOS

This project implements a web server on the ESP32 platform, utilizing FreeRTOS for efficient task management. The server is designed to handle tasks such as reading sensor data, updating a display, and responding to emergency situations.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Features](#features)
- [Code Structure](#code-structure)
    - [main.cpp](#maincpp)
    - [Webserver.h](#Webserverh)
- [Usage](#usage)
- [Testing](#testing)
- [Live Demo](#live-demo)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgements](#acknowledgements)

## Prerequisites

Before you begin, ensure you have met the following requirements:

- You have a ESP32 board.
- You have installed the Arduino IDE or PlatformIO.
- You have a stable internet connection.

## Installation

1. Clone the repository to your local machine.
2. Open the project in your preferred IDE.
3. Update the WiFi SSID and password in the `main.cpp` file to match your network.
4. Upload the code to your ESP32 board.

## Features

- Web server running on ESP32
- Efficient task management with FreeRTOS
- Sensor data reading
- Display updating
- Emergency situation handling

## Code Structure

### main.cpp

The `main.cpp` file is the main code file, containing setup and loop functions, as well as the definitions of FreeRTOS tasks.

Tasks defined in `main.cpp`:

1. **`sensorTask`**: Reads data from sensors at regular intervals, updating global variables with the latest sensor readings.

2. **`displayTask`**: Updates the display with the latest sensor readings whenever there is a change in the sensor data.

3. **`emergencyTask`**: Checks for emergency situations, such as sensor readings exceeding a threshold. Takes appropriate action in case of an emergency.

```cpp

#include <FreeRTOS.h>

// Global variables for sensor data
int temperature = 0;
int humidity = 0;

void setup() {
    // Setup code goes here

    // Create tasks
    xTaskCreate(sensorTask, "Sensor Task", 1000, NULL, 1, NULL);
    xTaskCreate(displayTask, "Display Task", 1000, NULL, 1, NULL);
    xTaskCreate(emergencyTask, "Emergency Task", 1000, NULL, 1, NULL);
}

void loop() {
    // Empty loop
}

void sensorTask(void * parameter) {
    for (;;) {
        // Read data from sensors
        temperature = readTemperatureSensor();
        humidity = readHumiditySensor();

        // Delay for a regular interval
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void displayTask(void * parameter) {
    for (;;) {
        // Update display with latest sensor readings
        updateDisplay(temperature, humidity);

        // Delay until there is a change in the sensor data
        vTaskDelayUntil(&lastWakeTime, (1000 / portTICK_PERIOD_MS));
    }
}

void emergencyTask(void * parameter) {
    for (;;) {
        // Check for emergency situations
        if (temperature > TEMPERATURE_THRESHOLD || humidity > HUMIDITY_THRESHOLD) {
            // Take appropriate action
            triggerEmergencyAlert();
        }

        // Delay for a short time
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
```

These code snippets show how the tasks might be defined and created in the `main.cpp` file. The `sensorTask` reads data from the sensors, the `displayTask` updates the display with the latest sensor readings, and the `emergencyTask` checks for emergency situations. The actual implementation of these tasks would depend on the specific sensors and display used in the project.

### Webserver.h

The `Webserver.h` file is a header file that contains the HTML code for the web server's main page. This HTML code is served to the client when they connect to the ESP32's IP address in a web browser.

This file is included in `main.cpp`, and it's responsible for displaying the current sensor readings and a section for emergency alerts.

Here's a simplified example of what the HTML code might look like:

```cpp
const char MAIN_page[] PROGMEM = R"=====(
<html>
  <body>
    <h1>ESP32 Web Server</h1>
    <p>Current Sensor Readings:</p>
    <p id="sensorData"></p>
    <h2>Emergency Alerts</h2>
    <p id="emergencyAlerts"></p>
    <script>
      // JavaScript code to update sensorData and emergencyAlerts goes here
    </script>
  </body>
</html>
)=====";
```

In this example, the `sensorData` and `emergencyAlerts` elements are placeholders that will be updated with real data by JavaScript code. This JavaScript code would typically make AJAX requests to the ESP32 to get the latest sensor data and emergency alerts, and then update the HTML elements with this data.

The `PROGMEM` keyword is used to store the HTML code in program memory, which is necessary because the ESP32 has limited RAM.

This HTML code is served to the client in the `handleRoot()` function in `main.cpp`, which is called when the client makes a GET request to the root URL (`/`).

## Usage

Once the code is uploaded to your ESP32 board, access the web server by entering the ESP32's IP address in a web browser.

## Testing

To test the project, connect to the web server and verify that the sensor readings are displayed correctly and that the emergency alerts are working as expected.

## Live Demo

[Screencast from 01-12-2024 01:15:53 AM.webm](https://github.com/ESP32-Work/Embedded-Systems-Semester-Project/assets/81290322/210c93d9-d37c-4b91-9d12-6f95ebebdda4)

## Contributing

Contributions are welcome. Please open an issue or submit a pull request.

## License

This project is licensed under the MIT [License](LICENSE).

## Acknowledgements

- Original code can be found [here](https://github.com/KrisKasprzak/ESP32_WebPage).
