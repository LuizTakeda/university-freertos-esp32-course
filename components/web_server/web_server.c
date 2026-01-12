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

extern const uint8_t s_index_html_start[] asm("_binary_index_html_start");
extern const uint8_t s_index_html_end[] asm("_binary_index_html_end");
extern const uint8_t s_bundle_js_start[] asm("_binary_bundle_js_start");
extern const uint8_t s_bundle_js_end[] asm("_binary_bundle_js_end");

//**************************************************
// Globals
//**************************************************

static char TAG[] = "web-server";

static httpd_handle_t s_server = NULL;

// ##### Endpoints #####

static const httpd_uri_t s_uri_get_index_html = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_index_html_handler,
    .user_ctx = NULL};

static const httpd_uri_t s_uri_get_bundle_js = {
    .uri = "/bundle.js",
    .method = HTTP_GET,
    .handler = get_bundle_js_handler,
    .user_ctx = NULL};

//**************************************************
// Public Functions
//**************************************************

esp_err_t web_server_initialize()
{
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

  return ESP_OK;
}

//**************************************************
// Static Functions
//**************************************************

/**
 * @brief Dispatches Wi-Fi and IP events to start or stop the server.
 *        Ensures the server only runs when a network connection is active.
 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
    stop();
  }
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    start();
  }
}

/**
 * @brief Starts the HTTP server and registers all application-specific 
 *        URI handlers and hardware modules.
 * @return - ESP_OK: Server started and modules registered.
 * 
 *         - ESP_FAIL: Critical failure during server startup.
 */
static esp_err_t start()
{
  if (s_server != NULL)
  {
    return ESP_OK;
  }

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.lru_purge_enable = true;

  if (httpd_start(&s_server, &config) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Error starting server!", __func__);
    return ESP_FAIL;
  }

  // Registering Core Web Content
  httpd_register_uri_handler(s_server, &s_uri_get_index_html);
  httpd_register_uri_handler(s_server, &s_uri_get_bundle_js);

  // Registering Application Modules
  digital_output_register(s_server);
  digital_input_register(s_server);
  events_register(s_server);
  analog_input_register(s_server);
  sensor_register(s_server);
  sensor_register(s_server);

  return ESP_OK;
}

/**
 * @brief Stops the HTTP server and clears the server handle.
 * @return - ESP_OK: Server stopped successfully.
 */
static esp_err_t stop()
{
  if (s_server)
  {
    httpd_stop(s_server);
    s_server = NULL;
  }
  return ESP_OK;
}

/**
 * @brief Handler for the root path. Serves the embedded 'index.html' file.
 */
static esp_err_t get_index_html_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)s_index_html_start, s_index_html_end - s_index_html_start);
}

/**
 * @brief Handler for the JavaScript bundle. Serves the embedded 'bundle.js' file.
 */
static esp_err_t get_bundle_js_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "application/javascript");
  return httpd_resp_send(req, (const char *)s_bundle_js_start, s_bundle_js_end - s_bundle_js_start);
}
