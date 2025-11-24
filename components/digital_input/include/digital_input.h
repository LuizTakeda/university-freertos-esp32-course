#pragma once

#include "esp_err.h"

//**************************************************
// Typedefs
//**************************************************

typedef enum
{
  DIGITAL_INPUT_NUM_1 = 0,
  DIGITAL_INPUT_NUM_2,
  DIGITAL_INPUT_NUM_3,
  DIGITAL_INPUT_NUM_4,
  _DIGITAL_INPUT_NUM_MAX,
} digital_input_num_t;

typedef struct
{

} digital_input_event_t;

typedef void (*digital_input_event_handler_t)(digital_input_event_t);

typedef enum
{
  DIGITAL_INPUT_STATE_FAIL = -1,
  DIGITAL_INPUT_STATE_OFF = 0,
  DIGITAL_INPUT_STATE_ON,
} digital_input_state_t;

//**************************************************
// Funtions
//**************************************************

esp_err_t digital_input_initialize();

esp_err_t digital_input_add_event_handler(digital_input_event_handler_t handler);

digital_input_state_t digital_input_get_state(digital_input_num_t num);