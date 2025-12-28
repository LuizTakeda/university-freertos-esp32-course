#include "web_server_internals.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "analog_input.h"

//**************************************************
// Functions Prototypes
//**************************************************

/**
 * @brief Callback handler triggered by the analog input module.
 */
void analog_input_event_handler(const analog_input_num_t num, const uint16_t value);

//**************************************************
// Globals
//**************************************************

static const char TAG[] = "web_server:analog_input";

//**************************************************
// Public Functions
//**************************************************

esp_err_t analog_input_register(httpd_handle_t server)
{
  // Subscribe to the analog input module updates
  if (analog_input_add_event_handler(analog_input_event_handler) != ESP_OK)
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
 * @brief Handles incoming data from the analog sensor and dispatches it to the web clients.
 *        This function acts as the bridge: it wraps the raw sensor data into a structured
 *        event type and sends it through the web server's event distribution system.
 * @param num   The logical ID of the analog input that changed.
 * @param value The raw or processed digital value from the ADC.
 */
void analog_input_event_handler(const analog_input_num_t num, const uint16_t value)
{
  // Create the event package to be serialized/sent over the network
  event_t event = {
      .name = EVENT_NAME_ANALOG_INPUT,
      .payload.analog_input = {
          .num = num,
          .value = value,
      },
  };

  // Dispatch the event to connected web clients
  if (events_send(&event) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Fail to send event", __func__);
  }
}
