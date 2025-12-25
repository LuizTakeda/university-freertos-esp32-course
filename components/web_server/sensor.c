#include "web_server_internals.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "sensor.h"

//**************************************************
// Function Prototypes
//**************************************************

static void sensor_event_handler(float humidty, float temperature);

//**************************************************
// Globals
//**************************************************

static const char TAG[] = "web_server:sensor";

//**************************************************
// Public Functions
//**************************************************

esp_err_t sensor_register(httpd_handle_t server)
{
  ESP_ERROR_CHECK(sensor_add_event_handler(sensor_event_handler));

  return ESP_FAIL;
}

//**************************************************
// Static Functions
//**************************************************

static void sensor_event_handler(float humidity, float temperature)
{
  event_t event = {
      .name = EVENT_NAME_SENSOR,
      .payload.sensor = {
          .humidity = humidity,
          .temperature = temperature,
      },
  };

  if (events_send(&event) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Fail to send event", __func__);
  }
}