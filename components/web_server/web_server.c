#include "web_server.h"
#include "web_server_internals.h"

#include <stdio.h>
#include <sys/param.h>

#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "lwip/err.h"
#include "lwip/sys.h"

//**************************************************
// Define
//**************************************************

//**************************************************
// Static Function Prototypes
//**************************************************

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

static esp_err_t start();

static esp_err_t stop();

static esp_err_t get_index_html_handler(httpd_req_t *req);

static esp_err_t get_bundle_js_handler(httpd_req_t *req);

//**************************************************
// Files
//**************************************************

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

extern const uint8_t bundle_js_start[] asm("_binary_bundle_js_start");
extern const uint8_t bundle_js_end[] asm("_binary_bundle_js_end");

//**************************************************
// Globals
//**************************************************

static char TAG[] = "web-server";

static httpd_handle_t server = NULL;

// ##### Endpoints #####

httpd_uri_t uri_get_index_html = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_index_html_handler,
    .user_ctx = NULL};

httpd_uri_t uri_get_bundle_js = {
    .uri = "/bundle.js",
    .method = HTTP_GET,
    .handler = get_bundle_js_handler,
    .user_ctx = NULL};

//**************************************************
// Funtions
//**************************************************

esp_err_t web_server_initialize()
{
  ESP_LOGE(TAG, "%s: Start", __func__);

  esp_err_t err = esp_event_loop_create_default();
  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
  {
    ESP_LOGE(TAG, "%s: Fail to create default event loop", __func__);
    return ESP_FAIL;
  }

  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                      WIFI_EVENT_STA_DISCONNECTED,
                                                      &wifi_event_handler,
                                                      NULL,
                                                      NULL));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                      IP_EVENT_STA_GOT_IP,
                                                      &wifi_event_handler,
                                                      NULL,
                                                      NULL));

  ESP_LOGE(TAG, "%s: Finish", __func__);

  return ESP_OK;
}

//**************************************************
// Static Functions
//**************************************************

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  if (event_base == WIFI_EVENT)
  {
    switch (event_id)
    {
    case WIFI_EVENT_STA_DISCONNECTED:
      stop();
      break;

    default:
      break;
    }
  }

  if (event_base == IP_EVENT)
  {
    switch (event_id)
    {
    case IP_EVENT_STA_GOT_IP:
      start();
      break;

    default:
      break;
    }
  }
}

static esp_err_t start()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  if (httpd_start(&server, &config) != ESP_OK)
  {
    return ESP_FAIL;
  }

  httpd_register_uri_handler(server, &uri_get_index_html);
  httpd_register_uri_handler(server, &uri_get_bundle_js);

  digital_output_register(server);

  ESP_LOGI(TAG, "server started");

  return ESP_OK;
}

static esp_err_t stop()
{
  if (server)
  {
    httpd_stop(server);
    ESP_LOGI(TAG, "server stopped");
  }
  return ESP_OK;
}

static esp_err_t get_index_html_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);
  return ESP_OK;
}

static esp_err_t get_bundle_js_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "application/javascript");
  httpd_resp_send(req, (const char *)bundle_js_start, bundle_js_end - bundle_js_start);
  return ESP_OK;
}
