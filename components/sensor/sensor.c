#include "sensor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <dht.h>

//**************************************************
// Defines
//**************************************************

#define SENSOR_GPIO GPIO_NUM_4
#define SENSOR_TYPE DHT_TYPE_DHT11
#define SENSOR_POLL_RATE 1500 // ms

//**************************************************
// Typedefs
//**************************************************

/**
 * @brief Linked list node representing a sensor data observer
 */
typedef struct event_node_t
{
  struct event_node_t *next;
  sensor_event_handler_t handler;
} event_node_t;

//**************************************************
// Funtion Prototypes
//**************************************************

void sensor_reader_task();

static esp_err_t is_handler_present(event_node_t *head, sensor_event_handler_t handler);
static esp_err_t add_node(event_node_t **head, sensor_event_handler_t handler);
static esp_err_t foreach_node(event_node_t *head, float humidity, float temperature);

//**************************************************
// Globals
//**************************************************

static const char TAG[] = "sensor";

static event_node_t *s_first_event_node = NULL;     /**< Head of the linked list of handlers */
static SemaphoreHandle_t s_event_node_mutex = NULL; /**< Mutex to protect list during concurrent access */

//**************************************************
// Public Functions
//**************************************************

esp_err_t sensor_initialize()
{
  // Initialize list protection
  if ((s_event_node_mutex = xSemaphoreCreateMutex()) == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to create event node mutex", __func__);
    return ESP_FAIL;
  }

  // Spawn the periodic sampling task (Higher stack for float operations)
  if (xTaskCreate(sensor_reader_task, "sensor_reader_task", 4096, NULL, 2, NULL) != pdPASS)
  {
    ESP_LOGE(TAG, "%s:Fail to create sensor reader task", __func__);
    return ESP_FAIL;
  }

  return ESP_OK;
}

esp_err_t sensor_add_event_handler(sensor_event_handler_t handler)
{
  // Secure the list modification
  if (xSemaphoreTake(s_event_node_mutex, portMAX_DELAY) != pdTRUE)
  {
    ESP_LOGE(TAG, "%s:Fail to take event node mutex", __func__);
    return ESP_FAIL;
  }

  esp_err_t err = add_node(&s_first_event_node, handler);

  xSemaphoreGive(s_event_node_mutex);

  return err;
}

//**************************************************
// Static Functions
//**************************************************

/**
 * @brief Background task responsible for sampling DHT data and notifying handlers.
 */
void sensor_reader_task()
{
  TickType_t last_wake_time = xTaskGetTickCount();

  float temperature, humidity;
  while (true)
  {
    // Enforce periodic execution
    xTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(SENSOR_POLL_RATE));

    // Perform hardware read
    if (dht_read_float_data(SENSOR_TYPE, SENSOR_GPIO, &humidity, &temperature) != ESP_OK)
    {
      ESP_LOGE(TAG, "%s:Fail to read sensor data", __func__);
      continue;
    }

    // Notify all observers in a thread-safe manner
    if (xSemaphoreTake(s_event_node_mutex, portMAX_DELAY) == pdTRUE)
    {
      foreach_node(s_first_event_node, humidity, temperature);
      xSemaphoreGive(s_event_node_mutex);
    }
  }

  vTaskDelete(NULL);
}

/**
 * @brief Search for a specific handler in the list to avoid duplicate entries.
 * @return ESP_OK if found, ESP_FAIL otherwise.
 */
static esp_err_t is_handler_present(event_node_t *head, sensor_event_handler_t handler)
{
  event_node_t *current = head;
  while (current != NULL)
  {
    if (current->handler == handler)
    {
      return ESP_OK;
    }
    current = current->next;
  }
  return ESP_FAIL;
}

/**
 * @brief Allocates and inserts a new node at the head of the list (LIFO).
 */
static esp_err_t add_node(event_node_t **head, sensor_event_handler_t handler)
{
  // Check if handler already exists in the list
  if (is_handler_present(*head, handler) == ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Handler is already present", __func__);
    return ESP_OK;
  }

  event_node_t *new_node = malloc(sizeof(event_node_t));
  if (new_node == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to alloc new node", __func__);
    return ESP_ERR_NO_MEM;
  }

  new_node->handler = handler;
  new_node->next = *head;
  *head = new_node;
  return ESP_OK;
}

/**
 * @brief Traverses the list and executes each registered handler callback.
 */
static esp_err_t foreach_node(event_node_t *head, float humidity, float temperature)
{
  event_node_t *current = head;
  while (current != NULL)
  {
    if (current->handler != NULL)
    {
      current->handler(humidity, temperature);
    }
    current = current->next;
  }

  return ESP_OK;
}