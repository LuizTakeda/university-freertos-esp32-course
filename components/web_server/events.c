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

  xTaskCreate(events_task, "events_task", 4096, NULL, 3, NULL);

  ESP_ERROR_CHECK(httpd_register_uri_handler(server, &s_uri_get_events));

  return ESP_OK;
}

esp_err_t events_send(const event_t event)
{
  return xQueueSend(s_events_queue, &event, pdMS_TO_TICKS(250));
}

//**************************************************
// Static Functions
//**************************************************

static void events_task()
{
  char buf[256] = "";
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

    switch (event.name)
    {
    case EVENT_NAME_DIGITAL_INPUT:
      sprintf(buf,
              "event: digital-input\n"
              "data: {\"num\":%d, \"value\":%d}\n\n",
              event.payload.digital_input.num, event.payload.digital_input.value);
      break;

    default:
      ESP_LOGE(TAG, "%s:Invalid event name", __func__);
      goto end;
    }

    req_node_t *node = s_first_req_node;
    while (node != NULL)
    {
      ESP_LOGI(TAG, "%s:Sending chunk \n%s", __func__, buf);

      if (httpd_resp_send_chunk(node->req, buf, HTTPD_RESP_USE_STRLEN) != ESP_OK)
      {
        ESP_LOGE(TAG, "%s:Fail to send chunk", __func__);
        req_node_t *invalid_node = node;

        if (invalid_node->prev == NULL)
        {
          node = invalid_node->next;
          if (node != NULL)
          {
            node->prev = NULL;
          }
          s_first_req_node = node;
        }
        else
        {
          node = invalid_node->next;
          invalid_node->prev->next = node;
          if (node != NULL)
          {
            node->prev = invalid_node->prev;
          }
        }

        httpd_resp_send_chunk(invalid_node->req, NULL, 0);
        httpd_req_async_handler_complete(invalid_node->req);
        free(invalid_node);
        continue;
      }

      node = node->next;
    }

  end:
    xSemaphoreGive(s_req_node_mutex);
  }

  vTaskDelete(NULL);
}

static esp_err_t events_handler(httpd_req_t *req)
{
  esp_err_t return_value = ESP_FAIL;
  httpd_req_t *async_req = NULL;
  req_node_t *new_node = NULL;

  httpd_resp_set_type(req, "text/event-stream");
  httpd_resp_set_hdr(req, "Cache-Control", "no-cache, no-transform");
  httpd_resp_set_hdr(req, "Connection", "keep-alive");

  if (httpd_req_async_handler_begin(req, &async_req) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Fail to create async req", __func__);
    goto end;
  }

  if ((new_node = malloc(sizeof(req_node_t))) == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to alloc req node", __func__);
    goto end;
  }

  new_node->prev = NULL;
  new_node->next = NULL;
  new_node->req = async_req;

  if (xSemaphoreTake(s_req_node_mutex, portMAX_DELAY) != pdTRUE)
  {
    ESP_LOGE(TAG, "%s:Fail to take req node mutex", __func__);
    goto end;
  }

  if (s_first_req_node == NULL)
  {
    s_first_req_node = new_node;
  }
  else
  {
    req_node_t *last_node = s_first_req_node;
    while (last_node->next != NULL)
    {
      last_node = last_node->next;
    }
    last_node->next = new_node;
    new_node->prev = last_node;
  }

  xSemaphoreGive(s_req_node_mutex);

  return_value = ESP_OK;
end:
  if (return_value != ESP_OK && new_node)
  {
    free(new_node);
  }

  if (return_value != ESP_OK && async_req)
  {
    httpd_resp_send_chunk(async_req, NULL, 0);
    httpd_req_async_handler_complete(async_req);
    httpd_resp_send_500(req);
  }

  return return_value;
}
