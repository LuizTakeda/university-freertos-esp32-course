#pragma once

#include "esp_err.h"
#include "stdbool.h"
#include "driver/gpio.h"

//**************************************************
// Typedef
//**************************************************

/**
 * @brief Logical identifiers for available digital output channels.
 * These IDs are mapped to physical GPIOs in the implementation file.
 */
typedef enum
{
  DIGITAL_OUTPUT_NUM_1 = 0,
  DIGITAL_OUTPUT_NUM_2,
  DIGITAL_OUTPUT_NUM_3,
  DIGITAL_OUTPUT_NUM_4,
  _DIGITAL_OUTPUT_NUM_MAX
} digital_output_num_t;

/**
 * @brief Possible states for a digital output, including error status.
 */
typedef enum
{
  DIGITAL_OUTPUT_INVALID_ARG = -1,
  DIGITAL_OUTPUT_OFF = 0,
  DIGITAL_OUTPUT_ON = 1,
} digital_output_state_t;

//**************************************************
// Functions
//**************************************************

/**
 * @brief Initializes the digital output component.
 *        Configures the mapped GPIOs as input/output mode, disables pull-up/down resistors,
 *        and sets the initial state to low.
 * @return
 * - ESP_OK: All outputs configured successfully.
 * - ESP_FAIL: Hardware configuration error.
 */
esp_err_t digital_output_initialize();

/**
 * @brief Retrieves the current logic state of a specific output.
 *        Uses hardware feedback to determine if the pin is currently HIGH or LOW.
 * @param num The logical output number to query.
 * @return
 * - DIGITAL_OUTPUT_ON: The pin is currently HIGH.
 * - DIGITAL_OUTPUT_OFF: The pin is currently LOW.
 * - DIGITAL_OUTPUT_INVALID_ARG: Provided channel is out of bounds.
 */
digital_output_state_t digital_output_get_state(digital_output_num_t num);

/**
 * @brief Sets the logic level of a specific digital output.
 * @param num The logical output number to modify.
 * @param new_state The desired logic level (true for HIGH, false for LOW).
 * @return
 * - ESP_OK: Output set successfully.
 * - ESP_ERR_INVALID_ARG: Provided channel is out of bounds.
 */
esp_err_t digital_output_set_state(digital_output_num_t num, bool new_state);
