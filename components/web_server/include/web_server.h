#pragma once

#include "esp_err.h"

//**************************************************
// Public Functions
//**************************************************

/**
 * @brief High-level initialization of the Web Server subsystem.
 *        This function prepares the necessary environment for the server to operate
 *        based on network availability:
 *
 *        1. Creates the default system event loop if it has not been initialized.
 *
 *        2. Hooks into Wi-Fi and IP events to automatically manage the server's 
 *        life cycle (starting on IP acquisition and stopping on disconnection).
 * @return - ESP_OK: Initialized successfully and ready for network events.
 *
 *         - ESP_FAIL: Resource allocation failed or event handler registration error.
 */
esp_err_t web_server_initialize();
