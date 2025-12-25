#pragma once

#include "esp_err.h"

//**************************************************
// Typedefs
//**************************************************

typedef void (*sensor_event_handler_t)(float humidity, float temperature);

//**************************************************
// Public Functions
//**************************************************

esp_err_t sensor_initialize();

esp_err_t sensor_add_event_handler(sensor_event_handler_t handler);