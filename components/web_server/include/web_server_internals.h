#pragma once

#include "esp_err.h"
#include "esp_http_server.h"
#include "stdbool.h"
#include "digital_input.h"

//**************************************************
// Typedefs
//**************************************************

/**
 * @brief Enumerates the supported event types for the web server's
 *        asynchronous notification system.
 */
typedef enum : uint8_t
{
  EVENT_NAME_DIGITAL_INPUT = 0,
  EVENT_NAME_ANALOG_INPUT,
  EVENT_NAME_SENSOR,
} event_name_t;

/**
 * @brief Data payload for digital input state changes.
 */
typedef struct __attribute__((packed))
{
  digital_input_num_t num;
  bool value;
} digital_input_payload_t;

/**
 * @brief Data payload for analog input updates.
 */
typedef struct __attribute__((packed))
{
  digital_input_num_t num;
  uint16_t value;
} analog_input_payload_t;

/**
 * @brief Data payload for environmental sensor readings.
 */
typedef struct __attribute__((packed))
{
  float humidity;
  float temperature;
} sensor_payload_t;

/**
 * @brief Unified event structure for the internal broadcast system.
 *        Uses a union to optimize memory usage by overlaying different
 *        payload types under a single event identifier.
 */
typedef struct __attribute__((packed))
{
  event_name_t name;

  union
  {
    digital_input_payload_t digital_input;
    analog_input_payload_t analog_input;
    sensor_payload_t sensor;
  } payload;
} event_t;

//**************************************************
// Public Functions
//**************************************************

/**
 * @brief Registers the digital output module within the Web Server context.
 *        This function performs the necessary setup for remote output control:
 *
 *        1. Registers the GET handler (`/api/digital-output`) to allow web clients
 *        to poll the current state of any specific digital output.
 *
 *        2. Registers the POST handler (`/api/digital-output`) to allow remote
 *        switching of output states via JSON payloads.
 *
 * @param server Handle to the active HTTP server instance where the URIs will be registered.
 * @return - ESP_OK: All URI handlers registered successfully.
 *
 *         - ESP_FAIL: Failed to register one or more URI handlers.
 */
esp_err_t digital_output_register(httpd_handle_t server);

/**
 * @brief Registers the digital input module within the Web Server context.
 *        This function performs two main integration steps:
 *
 *        1. Registers the REST API URI handler (`/api/digital-input`) to allow
 *        synchronous polling of input states via HTTP GET requests.
 *
 *        2. Attaches an internal event handler to the digital input driver, enabling
 *        asynchronous "push" notifications to web clients whenever a hardware
 *        state change (interrupt-driven) occurs.
 * @param server Handle to the active HTTP server instance where the URI will be registered.
 * @return - ESP_OK: All handlers registered successfully.
 *
 *         - ESP_FAIL: Failed to register URI or attach the observer callback.
 */
esp_err_t digital_input_register(httpd_handle_t server);

/**
 * @brief Initializes the Server-Sent Events (SSE) infrastructure for the Web Server.
 *        This function prepares the asynchronous "push" notification system:
 *
 *        1. Creates a mutex to synchronize access to the client list (thread safety).
 *
 *        2. Initializes the event queue to buffer hardware updates before broadcasting.
 *
 *        3. Launches the dedicated background task responsible for message serialization
 *        and transmission to all connected clients.
 *
 *        4. Registers the GET handler (`/api/events`) to allow clients to establish
 *        persistent SSE connections.
 *
 * @param server Handle to the active HTTP server instance.
 * @return - ESP_OK: SSE module initialized and URI registered successfully.
 *
 *         - ESP_FAIL: Resource allocation failed or URI registration error.
 */
esp_err_t events_register(httpd_handle_t server);

/**
 * @brief Enqueues an event to be broadcasted to all active SSE web clients.
 *        This function acts as the bridge between hardware drivers (Input, Analog, Sensors)
 *        and the Web Server's asynchronous notification system.
 *
 * @param event Pointer to the event structure containing the type and payload data.
 * @return - ESP_OK: Event successfully added to the broadcast queue.
 *
 *         - ESP_FAIL: Queue is full or message could not be sent within the timeout period.
 */
esp_err_t events_send(event_t *event);

/**
 * @brief Registers the web server as an observer of analog input events.
 * @param server Handle to the running HTTP server instance.
 * @return - ESP_OK: Registration successful.
 *
 *         - ESP_FAIL: Failed to attach the event handler.
 */
esp_err_t analog_input_register(httpd_handle_t server);

/**
 * @brief Subscribes the web server to environment sensor updates.
 * @param server Handle to the running HTTP server instance.
 * @return - ESP_OK: Successfully registered as a sensor observer.
 *
 *         - ESP_FAIL: Failed to register the event handler.
 */
esp_err_t sensor_register(httpd_handle_t server);