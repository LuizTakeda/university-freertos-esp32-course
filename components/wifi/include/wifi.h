#pragma once

#include "esp_err.h"

//**************************************************
// Public Functions
//**************************************************

/**
 * @brief Initializes the Wi-Fi station mode.
 * * This function performs the complete setup for Wi-Fi station (STA) mode:
 *
 * 1. Initializes the underlying TCP/IP stack (netif).
 *
 * 2. Creates the default event loop if not already present.
 *
 * 3. Registers event handlers for Wi-Fi and IP events.
 *
 * 4. Configures the Wi-Fi driver with SSID and Password from Kconfig.
 *
 * 5. Starts the Wi-Fi driver to begin the connection process.
 * @note This function expects NVS flash to be initialized beforehand as
 *       the Wi-Fi stack uses it to store configuration data.
 * @return
 * - ESP_OK: Wi-Fi started successfully.
 *
 * - ESP_FAIL: Failed to initialize netif, events, or driver.
 */
esp_err_t wifi_initialize();
