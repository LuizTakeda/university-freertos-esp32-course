#include "analog_input.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_adc/adc_oneshot.h"

//**************************************************
// Typedefs
//**************************************************

typedef struct event_node_t
{
  struct event_node_t *next;
  analog_input_event_handler_t handler;
} event_node_t;

//**************************************************
// Function Prototypes
//**************************************************

static void analog_reader_task(void *args);

//**************************************************
// Globals
//**************************************************

static const char TAG[] = "analog_input";

static event_node_t *s_first_event_node = NULL;

static SemaphoreHandle_t s_event_node_mutex = NULL;

static adc_oneshot_unit_handle_t s_adc1_handler;

static const uint16_t s_analog_input_num_map[] = {ADC_CHANNEL_6, ADC_CHANNEL_7};

//**************************************************
// Public Functions
//**************************************************

esp_err_t analog_input_initialize(void)
{
  if ((s_event_node_mutex = xSemaphoreCreateMutex()) == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to create event node mutex", __func__);
    return ESP_FAIL;
  }

  if (xTaskCreate(analog_reader_task, "analog_reader_task", 2048, NULL, 1, NULL) != pdPASS)
  {
    ESP_LOGE(TAG, "%s:Fail to create analog reader task", __func__);
    return ESP_FAIL;
  }

  adc_oneshot_unit_init_cfg_t init_config = {
      .unit_id = ADC_UNIT_1,
  };

  if (adc_oneshot_new_unit(&init_config, &s_adc1_handler) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Fail to init oneshot adc1", __func__);
    return ESP_FAIL;
  }

  adc_oneshot_chan_cfg_t config = {
      .bitwidth = ADC_BITWIDTH_DEFAULT,
      .atten = ADC_ATTEN_DB_12,
  };

  for (int i = 0; i < _ANALOG_INPUT_NUM_MAX; i++)
  {
    if (adc_oneshot_config_channel(s_adc1_handler, s_analog_input_num_map[i], &config) != ESP_OK)
    {
      ESP_LOGE(TAG, "%s:Fail to confi channel %d", __func__, i);
      return ESP_FAIL;
    }
  }

  return ESP_OK;
}

esp_err_t analog_input_add_event_handler(analog_input_event_handler_t handler)
{
  if ((xSemaphoreTake(s_event_node_mutex, portMAX_DELAY)) != pdTRUE)
  {
    ESP_LOGE(TAG, "%s:Fail to take event node mutex", __func__);
    return ESP_FAIL;
  }

  event_node_t *new_node = malloc(sizeof(event_node_t));

  if (new_node == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to alloc new event node", __func__);
    xSemaphoreGive(s_event_node_mutex);
    return ESP_FAIL;
  }

  new_node->handler = handler;
  new_node->next = NULL;

  if (s_first_event_node == NULL)
  {
    s_first_event_node = new_node;
  }
  else
  {
    event_node_t *last_node = s_first_event_node;
    while (last_node->next != NULL)
    {
      last_node = last_node->next;
    }
    last_node->next = new_node;
  }

  xSemaphoreGive(s_event_node_mutex);
  return ESP_OK;
}

//**************************************************
// Static Functions
//**************************************************

static void analog_reader_task(void *args)
{
  TickType_t last_wake_time = xTaskGetTickCount();

  int raw[_ANALOG_INPUT_NUM_MAX];
  while (true)
  {
    xTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(50));

    for (int i = 0; i < _ANALOG_INPUT_NUM_MAX; i++)
    {
      if (adc_oneshot_read(s_adc1_handler, s_analog_input_num_map[i], &raw[i]) != ESP_OK)
      {
        ESP_LOGE(TAG, "%s:Fail to read analog input", __func__);
      }
    }

    if ((xSemaphoreTake(s_event_node_mutex, portMAX_DELAY)) != pdTRUE)
    {
      continue;
    }

    event_node_t *node = s_first_event_node;

    while (node != NULL)
    {
      for (int i = 0; i < _ANALOG_INPUT_NUM_MAX; i++)
      {
        node->handler(i, raw[i]);
      }

      node = node->next;
    }

    xSemaphoreGive(s_event_node_mutex);
  }

  vTaskDelete(NULL);
}