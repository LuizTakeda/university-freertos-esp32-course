#include "analog_input.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_adc/adc_oneshot.h"

//**************************************************
// Typedefs
//**************************************************

/**
 * @brief Linked list node representing a registered observer (handler).
 */
typedef struct event_node_t
{
  struct event_node_t *next;
  analog_input_event_handler_t handler;
} event_node_t;

//**************************************************
// Function Prototypes
//**************************************************

static void analog_reader_task(void *args);

static esp_err_t is_handler_present(event_node_t *head, analog_input_event_handler_t handler);
static esp_err_t foreach_node(event_node_t *head, const analog_input_num_t num, const uint16_t value);
static esp_err_t add_node(event_node_t **head, analog_input_event_handler_t handler);

//**************************************************
// Globals
//**************************************************

static const char TAG[] = "analog_input";

static event_node_t *s_first_event_node = NULL;     /**< Head pointer of the handlers list */
static SemaphoreHandle_t s_event_node_mutex = NULL; /**< Mutex for thread-safe list access */
static adc_oneshot_unit_handle_t s_adc1_handler;    /**< Handle for the ADC unit */

/**
 * @brief Maps logical input IDs to physical ADC channels.
 */
static const uint16_t s_analog_input_num_map[] = {ADC_CHANNEL_6, ADC_CHANNEL_7};

//**************************************************
// Public Functions
//**************************************************

esp_err_t analog_input_initialize(void)
{
  // Initialize synchronization primitive
  if ((s_event_node_mutex = xSemaphoreCreateMutex()) == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to create event node mutex", __func__);
    return ESP_FAIL;
  }

  // Create the background task for periodic sampling
  if (xTaskCreate(analog_reader_task, "analog_reader_task", 2048, NULL, 1, NULL) != pdPASS)
  {
    ESP_LOGE(TAG, "%s:Fail to create analog reader task", __func__);
    return ESP_FAIL;
  }

  // Initialize ADC Unit 1 configuration
  adc_oneshot_unit_init_cfg_t init_config = {
      .unit_id = ADC_UNIT_1,
  };

  if (adc_oneshot_new_unit(&init_config, &s_adc1_handler) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Fail to init oneshot adc1", __func__);
    return ESP_FAIL;
  }

  // Configure specific channels with default bitwidth and attenuation
  adc_oneshot_chan_cfg_t config = {
      .bitwidth = ADC_BITWIDTH_DEFAULT,
      .atten = ADC_ATTEN_DB_12,
  };

  for (int i = 0; i < _ANALOG_INPUT_NUM_MAX; i++)
  {
    if (adc_oneshot_config_channel(s_adc1_handler, s_analog_input_num_map[i], &config) != ESP_OK)
    {
      ESP_LOGE(TAG, "%s:Fail to config channel %d", __func__, i);
      return ESP_FAIL;
    }
  }

  return ESP_OK;
}

esp_err_t analog_input_add_event_handler(analog_input_event_handler_t handler)
{
  if (handler == NULL)
  {
    return ESP_ERR_INVALID_ARG;
  }

  // Secure the list modification with a mutex
  if (xSemaphoreTake(s_event_node_mutex, portMAX_DELAY) != pdTRUE)
  {
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
 * @brief Periodic task that samples ADC channels and notifies all observers.
 */
static void analog_reader_task(void *args)
{
  TickType_t last_wake_time = xTaskGetTickCount();

  while (true)
  {
    // Run loop every 50ms
    xTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(50));

    // Lock list during the notification process
    if ((xSemaphoreTake(s_event_node_mutex, portMAX_DELAY)) != pdTRUE)
    {
      continue;
    }

    for (int i = 0; i < _ANALOG_INPUT_NUM_MAX; i++)
    {
      int raw = 0;
      // Only notify if ADC conversion was successful
      if (adc_oneshot_read(s_adc1_handler, s_analog_input_num_map[i], &raw) == ESP_OK)
      {
        foreach_node(s_first_event_node, i, raw);
      }
      else
      {
        ESP_LOGE(TAG, "%s:Fail to reac adc %d", __func__, i);
      }
    }

    xSemaphoreGive(s_event_node_mutex);
  }

  vTaskDelete(NULL);
}

/**
 * @brief Search for a specific handler in the list to avoid duplicate entries.
 * @return ESP_OK if found, ESP_FAIL otherwise.
 */
static esp_err_t is_handler_present(event_node_t *head, analog_input_event_handler_t handler)
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
static esp_err_t add_node(event_node_t **head, analog_input_event_handler_t handler)
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
static esp_err_t foreach_node(event_node_t *head, const analog_input_num_t num, const uint16_t value)
{
  event_node_t *current = head;
  while (current != NULL)
  {
    if (current->handler != NULL)
    {
      current->handler(num, value);
    }
    current = current->next;
  }

  return ESP_OK;
}