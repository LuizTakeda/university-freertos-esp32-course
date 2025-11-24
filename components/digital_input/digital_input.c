#include <stdio.h>
#include "digital_input.h"
#include "digital_input_internals.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

//**************************************************
// Funtion Prototypes
//**************************************************

static void task(void *args);

//**************************************************
// Globals
//**************************************************

static const char TAG[] = "digital_input";

static SemaphoreHandle_t s_node_mutex = NULL;

static digital_input_internal_handler_node_t s_first_node = {.next = NULL, .event_handler = NULL};

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
  return DIGITAL_INPUT_STATE_OFF;
}

//**************************************************
// Static Funtions
//**************************************************

static void reader_task(void *args)
{
}

static void event_dispatcher_task(void *args) {
  
}