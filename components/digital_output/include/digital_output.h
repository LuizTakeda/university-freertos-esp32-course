#pragma once

#include "esp_err.h"
#include "stdbool.h"
#include "driver/gpio.h"

//**************************************************
// Typedef
//**************************************************

typedef enum
{
  OUTPUT_NUM_1 = GPIO_NUM_13,
  OUTPUT_NUM_2 = GPIO_NUM_19,
  OUTPUT_NUM_3 = GPIO_NUM_21,
  OUTPUT_NUM_4 = GPIO_NUM_18,
} digital_output_num_t;

typedef enum
{
  DIGITAL_OUTPUT_FAIL = -1,
  DIGITAL_OUTPUT_OFF = 0,
  DIGITAL_OUTPUT_ON = 1,
} digital_output_state_t;

//**************************************************
// Functions
//**************************************************

esp_err_t digital_output_initialize();

digital_output_state_t digital_output_get_state(digital_output_num_t num);

esp_err_t digital_output_set_state(digital_output_num_t num, bool new_state);
