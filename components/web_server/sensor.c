#include "web_server_internals.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "sensor.h"

//**************************************************
// Function Prototypes
//**************************************************

/**
 * @brief Callback triggered by the sensor module when new data is sampled.
 */
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
  if (sensor_add_event_handler(sensor_event_handler) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Fail to add event handler", __func__);
    return ESP_FAIL;
  }

  return ESP_OK;
}

//**************************************************
// Static Functions
//**************************************************

/**
 * @brief Formats and dispatches sensor data to the web event distribution system.
 *        This bridge function receives floating-point data from the DHT task and
 *        packages it into a standard event structure for network transmission.
 * @param humidity    Relative humidity value.
 * @param temperature Temperature value in Celsius.
 */
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