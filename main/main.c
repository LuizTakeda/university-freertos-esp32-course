#include <stdio.h>

#include "nvs_flash.h"
#include "esp_log.h"

#include "wifi.h"
#include "web_server.h"

//**************************************************
// Function Prototypes
//**************************************************

static void on_wifi_connection();

static void on_wifi_disconnection();

//**************************************************
// Globals
//**************************************************

static const char TAG[] = "main";

//**************************************************
// Functions
//**************************************************

void app_main(void)
{
	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	wifi_on_connection(on_wifi_connection);
	wifi_on_disconnection(on_wifi_disconnection);

	wifi_connect();
}

//**************************************************
// Static Functions
//**************************************************

static void on_wifi_connection()
{
	ESP_LOGI(TAG, "wifi connection");

	web_server_start();
}

static void on_wifi_disconnection()
{
	ESP_LOGI(TAG, "wifi disconnection");
	web_server_stop();
}