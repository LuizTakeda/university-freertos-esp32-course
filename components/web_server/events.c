#include "web_server_internals.h"

#include "esp_http_server.h"
#include "esp_log.h"

#include <time.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

//**************************************************
// Typedefs
//**************************************************

typedef struct req_node_t
{
  struct req_node_t *prev;
  httpd_req_t *req;
  struct req_node_t *next;
} req_node_t;

//**************************************************
// Function Prototypes
//**************************************************

static void events_task();

static esp_err_t events_handler(httpd_req_t *req);
static esp_err_t is_req_present(req_node_t *head, httpd_req_t *req);
static esp_err_t add_node(req_node_t **head, httpd_req_t *req);
static httpd_req_t *pop_node(req_node_t **node);

//**************************************************
// Globals
//**************************************************

static const char TAG[] = "web_server:events";

static const httpd_uri_t s_uri_get_events = {
    .uri = "/api/events",
    .method = HTTP_GET,
    .user_ctx = NULL,
    .handler = events_handler,
};

static req_node_t *s_first_req_node = NULL;

static SemaphoreHandle_t s_req_node_mutex = NULL;

static QueueHandle_t s_events_queue = NULL;

//**************************************************
// Public Functions
//**************************************************

esp_err_t events_register(httpd_handle_t server)
{
  if ((s_req_node_mutex = xSemaphoreCreateMutex()) == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to create req node mutex", __func__);
    return ESP_FAIL;
  }

  if ((s_events_queue = xQueueCreate(sizeof(event_t), 10)) == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to create events queue", __func__);
    return ESP_FAIL;
  }

  if (xTaskCreate(events_task, "events_task", 4096, NULL, 3, NULL) != pdTRUE)
  {
    ESP_LOGE(TAG, "%s:Fail to create event task", __func__);
    return ESP_FAIL;
  }

  if (httpd_register_uri_handler(server, &s_uri_get_events) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Fail to register uri handler", __func__);
    return ESP_FAIL;
  }

  return ESP_OK;
}

esp_err_t events_send(event_t *event)
{
  return xQueueSend(s_events_queue, event, pdMS_TO_TICKS(250)) == pdTRUE ? ESP_OK : ESP_FAIL;
}

//**************************************************
// Static Functions
//**************************************************

/**
 * @brief Task that consumes events from the queue and sends them to all
 *        registered SSE clients. Handles automatic cleanup of dead connections.
 */
static void events_task()
{
  static char buf[256] = "";
  event_t event;

  while (1)
  {
    if (xQueueReceive(s_events_queue, &event, portMAX_DELAY) != pdTRUE)
    {
      continue;
    }

    if (xSemaphoreTake(s_req_node_mutex, portMAX_DELAY) != pdTRUE)
    {
      ESP_LOGE(TAG, "%s:Fail to take req node mutex", __func__);
      continue;
    }

    // 1. Format the SSE message based on event type
    switch (event.name)
    {
    case EVENT_NAME_DIGITAL_INPUT:
      snprintf(buf, sizeof(buf),
               "event: digital-input\n"
               "data: {\"num\":%d, \"value\":%d}\n\n",
               event.payload.digital_input.num, event.payload.digital_input.value);
      break;

    case EVENT_NAME_ANALOG_INPUT:
      snprintf(buf, sizeof(buf),
               "event: analog-input\n"
               "data: {\"num\":%d, \"value\":%d}\n\n",
               event.payload.analog_input.num, event.payload.analog_input.value);
      break;

    case EVENT_NAME_SENSOR:
      snprintf(buf, sizeof(buf),
               "event: analog-input\n"
               "data: {\"num\":%d, \"value\":%f}\n\n"
               "event: analog-input\n"
               "data: {\"num\":%d, \"value\":%f}\n\n",
               2, event.payload.sensor.temperature,
               3, event.payload.sensor.humidity);
      break;

    default:
      ESP_LOGE(TAG, "%s:Invalid event name", __func__);
      goto end;
    }

    // 2. Iterate through clients and send the formatted chunk
    req_node_t *node = s_first_req_node;
    while (node != NULL)
    {
      if (httpd_resp_send_chunk(node->req, buf, HTTPD_RESP_USE_STRLEN) != ESP_OK)
      {
        ESP_LOGE(TAG, "%s:Fail to send chunk", __func__);
        httpd_req_t *req = pop_node(&node);

        httpd_resp_send_chunk(req, NULL, 0);
        httpd_req_async_handler_complete(req);
        continue; // 'node' was updated by pop_node
      }

      node = node->next;
    }

  end:
    xSemaphoreGive(s_req_node_mutex);
  }

  vTaskDelete(NULL);
}

/**
 * @brief Handles incoming GET requests for SSE. Upgrades the connection
 *        to asynchronous and adds it to the list.
 */
static esp_err_t events_handler(httpd_req_t *req)
{
  httpd_req_t *async_req = NULL;

  httpd_resp_set_type(req, "text/event-stream");
  httpd_resp_set_hdr(req, "Cache-Control", "no-cache, no-transform");
  httpd_resp_set_hdr(req, "Connection", "keep-alive");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  if (httpd_req_async_handler_begin(req, &async_req) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Failed to create async req", __func__);
    return ESP_ERR_NO_MEM;
  }

  if (xSemaphoreTake(s_req_node_mutex, portMAX_DELAY) != pdTRUE)
  {
    ESP_LOGE(TAG, "%s:Fail to take mutex", __func__);
    httpd_req_async_handler_complete(async_req);
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  esp_err_t err = add_node(&s_first_req_node, async_req);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Failed to add node to list", __func__);
    httpd_req_async_handler_complete(async_req);
    httpd_resp_send_500(req);
  }

  xSemaphoreGive(s_req_node_mutex);

  return err;
}

/**
 * @brief Checks if a request is already present in the linked list.
 */
static esp_err_t is_req_present(req_node_t *head, httpd_req_t *req)
{
  req_node_t *node = head;

  while (node != NULL)
  {
    if (node->req == req)
    {
      return ESP_OK;
    }

    node = node->next;
  }

  return ESP_FAIL;
}

/**
 * @brief Allocates and adds a new node to the head of the doubly linked list.
 */
static esp_err_t add_node(req_node_t **head, httpd_req_t *req)
{
  if (req == NULL || head == NULL)
  {
    return ESP_ERR_INVALID_ARG;
  }

  if (is_req_present(*head, req) == ESP_OK)
  {
    return ESP_OK;
  }

  req_node_t *new_node = malloc(sizeof(req_node_t));
  if (new_node == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to alloc new node", __func__);
    return ESP_ERR_NO_MEM;
  }

  new_node->req = req;
  new_node->prev = NULL;
  new_node->next = *head;

  if (*head != NULL)
  {
    (*head)->prev = new_node;
  }

  *head = new_node;
  return ESP_OK;
}

/**
 * @brief Removes a specific node from the doubly linked list and frees its memory.
 *        Updates the pointer to the next node for iteration safety.
 */
static httpd_req_t *pop_node(req_node_t **node_ptr)
{
  if (node_ptr == NULL || *node_ptr == NULL)
  {
    return NULL;
  }

  req_node_t *to_remove = *node_ptr;
  req_node_t *prev_node = to_remove->prev;
  req_node_t *next_node = to_remove->next;

  httpd_req_t *req = to_remove->req;

  if (prev_node != NULL)
  {
    prev_node->next = next_node;
  }

  if (next_node != NULL)
  {
    next_node->prev = prev_node;
  }

  *node_ptr = next_node;
  free(to_remove);
  return req;
}