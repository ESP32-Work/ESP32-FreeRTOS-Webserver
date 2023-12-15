# Embedded-Systems-Semester-Project
# ESP32 Web Server with FreeRTOS

This project is a web server running on ESP32, using FreeRTOS for task management. It includes tasks for reading sensor data, updating a display, and handling emergency situations.

## Features

- Web server running on ESP32
- FreeRTOS for task management
- Sensor data reading
- Display updating
- Emergency situation handling

## Code Structure

The main code file is `main.cpp`, which includes the setup and loop functions, as well as the definitions of the FreeRTOS tasks.

The `SuperMon.h` file is a header file that contains the HTML code for the web server's main page.

### main.cpp

The `main.cpp` file starts by including necessary libraries and defining constants and global variables. It then defines the FreeRTOS tasks and their functions.

In the `setup()` function, it sets up the WiFi and the web server, and creates the FreeRTOS tasks.

The `loop()` function is left empty as the server is now in its task.

### SuperMon.h

The `SuperMon.h` file contains the HTML code for the web server's main page. It is included in the `main.cpp` file.

## Usage

To use this project, upload the code to your ESP32 board using your preferred IDE (e.g., Arduino IDE, PlatformIO). Make sure to update the WiFi SSID and password in the `main.cpp` file to match your network.

Once the code is uploaded and the ESP32 is running, you can connect to the web server by entering the ESP32's IP address in a web browser.

## Contributing

Contributions are welcome. Please open an issue or submit a pull request.

## License

This project is licensed under the MIT License.

## Acknowledgements
- Original code can be found [here](https://github.com/KrisKasprzak/ESP32_WebPage).
