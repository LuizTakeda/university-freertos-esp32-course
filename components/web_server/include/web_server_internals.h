#pragma once

#include "esp_err.h"

#include "esp_http_server.h"

esp_err_t digital_output_register(httpd_handle_t server);

esp_err_t digital_input_register(httpd_handle_t server);