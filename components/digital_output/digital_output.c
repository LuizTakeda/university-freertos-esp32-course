#include "digital_output.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"

//**************************************************
// Globals
//**************************************************

static const char TAG[] = "digital_output";

SemaphoreHandle_t s_mutex = NULL;

//**************************************************
// Public Functions
//**************************************************

esp_err_t digital_output_initialize()
{
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << OUTPUT_NUM_1) |
                         (1ULL << OUTPUT_NUM_2) |
                         (1ULL << OUTPUT_NUM_3) |
                         (1ULL << OUTPUT_NUM_4);
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 0;

  if (gpio_config(&io_conf) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Fail to configure gpio", __func__);
    return ESP_FAIL;
  }

  if ((s_mutex = xSemaphoreCreateMutex()) == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to create mutex", __func__);
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "%s:Finished", __func__);
  return ESP_OK;
}

digital_output_state_t digital_output_get_state(digital_output_num_t num)
{
  if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(500)) != pdPASS)
  {
    return DIGITAL_OUTPUT_FAIL;
  }

  bool state = gpio_get_level(num);

  xSemaphoreGive(s_mutex);
  return state ? DIGITAL_OUTPUT_ON : DIGITAL_OUTPUT_OFF;
}

esp_err_t digital_output_set_state(digital_output_num_t num, bool new_state)
{
  if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(500)) != pdPASS)
  {
    return ESP_FAIL;
  }

  esp_err_t return_value = ESP_FAIL;

  if (gpio_set_level(num, new_state) != ESP_OK)
  {
    goto end;
  }

  return_value = ESP_OK;

end:
  xSemaphoreGive(s_mutex);
  return return_value;
}

//**************************************************
// Static Functions
//**************************************************