#include <stdio.h>
#include "digital_input.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

//**************************************************
// Defines
//**************************************************

#define GET_BIT(val, bit) (((val) >> (bit)) & 0x01)
#define SET_BIT(val, bit) ((val) |= (1U << (bit)))
#define CLEAR_BIT(val, bit) ((val) &= ~(1U << (bit)))

//**************************************************
// Typedefs
//**************************************************

/**
 * @brief Linked list node for digital input event observers
 */
typedef struct event_node_t
{
  digital_input_event_handler_t handler;
  struct event_node_t *next;
} event_node_t;

/**
 * @brief Structure for queueing input state change events
 */
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

static esp_err_t is_handler_present(event_node_t *head, digital_input_event_handler_t handler);
static esp_err_t add_node(event_node_t **head, digital_input_event_handler_t handler);
static esp_err_t foreach_node(event_node_t *head, const digital_input_num_t num, const bool state);

//**************************************************
// Globals
//**************************************************

static const char TAG[] = "digital_input";

/**
 * @brief Physical GPIO mapping for logical inputs
 */
static const uint32_t s_input_num_map[] = {GPIO_NUM_25, GPIO_NUM_26, GPIO_NUM_27};

static event_node_t *s_first_node = NULL;             /**< Head of the observer list */
static SemaphoreHandle_t s_node_mutex = NULL;         /**< Protection for the observer list */
static SemaphoreHandle_t s_input_states_mutex = NULL; /**< Protection for the bitmask state */
static QueueHandle_t s_input_queue = NULL;            /**< Inter-task communication queue */
static uint16_t s_input_states = 0;                   /**< Bitmask of current input levels */

//**************************************************
// Public Funtions
//**************************************************

esp_err_t digital_input_initialize()
{
  // Create synchronization primitives
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

  // Initialize the event queue
  if ((s_input_queue = xQueueCreate(20, sizeof(input_queue_data_t))) == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to create input queue", __func__);
    return ESP_FAIL;
  }

  // Create the Producer task (Hardware polling)
  if (xTaskCreate(input_reader_task, "input_reader_task", 2048, NULL, 1, NULL) != pdPASS)
  {
    ESP_LOGE(TAG, "%s:Fail to create input reader task", __func__);
    return ESP_FAIL;
  }

  // Create the Consumer task (Event dispatching)
  if (xTaskCreate(event_dispatcher_task, "event_dispatcher_task", 2048, NULL, 2, NULL) != pdPASS)
  {
    ESP_LOGE(TAG, "%s:Fail to create event dispatcher task", __func__);
    return ESP_FAIL;
  }

  // Hardware configuration: Inputs with Internal Pull-up
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
  if (xSemaphoreTake(s_node_mutex, portMAX_DELAY) != pdTRUE)
  {
    return ESP_FAIL;
  }

  esp_err_t err = add_node(&s_first_node, handler);

  xSemaphoreGive(s_node_mutex);

  return err;
}

digital_input_state_t digital_input_get_state(digital_input_num_t num)
{
  if (xSemaphoreTake(s_input_states_mutex, pdMS_TO_TICKS(100)) != pdTRUE)
  {
    ESP_LOGE(TAG, "%s:Fail to take input states mutex", __func__);
    return DIGITAL_INPUT_STATE_FAIL;
  }

  bool state = GET_BIT(s_input_states, num);

  xSemaphoreGive(s_input_states_mutex);

  return state ? DIGITAL_INPUT_STATE_ON : DIGITAL_INPUT_STATE_OFF;
}

//**************************************************
// Static Funtions
//**************************************************

/**
 * @brief Producer Task: Polls GPIOs, detects edges, and queues events.
 */
static void input_reader_task(void *args)
{
  TickType_t last_wake_time = xTaskGetTickCount();

  while (true)
  {
    // Polling interval (acts as a basic debounce)
    xTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(50));

    if (xSemaphoreTake(s_input_states_mutex, pdMS_TO_TICKS(250)) != pdTRUE)
    {
      ESP_LOGE(TAG, "%s:Fail to take input states mutex", __func__);
      continue;
    }

    for (uint16_t i = 0; i < _DIGITAL_INPUT_NUM_MAX; i++)
    {
      // Read hardware (inverted because of internal Pull-up)
      bool level = !gpio_get_level(s_input_num_map[i]);

      // Edge detection: skip if state has not changed
      if (GET_BIT(s_input_states, i) == level)
      {
        continue;
      }

      input_queue_data_t data = {
          .num = i,
          .new_state = level,
      };

      // Queue the event for the dispatcher task
      if (xQueueSend(s_input_queue, &data, pdMS_TO_TICKS(250)) != pdTRUE)
      {
        ESP_LOGE(TAG, "%s:Fail to send input data to queue", __func__);
        continue;
      }

      // Update internal state bitmask
      if (level)
      {
        SET_BIT(s_input_states, i);
      }
      else
      {
        CLEAR_BIT(s_input_states, i);
      }
    }

    xSemaphoreGive(s_input_states_mutex);
  }

  vTaskDelete(NULL);
}

/**
 * @brief Consumer Task: Waits for queued events and notifies all observers.
 */
static void event_dispatcher_task(void *args)
{
  input_queue_data_t data;

  while (true)
  {
    // Blocks until an event arrives in the queue
    if (xQueueReceive(s_input_queue, &data, portMAX_DELAY) != pdTRUE)
    {
      continue;
    }

    if (xSemaphoreTake(s_node_mutex, portMAX_DELAY) != pdTRUE)
    {
      ESP_LOGE(TAG, "%s:Fail to take node mutex", __func__);
      continue;
    }

    // Notify all registered observers
    foreach_node(s_first_node, data.num, data.new_state);

    xSemaphoreGive(s_node_mutex);
  }

  vTaskDelete(NULL);
}

/**
 * @brief Search for a specific handler in the list to avoid duplicate entries.
 * @return ESP_OK if found, ESP_FAIL otherwise.
 */
static esp_err_t is_handler_present(event_node_t *head, digital_input_event_handler_t handler)
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
static esp_err_t add_node(event_node_t **head, digital_input_event_handler_t handler)
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
static esp_err_t foreach_node(event_node_t *head, const digital_input_num_t num, const bool state)
{
  event_node_t *current = head;
  while (current != NULL)
  {
    if (current->handler != NULL)
    {
      current->handler(num, state);
    }
    current = current->next;
  }

  return ESP_OK;
}