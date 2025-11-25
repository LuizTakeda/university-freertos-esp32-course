#include "sensor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <dht.h>

//**************************************************
// Typedefs
//**************************************************

typedef struct event_node_t
{
  struct event_node_t *next;
  sensor_event_handler_t handler;
} event_node_t;

//**************************************************
// Funtion Prototypes
//**************************************************

void sensor_reader_task();

//**************************************************
// Globals
//**************************************************

static const char TAG[] = "sensor";

static event_node_t *s_first_event_node = NULL;
static SemaphoreHandle_t s_event_node_mutex = NULL;

//**************************************************
// Public Functions
//**************************************************

esp_err_t sensor_initialize()
{
  if ((s_event_node_mutex = xSemaphoreCreateMutex()) == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to create event node mutex", __func__);
    return ESP_FAIL;
  }

  if (xTaskCreate(sensor_reader_task, "sensor_reader_task", 2048, NULL, 2, NULL) != pdPASS)
  {
    ESP_LOGE(TAG, "%s:Fail to create sensor reader task", __func__);
    return ESP_FAIL;
  }

  return ESP_OK;
}

esp_err_t sensor_add_event_handler(sensor_event_handler_t handler)
{
  event_node_t *new_node = malloc(sizeof(event_node_t));
  if (new_node == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to alloc event node", __func__);
    return ESP_FAIL;
  }

  new_node->handler = handler;
  new_node->next = NULL;

  if (xSemaphoreTake(s_event_node_mutex, portMAX_DELAY) != pdTRUE)
  {
    ESP_LOGE(TAG, "%s:Fail to take event node mutex", __func__);
    free(new_node);
    return ESP_FAIL;
  }

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

void sensor_reader_task()
{
  TickType_t last_wake_time = xTaskGetTickCount();

  float temperature, humidity;
  while (true)
  {
    xTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(1500));

    if (dht_read_float_data(DHT_TYPE_DHT11, GPIO_NUM_4, &humidity, &temperature) != ESP_OK)
    {
      ESP_LOGE(TAG, "%s:Fail to read sensor data", __func__);
      continue;
    }

    if (xSemaphoreTake(s_event_node_mutex, portMAX_DELAY) != pdTRUE)
    {
      ESP_LOGE(TAG, "%s:Fail to take event node mutex", __func__);
      continue;
    }

    event_node_t *node = s_first_event_node;
    while (node != NULL)
    {
      node->handler(temperature, humidity);
      node = node->next;
    }

    xSemaphoreGive(s_event_node_mutex);
  }

  vTaskDelete(NULL);
}