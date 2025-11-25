#include "nvs_flash.h"
#include "esp_log.h"
#include "wifi.h"
#include "web_server.h"
#include "digital_output.h"
#include "digital_input.h"
#include "analog_input.h"

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

	ESP_ERROR_CHECK(wifi_initialize());
	ESP_ERROR_CHECK(web_server_initialize());
	ESP_ERROR_CHECK(digital_output_initialize());
	ESP_ERROR_CHECK(digital_input_initialize());
	ESP_ERROR_CHECK(analog_input_initialize());
}

//**************************************************
// Static Functions
//**************************************************