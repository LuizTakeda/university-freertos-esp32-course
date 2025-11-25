#pragma once

#include "esp_err.h"
#include "stdbool.h"

//**************************************************
// Typedefs
//**************************************************

typedef enum
{
  ANALOG_INPUT_NUM_1 = 0,
  ANALOG_INPUT_NUM_2,
  ANALOG_INPUT_NUM_3,
  _ANALOG_INPUT_NUM_MAX,
} analog_input_num_t;

typedef void (*analog_input_event_handler_t)(const analog_input_num_t num, const bool state);

//**************************************************
// Function Prototypes
//**************************************************

esp_err_t analog_input_initialize(void);

esp_err_t analog_input_add_event_handler(analog_input_event_handler_t handler);
