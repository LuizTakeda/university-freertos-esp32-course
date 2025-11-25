#include "analog_input.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

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

static void analog_reader_task(void *args)

//**************************************************
// Globals
//**************************************************

static const char TAG[] = "analog_input";

static event_node_t *s_first_event_node = NULL;

static SemaphoreHandle_t s_event_node_mutex = NULL;

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
}