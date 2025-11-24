#include <stdio.h>
#include "digital_input.h"
#include "digital_input_internals.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

//**************************************************
// Typedefs
//**************************************************

typedef struct
{
  digital_input_num_t num;
  bool new_state;
} input_queue_data_t;

//**************************************************
// Funtion Prototypes
//**************************************************

static void input_reader_task(void *args);
static void event_dispatcher_task(void *args);

//**************************************************
// Globals
//**************************************************

static const char TAG[] = "digital_input";
static const uint32_t s_input_num_map[] = {GPIO_NUM_25, GPIO_NUM_26, GPIO_NUM_27, GPIO_NUM_4};

static digital_input_internal_handler_node_t s_first_node = {.next = NULL, .event_handler = NULL};
static SemaphoreHandle_t s_node_mutex = NULL;
static SemaphoreHandle_t s_input_states_mutex = NULL;
static QueueHandle_t s_input_queue = NULL;
static uint16_t s_input_states = 0;

//**************************************************
// Public Funtions
//**************************************************

esp_err_t digital_input_initialize()
{
  if ((s_node_mutex = xSemaphoreCreateMutex()) == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to create node mutex", __func__);
    return ESP_FAIL;
  }

  if ((s_input_states_mutex = xSemaphoreCreateMutex()) == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to create input states mutex", __func__);
    return ESP_FAIL;
  }

  if ((s_input_queue = xQueueCreate(20, sizeof(input_queue_data_t))) == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to create input queue", __func__);
    return ESP_FAIL;
  }

  if (xTaskCreate(input_reader_task, "input_reader_task", 2048, NULL, 1, NULL) != pdTRUE)
  {
    ESP_LOGE(TAG, "%s:Fail to create input reader task", __func__);
    return ESP_FAIL;
  }

  gpio_config_t io_conf = {
      .intr_type = GPIO_INTR_DISABLE,
      .mode = GPIO_MODE_INPUT,
      .pin_bit_mask = 0,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
  };

  for (uint16_t i = 0; i < _DIGITAL_INPUT_NUM_MAX; i++)
  {
    io_conf.pin_bit_mask |= 1ULL << s_input_num_map[i];
  }

  if (gpio_config(&io_conf) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Fail to config inputs", __func__);
    return ESP_FAIL;
  }

  return ESP_OK;
}

esp_err_t digital_input_add_event_handler(digital_input_event_handler_t handler)
{
  if (xSemaphoreTake(s_node_mutex, pdMS_TO_TICKS(1000)) != pdTRUE)
  {
    ESP_LOGE(TAG, "%s:Fail to take node mutex", __func__);
    return ESP_FAIL;
  }

  digital_input_internal_handler_node_t *new_node = malloc(sizeof(digital_input_internal_handler_node_t));

  if (new_node == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to alloc new node", __func__);
    xSemaphoreGive(s_node_mutex);
    return ESP_FAIL;
  }

  new_node->event_handler = handler;
  new_node->next = NULL;

  digital_input_internal_handler_node_t *last_node = &s_first_node;
  while (last_node->next != NULL)
  {
    last_node = last_node->next;
  }
  last_node->next = new_node;

  xSemaphoreGive(s_node_mutex);

  return ESP_OK;
}

digital_input_state_t digital_input_get_state(digital_input_num_t num)
{
  if (xSemaphoreTake(s_input_states_mutex, pdMS_TO_TICKS(1000)) != pdTRUE)
  {
    ESP_LOGE(TAG, "%s:Fail to take input states mutex", __func__);
    return DIGITAL_INPUT_STATE_FAIL;
  }

  bool state = (s_input_states >> num) & 0b1;

  xSemaphoreGive(s_input_states_mutex);

  return state ? DIGITAL_INPUT_STATE_ON : DIGITAL_INPUT_STATE_OFF;
}

//**************************************************
// Static Funtions
//**************************************************

static void input_reader_task(void *args)
{
  TickType_t last_wake_time = xTaskGetTickCount();

  while (true)
  {
    xTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(500));

    if (xSemaphoreTake(s_input_states_mutex, pdMS_TO_TICKS(1000)) != pdTRUE)
    {
      ESP_LOGE(TAG, "%s:Fail to take input states mutex", __func__);
      continue;
    }

    for (uint16_t i = 0; i < _DIGITAL_INPUT_NUM_MAX; i++)
    {
      bool level = gpio_get_level(s_input_num_map[i]);

      if (((s_input_states >> i) & 0b1) == level)
      {
        continue;
      }

      input_queue_data_t data = {
          .num = i,
          .new_state = level,
      };

      if (xQueueSend(s_input_queue, &data, pdMS_TO_TICKS(250)) != pdTRUE)
      {
        ESP_LOGE(TAG, "%s:Fail to send input data to queue", __func__);
        continue;
      }

      if (level)
      {
        s_input_states |= 0b1 << i;
      }
      else
      {
        s_input_states &= ~(0b1 << i);
      }
    }

    xSemaphoreGive(s_input_states_mutex);
  }

  vTaskDelete(NULL);
}

static void event_dispatcher_task(void *args)
{
  while (true)
  {
  }
  vTaskDelete(NULL);
}