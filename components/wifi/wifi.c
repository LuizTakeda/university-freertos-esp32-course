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

//**************************************************
// Static Function Prototypes
//**************************************************

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

//**************************************************
// Globals
//**************************************************

static const char *TAG = "wifi-station";

//**************************************************
// Functions
//**************************************************

esp_err_t wifi_initialize()
{
  ESP_LOGI(TAG, "%s: start", __func__);
  ESP_ERROR_CHECK(esp_netif_init());

  esp_err_t err = esp_event_loop_create_default();
  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
  {
    ESP_LOGE(TAG, "%s: Fail to create default event loop", __func__);
    return ESP_FAIL;
  }

  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                      ESP_EVENT_ANY_ID,
                                                      &event_handler,
                                                      NULL,
                                                      NULL));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                      IP_EVENT_STA_GOT_IP,
                                                      &event_handler,
                                                      NULL,
                                                      NULL));

  esp_netif_create_default_wifi_sta();
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  wifi_config_t wifi_config = {
      .sta = {
          .ssid = CONFIG_WIFI_SSID,
          .password = CONFIG_WIFI_PASSWORD,
      },
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "%s: finish", __func__);
  return ESP_OK;
}

//**************************************************
// Static Functions
//**************************************************

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  if (event_base == WIFI_EVENT)
  {
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
      ESP_LOGI(TAG, "wifi connect");
      esp_wifi_connect();
      break;

    case WIFI_EVENT_STA_DISCONNECTED:
      ESP_LOGI(TAG, "retry to connect to the AP");
      esp_wifi_connect();
      break;

    case WIFI_EVENT_STA_CONNECTED:
      ESP_LOGI(TAG, "sta connected");
      break;

    default:
      break;
    }
  }

  if (event_base == IP_EVENT)
  {
    switch (event_id)
    {
    case IP_EVENT_STA_GOT_IP:
      ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

      ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
      break;

    default:
      break;
    }
  }
}