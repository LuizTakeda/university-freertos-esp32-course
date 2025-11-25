#pragma once

#include "esp_err.h"
#include "esp_http_server.h"
#include "stdbool.h"
#include "digital_input.h"

//**************************************************
// Typedefs
//**************************************************

typedef enum
{
  EVENT_NAME_DIGITAL_INPUT = 0,
  EVENT_NAME_ANALOG_INPUT
} event_name_t;

typedef struct
{
  digital_input_num_t num;
  bool value;
} digital_input_payload_t;

typedef struct
{
  digital_input_num_t num;
  uint16_t value;
} analog_input_payload_t;

typedef union
{
  digital_input_payload_t digital_input;
  analog_input_payload_t analog_input;
} event_payload_t;

typedef struct
{
  event_name_t name;
  event_payload_t payload;
} event_t;

//**************************************************
// Function Prototypes
//**************************************************

esp_err_t digital_output_register(httpd_handle_t server);

esp_err_t digital_input_register(httpd_handle_t server);

esp_err_t events_register(httpd_handle_t server);

esp_err_t events_send(const event_t event);

esp_err_t analog_input_register(httpd_handle_t server);