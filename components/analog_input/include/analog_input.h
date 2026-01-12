#pragma once

#include "esp_err.h"
#include "stdbool.h"

//**************************************************
// Typedefs
//**************************************************

/**
 * @brief Logical identifiers for available analog input channels.
 *        These IDs map to physical pins defined in the implementation.
 */
typedef enum
{
  ANALOG_INPUT_NUM_1 = 0,
  ANALOG_INPUT_NUM_2,
  _ANALOG_INPUT_NUM_MAX,
} analog_input_num_t;

/**
 * @brief Callback function type for analog input events.
 * @param num   The logical channel number that triggered the event.
 * @param value The raw ADC value (digital) sampled from the hardware.
 */
typedef void (*analog_input_event_handler_t)(const analog_input_num_t num, const uint16_t value);

//**************************************************
// Function Prototypes
//**************************************************

/**
 * @brief Initializes the analog input component.
 *        This function sets up the ADC hardware, creates the synchronization mutex,
 *        and spawns the background sampling task.
 * @return - ESP_OK: Success.
 *
 *         - ESP_FAIL: Hardware initialization or task creation failed.
 */
esp_err_t analog_input_initialize(void);

/**
 * @brief Registers a new callback function to be notified on every ADC sample.
 *        The system uses an Observer pattern. Every registered handler will be called
 *        sequentially for each active analog channel.
 * @note This function is thread-safe and prevents duplicate registrations.
 * @param handler The function pointer to be registered.
 * @return - ESP_OK: Handler registered successfully.
 *
 *         - ESP_ERR_INVALID_ARG: Provided handler was NULL.
 *
 *         - ESP_ERR_NO_MEM: Failed to allocate memory for the new observer node.
 *
 *         - ESP_FAIL: Mutex timeout or other internal error.
 */
esp_err_t analog_input_add_event_handler(analog_input_event_handler_t handler);
