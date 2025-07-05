#include <stdio.h>
#include <sys/param.h>

#include "esp_http_server.h"
#include "esp_log.h"

#include "web_server.h"

//**************************************************
// Define
//**************************************************

//**************************************************
// Static Function Prototypes
//**************************************************

static esp_err_t get_handler(httpd_req_t *req);

static esp_err_t post_handler(httpd_req_t *req);

//**************************************************
// Globals
//**************************************************

static char TAG[] = "web-server";

static httpd_handle_t server = NULL;

// ##### endpoitns #####

/* URI handler structure for GET /uri */
httpd_uri_t uri_get = {
    .uri = "/uri",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = NULL};

/* URI handler structure for POST /uri */
httpd_uri_t uri_post = {
    .uri = "/uri",
    .method = HTTP_POST,
    .handler = post_handler,
    .user_ctx = NULL};

//**************************************************
// Funtions
//**************************************************

/**
 * Starts the web server
 */
bool web_server_start()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  if (httpd_start(&server, &config) != ESP_OK)
  {
    return false;
  }

  httpd_register_uri_handler(server, &uri_get);
  httpd_register_uri_handler(server, &uri_post);

  ESP_LOGI(TAG, "server started");

  return true;
}

/**
 * Stops the web server
 */
void web_server_stop()
{
  if (server)
  {
    httpd_stop(server);
    ESP_LOGI(TAG, "server stopped");
  }
}

//**************************************************
// Static Functions
//**************************************************

/**
 * Get exemple
 */
esp_err_t get_handler(httpd_req_t *req)
{
  ESP_LOGI(TAG, "get");

  const char resp[] = "URI GET Response";
  httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

/**
 * Post exemple
 */
esp_err_t post_handler(httpd_req_t *req)
{
  ESP_LOGI(TAG, "post");

  /* Destination buffer for content of HTTP POST request.
   * httpd_req_recv() accepts char* only, but content could
   * as well be any binary data (needs type casting).
   * In case of string data, null termination will be absent, and
   * content length would give length of string */
  char content[100];

  /* Truncate if content length larger than the buffer */
  size_t recv_size = MIN(req->content_len, sizeof(content));

  int ret = httpd_req_recv(req, content, recv_size);
  if (ret <= 0)
  { /* 0 return value indicates connection closed */
    /* Check if timeout occurred */
    if (ret == HTTPD_SOCK_ERR_TIMEOUT)
    {
      /* In case of timeout one can choose to retry calling
       * httpd_req_recv(), but to keep it simple, here we
       * respond with an HTTP 408 (Request Timeout) error */
      httpd_resp_send_408(req);
    }
    /* In case of error, returning ESP_FAIL will
     * ensure that the underlying socket is closed */
    return ESP_FAIL;
  }

  /* Send a simple response */
  const char resp[] = "URI POST Response";
  httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}
