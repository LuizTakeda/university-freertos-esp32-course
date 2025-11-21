#include "web_server_internals.h"

#include "esp_http_server.h"
#include "esp_log.h"

//**************************************************
// Function Prototypes
//**************************************************

static esp_err_t get_digital_output_handler(httpd_req_t *req);

//**************************************************
// Globals
//**************************************************

const static char TAG[] = "web_server:digital_output";

httpd_uri_t uri_get_digital_output = {
    .uri = "/api/digital-output",
    .method = HTTP_GET,
    .handler = get_digital_output_handler,
    .user_ctx = NULL};

//**************************************************
// Public Functions
//**************************************************

esp_err_t digital_output_register(httpd_handle_t server)
{
  httpd_register_uri_handler(server, &uri_get_digital_output);

  return ESP_OK;
}

//**************************************************
// Static Functions
//**************************************************

static esp_err_t get_digital_output_handler(httpd_req_t *req)
{
  esp_err_t return_value = ESP_FAIL;
  char *buf = NULL;

  size_t query_len = httpd_req_get_url_query_len(req);
  if (query_len <= 0 || query_len >= 1024)
  {
    ESP_LOGE(TAG, "%s:Query len invalid", __func__);
    httpd_resp_send_500(req);
    goto end;
  }

  buf = malloc(query_len + 1);
  if (buf == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to alloc buf", __func__);
    httpd_resp_send_500(req);
    goto end;
  }

  if (httpd_req_get_url_query_str(req, buf, query_len + 1) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Fail to get qury", __func__);
    httpd_resp_send_500(req);
    goto end;
  }

  ESP_LOGI(TAG, "%s:Data (%s)", __func__, buf);

  char value[6];
  if (httpd_query_key_value(buf, "id", value, sizeof(value)) == ESP_OK)
  {
  }

  httpd_resp_send(req, "{\"state\":1}", HTTPD_RESP_USE_STRLEN);

  return_value = ESP_OK;

end:
  if (buf)
  {
    free(buf);
  }

  return return_value;
}

static esp_err_t post_digital_output_handler(httpd_req_t *req)
{
  esp_err_t return_value = ESP_FAIL;
  char *buf = NULL;

  size_t data_len = req->content_len + 1;
  if (data_len <= 0 || data_len >= 1024)
  {
    ESP_LOGE(TAG, "%s:Data len invalid", __func__);
    httpd_resp_send_500(req);
    goto end;
  }

  buf = malloc(data_len);
  if (buf == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to alloc buf", __func__);
    httpd_resp_send_500(req);
    goto end;
  }

  int ret = httpd_req_recv(req, buf, data_len);
  if (ret <= 0)
  {
    if (ret == HTTPD_SOCK_ERR_TIMEOUT)
    {
      ESP_LOGE(TAG, "%s:Sock timeout", __func__);
      httpd_resp_send_408(req);
    }
    else
    {
      ESP_LOGE(TAG, "%s:Fail to recive data", __func__);
      httpd_resp_send_500(req);
    }
    goto end;
  }
  buf[data_len - 1] = '\0';

  ESP_LOGI(TAG, "%s:Data (%s)", __func__, buf);

  return_value = ESP_OK;

end:
  if (buf)
  {
    free(buf);
  }

  return return_value;
}