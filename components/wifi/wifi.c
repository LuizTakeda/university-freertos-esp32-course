#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

//**************************************************
// Defines
//**************************************************

#if !defined(CONFIG_WIFI_SSID) || !defined(CONFIG_WIFI_PASSWORD)
#error "WiFi SSID or Password not defined! Please run idf.py menuconfig"
#endif

#define WIFI_SSID CONFIG_WIFI_SSID
#define WIFI_PASSWORD CONFIG_WIFI_PASSWORD

//**************************************************
// Static Function Prototypes
//**************************************************

static void event_handler(void *arg, esp_event_base_t base, int32_t id, void *data);

//**************************************************
// Globals
//**************************************************

static const char *TAG = "wifi-station";

//**************************************************
// Functions
//**************************************************

esp_err_t wifi_initialize()
{
  // 1. Network Interface & Event Loop
  ESP_ERROR_CHECK(esp_netif_init());
  esp_err_t err = esp_event_loop_create_default();
  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
  {
    return err;
  }

  esp_netif_create_default_wifi_sta();

  // 2. Wi-Fi Driver Init
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  // 3. Register Event Handlers
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

  // 4. Wi-Fi Configuration
  wifi_config_t wifi_config = {
      .sta = {
          .ssid = WIFI_SSID,
          .password = WIFI_PASSWORD,
          .threshold.authmode = WIFI_AUTH_WPA2_PSK,
      },
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

  ESP_LOGI(TAG, "%s:Starting Wi-Fi station mode...", __func__);
  return esp_wifi_start();
}

//**************************************************
// Static Functions
//**************************************************

/**
 * @brief Consolidated event handler for Wi-Fi and IP events.
 */
static void event_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
  if (base == WIFI_EVENT)
  {
    switch (id)
    {
    case WIFI_EVENT_STA_START:
    case WIFI_EVENT_STA_DISCONNECTED:
      ESP_LOGI(TAG, "%s:Connecting to AP...", __func__);
      esp_wifi_connect();
      break;

    case WIFI_EVENT_STA_CONNECTED:
      ESP_LOGI(TAG, "%s:Connected to AP, waiting for IP...", __func__);
      break;
    }
  }
  else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP)
  {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)data;
    ESP_LOGI(TAG, "%s:Successfully got IP: " IPSTR, __func__, IP2STR(&event->ip_info.ip));
  }
}