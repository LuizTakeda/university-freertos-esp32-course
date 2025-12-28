#pragma once

#include "esp_err.h"
#include "esp_http_server.h"
#include "stdbool.h"
#include "digital_input.h"

//**************************************************
// Typedefs
//**************************************************

typedef enum : uint8_t
{
  EVENT_NAME_DIGITAL_INPUT = 0,
  EVENT_NAME_ANALOG_INPUT,
  EVENT_NAME_SENSOR,
} event_name_t;

typedef struct __attribute__((packed))
{
  digital_input_num_t num;
  bool value;
} digital_input_payload_t;

typedef struct __attribute__((packed))
{
  digital_input_num_t num;
  uint16_t value;
} analog_input_payload_t;

typedef struct __attribute__((packed))
{
  float humidity;
  float temperature;
} sensor_payload_t;

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
// Function Prototypes
//**************************************************

esp_err_t digital_output_register(httpd_handle_t server);

esp_err_t digital_input_register(httpd_handle_t server);

esp_err_t events_register(httpd_handle_t server);

esp_err_t events_send(event_t *event);

/**
 * @brief Registers the web server as an observer of analog input events.
 * @param server Handle to the running HTTP server instance.
 * @return
 * - ESP_OK: Registration successful.
 * 
 * - ESP_FAIL: Failed to attach the event handler.
 */
esp_err_t analog_input_register(httpd_handle_t server);

/**
 * @brief Subscribes the web server to environment sensor updates.
 * @param server Handle to the running HTTP server instance.
 * @return 
 * - ESP_OK: Successfully registered as a sensor observer.
 * 
 * - ESP_FAIL: Failed to register the event handler.
 */
esp_err_t sensor_register(httpd_handle_t server);