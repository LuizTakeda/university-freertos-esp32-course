#pragma once

#include "esp_err.h"
#include "esp_http_server.h"

typedef union
{
} event_t;

esp_err_t digital_output_register(httpd_handle_t server);

esp_err_t digital_input_register(httpd_handle_t server);

esp_err_t events_register(httpd_handle_t server);

esp_err_t events_send(const event_t event);