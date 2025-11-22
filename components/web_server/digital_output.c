#include "web_server_internals.h"

#include "esp_http_server.h"
#include "esp_log.h"
#include "digital_output.h"

#include "cJSON.h"

//**************************************************
// Function Prototypes
//**************************************************

static esp_err_t get_digital_output_handler(httpd_req_t *req);
static esp_err_t post_digital_output_handler(httpd_req_t *req);

//**************************************************
// Globals
//**************************************************

const static char TAG[] = "web_server:digital_output";

httpd_uri_t uri_get_digital_output = {
    .uri = "/api/digital-output",
    .method = HTTP_GET,
    .handler = get_digital_output_handler,
    .user_ctx = NULL};

httpd_uri_t uri_post_digital_output = {
    .uri = "/api/digital-output",
    .method = HTTP_POST,
    .handler = post_digital_output_handler,
    .user_ctx = NULL};

//**************************************************
// Public Functions
//**************************************************

esp_err_t digital_output_register(httpd_handle_t server)
{
  httpd_register_uri_handler(server, &uri_get_digital_output);
  httpd_register_uri_handler(server, &uri_post_digital_output);

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
    ESP_LOGE(TAG, "%s:Fail to get query", __func__);
    httpd_resp_send_500(req);
    goto end;
  }

  ESP_LOGI(TAG, "%s:Data (%s)", __func__, buf);

  uint32_t num;
  char value[6];
  if (httpd_query_key_value(buf, "id", value, sizeof(value)) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Id not found", __func__);
    httpd_resp_send_500(req);
    goto end;
  }

  num = atoi(value);

  digital_output_state_t state;
  switch (num)
  {
  case 0:
    state = digital_output_get_state(DIGITAL_OUTPUT_NUM_1);
    break;
  case 1:
    state = digital_output_get_state(DIGITAL_OUTPUT_NUM_2);
    break;
  case 2:
    state = digital_output_get_state(DIGITAL_OUTPUT_NUM_3);
    break;
  case 3:
    state = digital_output_get_state(DIGITAL_OUTPUT_NUM_4);
    break;

  default:
    ESP_LOGE(TAG, "%s:Invalid Id", __func__);
    goto end;
  }

  if (state == DIGITAL_OUTPUT_FAIL)
  {
    ESP_LOGE(TAG, "%s:Fail to get output state", __func__);
    httpd_resp_send_500(req);
    goto end;
  }

  char str[16];
  sprintf(str, "{\"state\":%d}", state == DIGITAL_OUTPUT_ON ? 1 : 0);
  httpd_resp_send(req, str, HTTPD_RESP_USE_STRLEN);

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

  char *query_buf = NULL;
  char *body_buf = NULL;
  cJSON *body_json = NULL;
  const cJSON *state = NULL;

  // START - Allocate resources
  size_t query_len = httpd_req_get_url_query_len(req);
  if (query_len <= 0 || query_len >= 512)
  {
    ESP_LOGE(TAG, "%s:Query len invalid", __func__);
    goto end;
  }

  if ((query_buf = malloc(query_len + 1)) == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to alloc query buf", __func__);
    goto end;
  }

  size_t body_len = req->content_len;
  if (body_len <= 0 || body_len >= 1024)
  {
    ESP_LOGE(TAG, "%s:Data len invalid", __func__);
    goto end;
  }

  if ((body_buf = malloc(body_len + 1)) == NULL)
  {
    ESP_LOGE(TAG, "%s:Fail to alloc body buf", __func__);
    goto end;
  }
  // END

  // START - Read values
  if (httpd_req_get_url_query_str(req, query_buf, query_len + 1) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s: Fail to get query str", __func__);
    goto end;
  }

  ESP_LOGI(TAG, "%s:Query (%s)", __func__, query_buf);

  char id_str[6];
  if (httpd_query_key_value(query_buf, "id", id_str, sizeof(id_str)) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Id not found", __func__);
    goto end;
  }

  int offset = 0;
  int bytes_read = 0;

  while ((bytes_read = httpd_req_recv(req, body_buf + offset, body_len - offset)) > 0)
  {
    offset += bytes_read;

    if (body_len - offset < 0)
    {
      ESP_LOGE(TAG, "%s:Body content too long", __func__);
      goto end;
    }
  }
  body_buf[offset] = '\0';

  ESP_LOGI(TAG, "%s:Body (%s)", __func__, body_buf);

  if ((body_json = cJSON_Parse(body_buf)) == NULL)
  {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL)
    {
      ESP_LOGE(TAG, "%s:Error before: %s", __func__, error_ptr);
    }
    goto end;
  }

  state = cJSON_GetObjectItemCaseSensitive(body_json, "state");
  if (state == NULL || !cJSON_IsBool(state))
  {
    ESP_LOGE(TAG, "%s:State not found", __func__);
    goto end;
  }

  bool new_state = cJSON_IsTrue(state);
  // END

  esp_err_t result;
  switch (atoi(id_str))
  {
  case 0:
    result = digital_output_set_state(DIGITAL_OUTPUT_NUM_1, new_state);
    break;
  case 1:
    result = digital_output_set_state(DIGITAL_OUTPUT_NUM_2, new_state);
    break;
  case 2:
    result = digital_output_set_state(DIGITAL_OUTPUT_NUM_3, new_state);
    break;
  case 3:
    result = digital_output_set_state(DIGITAL_OUTPUT_NUM_4, new_state);
    break;

  default:
    ESP_LOGE(TAG, "%s:Invalid Id", __func__);
    goto end;
  }

  if (result != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Fail to set output state", __func__);
    goto end;
  }

  httpd_resp_send(req, body_buf, HTTPD_RESP_USE_STRLEN);

  return_value = ESP_OK;
end:
  // START - Free resources
  if (query_buf)
  {
    free(query_buf);
  }
  if (body_buf)
  {
    free(body_buf);
  }
  if (body_json)
  {
    cJSON_Delete(body_json);
  }
  // END

  if (return_value != ESP_OK)
  {
    httpd_resp_send_500(req);
  }

  return return_value;
}