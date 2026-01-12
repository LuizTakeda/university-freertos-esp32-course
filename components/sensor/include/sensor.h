#pragma once

#include "esp_err.h"

//**************************************************
// Typedefs
//**************************************************

/**
 * @brief Callback function type for sensor data updates.
 * @param humidity    The relative humidity percentage (0.0 to 100.0).
 * @param temperature The temperature in degrees Celsius.
 */
typedef void (*sensor_event_handler_t)(float humidity, float temperature);

//**************************************************
// Public Functions
//**************************************************

/**
 * @brief Initializes the sensor component resources.
 *        This function creates the synchronization primitives (mutex) and spawns
 *        the periodic background task responsible for sampling the physical sensor.
 * @return - ESP_OK: Initialization successful.
 *
 *         - ESP_FAIL: Failed to create OS resources or task.
 */
esp_err_t sensor_initialize();

/**
 * @brief Registers a new observer to receive periodic sensor data.
 *        The system follows an Observer pattern. All registered handlers will be
 *        called sequentially every time a valid sample is read from the hardware.
 * @note This function is thread-safe and prevents duplicate handler registration.
 * @param handler The callback function to be registered.
 * @return - ESP_OK: Handler registered successfully.
 *
 *         - ESP_ERR_INVALID_ARG: Provided handler was NULL.
 *
 *         - ESP_ERR_NO_MEM: Memory allocation failed for the observer node.
 */
esp_err_t sensor_add_event_handler(sensor_event_handler_t handler);