#include <stdio.h>

#include "nvs_flash.h"
#include "esp_log.h"

#include "wifi.h"
#include "web_server.h"

#include "examples.h"

//**************************************************
// Globals
//**************************************************

// static const char TAG[] = "main";

//**************************************************
// Functions
//**************************************************

void app_main(void)
{
	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ESP_ERROR_CHECK(nvs_flash_init());
	}

	err = wifi_initialize();
}

//**************************************************
// Static Functions
//**************************************************