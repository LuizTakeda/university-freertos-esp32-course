#include "web_server_internals.h"

#include "esp_http_server.h"
#include "esp_log.h"
#include "digital_input.h"

#include "cJSON.h"

//**************************************************
// Function Prototypes
//**************************************************

static esp_err_t get_digital_input_handler(httpd_req_t *req);

//**************************************************
// Globals
//**************************************************

static const char TAG[] = "web_server:digital_input";

static const httpd_uri_t s_uri_get_digital_input = {
    .uri = "/api/digital-input",
    .method = HTTP_GET,
    .user_ctx = NULL,
    .handler = get_digital_input_handler,
};

//**************************************************
// Public Functions
//**************************************************

esp_err_t digital_input_register(httpd_handle_t server)
{
  ESP_ERROR_CHECK(httpd_register_uri_handler(server, &s_uri_get_digital_input));

  return ESP_OK;
}

//**************************************************
// Static Functions
//**************************************************

static esp_err_t get_digital_input_handler(httpd_req_t *req)
{
  esp_err_t return_value = ESP_FAIL;
  char *buf = NULL;

  size_t query_len = httpd_req_get_url_query_len(req);
  if (query_len <= 0 || query_len >= 256)
  {
    ESP_LOGE(TAG, "%s:Invalid query len", __func__);
    goto end;
  }

  if ((buf = malloc(query_len + 1)) == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to alloc query buf", __func__);
    goto end;
  }

  if (httpd_req_get_url_query_str(req, buf, query_len + 1) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Fail to get query str", __func__);
    goto end;
  }

  int id;
  char value[6];
  if (httpd_query_key_value(buf, "id", value, sizeof(value)) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Fail to get id from query str", __func__);
    goto end;
  }
  id = atoi(value);

  if (id < 0 || id >= _DIGITAL_INPUT_NUM_MAX)
  {
    ESP_LOGE(TAG, "%s:Invalid id", __func__);
    goto end;
  }

  digital_input_state_t state = digital_input_get_state(id);

  if (state == DIGITAL_INPUT_STATE_FAIL)
  {
    ESP_LOGE(TAG, "%s: Fail to get the input state", __func__);
    goto end;
  }

  char str[16];
  sprintf(str, "{\"state\":%d}", state == DIGITAL_INPUT_STATE_ON);
  httpd_resp_send(req, str, HTTPD_RESP_USE_STRLEN);

  return_value = ESP_OK;

end:
  if (buf)
  {
    free(buf);
  }

  if (return_value != ESP_OK)
  {
    httpd_resp_send_500(req);
  }

  return return_value;
}