# FreeRTOS + ESP32: Embedded Systems Programming

This repository contains the source code for an academic extension project on embedded systems using FreeRTOS and ESP32.

The course covers multitasking, real-time scheduling, and inter-task communication. In the final project, you’ll build a dashboard hosted on the ESP32 to control and monitor it in real-time.

Part of [*The Parallel Computing School: Short Courses and Tutorials*](https://github.com/rogerioag/ecp-minicursos).

## Tech Stack

**Firmware:**  
[![C](https://img.shields.io/badge/Language-C-blue?logo=c)](https://www.w3schools.com/c/c_intro.php)
[![ESP32](https://img.shields.io/badge/Microcontroller-ESP32-red?logo=espressif)](https://www.espressif.com/en/products/socs/esp32)
[![ESP-IDF](https://img.shields.io/badge/Framework-ESP--IDF-red?logo=espressif)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)

**Frontend Dev:**  
[![TypeScript](https://img.shields.io/badge/TypeScript-007ACC?logo=typescript&logoColor=white)](https://www.typescriptlang.org/)
[![SASS](https://img.shields.io/badge/SASS-CC6699?logo=sass&logoColor=white)](https://sass-lang.com/)
[![Webpack](https://img.shields.io/badge/Webpack-8DD6F9?logo=webpack&logoColor=white)](https://webpack.js.org/)
[![Web Components](https://img.shields.io/badge/Web%20Components-native-blue)](https://developer.mozilla.org/en-US/docs/Web/API/Web_components)
[![Node.js](https://img.shields.io/badge/Node.js-339933?logo=node.js&logoColor=white)](https://nodejs.org/)
[![Express](https://img.shields.io/badge/Express.js-000000?logo=express&logoColor=white)](https://expressjs.com/)

## Architecture

The system is designed using an Event-Driven Architecture.

- **Real-Time Monitoring**: Instead of traditional HTTP polling, this project implements Server-Sent Events (SSE). This allows the ESP32 to push sensor updates to the frontend instantly as they happen, significantly reducing network traffic and latency.
- **Non-Blocking Hardware Abstraction**: Each hardware peripheral (Analog, Digital, Sensors) operates in its own dedicated FreeRTOS task. They communicate with the Web Server via asynchronous events, ensuring that a slow sensor read never freezes the UI or the WiFi stack.

```mermaid
flowchart TB
    %% Extern entity
    User((User/Browser)) <-->|HTTP/SSE| Web_Server 

    subgraph ESP32_Firmware [ESP32 Application]
        direction LR

        subgraph ESPIDF [System Services]
            direction TB
            WiFi[WiFi Driver] --> EventLoop[Default Event Loop]
        end

        subgraph Web_Server [Web Server]
            direction TB
            WBURIHandler[URI Handlers]
            WBEvent[Event Handlers]
            WBSSE[SSE Task]
            WBEvent --> WBSSE 
        end

        subgraph Hardware_Drivers [Hardware Abstraction Layer]
            direction TB
            
            subgraph Inputs [Monitoring]
                direction LR
                Analog_Input[Analog Input Task]
                Digital_Input[Digital Input Task]
                Sensor_Block[Sensor Task]
            end

            DigitalOutput[Digital Output]
        end

        %% Connections
        EventLoop --"IP/WIFI Events"--> Web_Server
        Inputs --"Events"--> Web_Server
        Web_Server <--"Control"--> DigitalOutput
    end

    %% Styling
    style Web_Server stroke:#01579b,stroke-width:2px
    style Hardware_Drivers stroke:#33691e
    style ESPIDF stroke:#e65100
```

## Frontend

The dashboard is built with [Web Components](https://developer.mozilla.org/en-US/docs/Web/API/Web_components) and [Webpack](https://webpack.js.org/) to bundle and minify the project, making it ideal for resource-limited devices like the ESP32.

### Desktop
<img width="1920" height="970" alt="image" src="https://github.com/user-attachments/assets/9e0e7447-379c-4b75-963f-592c2925e2e3" />
<img width="1920" height="970" alt="image" src="https://github.com/user-attachments/assets/94fded25-5059-4fc1-a0be-00b0b86bcaae" />

### Mobile
<img height="500" alt="image" src="https://github.com/user-attachments/assets/38c5143b-0b90-49f5-bce2-a47aee993ab1" />
<img height="500" alt="image" src="https://github.com/user-attachments/assets/ebdbfb8a-161f-4b1d-ae9e-bef3b525f36d" />
<img height="500" alt="image" src="https://github.com/user-attachments/assets/483c86ad-356a-498a-ae5c-352f311a1cb6" />

For more details, see the [Frontend README](./components/web_server/frontend/README.md)
