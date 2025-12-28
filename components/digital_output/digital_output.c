#include "digital_output.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"

//**************************************************
// Globals
//**************************************************

static const char TAG[] = "digital_output";

/**
 * @brief Map logical output IDs to physical GPIO numbers
 */
static const uint32_t s_output_pins[_DIGITAL_OUTPUT_NUM_MAX] = {
    GPIO_NUM_13,
    GPIO_NUM_19,
    GPIO_NUM_21,
    GPIO_NUM_18};

//**************************************************
// Public Functions
//**************************************************

esp_err_t digital_output_initialize()
{
  // Initialize GPIOs as Output and Input (to allow reading state back)
  gpio_config_t io_conf = {
      .intr_type = GPIO_INTR_DISABLE,
      .mode = GPIO_MODE_INPUT_OUTPUT,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pin_bit_mask = 0};

  // Build bit mask from the pin map
  for (int i = 0; i < _DIGITAL_OUTPUT_NUM_MAX; i++)
  {
    io_conf.pin_bit_mask |= (1ULL << s_output_pins[i]);
  }

  if (gpio_config(&io_conf) != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to configure GPIOs");
    return ESP_FAIL;
  }

  // Set all low
  for (int i = 0; i < _DIGITAL_OUTPUT_NUM_MAX; i++)
  {
    gpio_set_level(s_output_pins[i], 0);
  }

  return ESP_OK;
}

digital_output_state_t digital_output_get_state(digital_output_num_t num)
{
  if (num >= _DIGITAL_OUTPUT_NUM_MAX)
  {
    return DIGITAL_OUTPUT_INVALID_ARG;
  }

  return gpio_get_level(s_output_pins[num]) ? DIGITAL_OUTPUT_ON : DIGITAL_OUTPUT_OFF;
}

esp_err_t digital_output_set_state(digital_output_num_t num, bool new_state)
{
  if (num >= _DIGITAL_OUTPUT_NUM_MAX)
  {
    return ESP_ERR_INVALID_ARG;
  }

  return gpio_set_level(s_output_pins[num], new_state);
}