#pragma once

#include "esp_err.h"
#include "stdbool.h"

//**************************************************
// Typedefs
//**************************************************

/**
 * @brief Logical identifiers for available digital input channels.
 *        These map to physical GPIOs defined in the implementation file.
 */
typedef enum
{
  DIGITAL_INPUT_NUM_1 = 0,
  DIGITAL_INPUT_NUM_2,
  DIGITAL_INPUT_NUM_3,
  _DIGITAL_INPUT_NUM_MAX,
} digital_input_num_t;

/**
 * @brief Callback function type for digital input state change events.
 * @param num   The logical input number that triggered the event.
 * @param state The new state of the input (true for active/ON, false for inactive/OFF).
 */
typedef void (*digital_input_event_handler_t)(const digital_input_num_t num, const bool state);

/**
 * @brief Possible states of a digital input, including error handling.
 */
typedef enum
{
  DIGITAL_INPUT_STATE_FAIL = -1,
  DIGITAL_INPUT_STATE_OFF = 0,
  DIGITAL_INPUT_STATE_ON,
} digital_input_state_t;

//**************************************************
// Funtions
//**************************************************

/**
 * @brief Initializes the digital input component.
 *        Sets up GPIOs, creates synchronization primitives (mutexes), initializes
 *        the event queue, and spawns the reader and dispatcher tasks.
 * @return - ESP_OK: Success.
 *
 *         - ESP_FAIL: Initialization of hardware or OS resources failed.
 */
esp_err_t digital_input_initialize();

/**
 * @brief Registers a handler to be notified whenever a digital input state changes.
 *        This implements the Observer pattern using an internal linked list of handlers.
 * @note This function is thread-safe and prevents duplicate registrations.
 * @param handler The callback function to be registered.
 * @return - ESP_OK: Successfully added.
 *
 *         - ESP_ERR_INVALID_ARG: Handler was NULL.
 *
 *         - ESP_ERR_NO_MEM: Failed to allocate memory for the new observer node.
 */
esp_err_t digital_input_add_event_handler(digital_input_event_handler_t handler);

/**
 * @brief Retrieves the current state of a specific digital input.
 *        Accesses the internal state bitmask in a thread-safe manner.
 * @param num The logical input number to check.
 * @return - DIGITAL_INPUT_STATE_ON: Input is active.
 *
 *         - DIGITAL_INPUT_STATE_OFF: Input is inactive.
 *
 *         - DIGITAL_INPUT_STATE_FAIL: Could not access state (mutex timeout).
 */
digital_input_state_t digital_input_get_state(digital_input_num_t num);