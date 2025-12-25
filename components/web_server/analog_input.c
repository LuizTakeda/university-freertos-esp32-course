#include "web_server_internals.h"

#include "esp_http_server.h"
#include "esp_log.h"
#include "analog_input.h"

//**************************************************
// Functions Prototypes
//**************************************************

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
  ESP_ERROR_CHECK(analog_input_add_event_handler(analog_input_event_handler));

  return ESP_OK;
}

//**************************************************
// Static Functions
//**************************************************

void analog_input_event_handler(const analog_input_num_t num, const uint16_t value)
{
  event_t event = {
      .name = EVENT_NAME_ANALOG_INPUT,
      .payload.analog_input = {
          .num = num,
          .value = value,
      },
  };

  if (events_send(&event) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Fail to send event", __func__);
  }
}
