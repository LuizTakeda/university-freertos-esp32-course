#include "web_server_internals.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "digital_output.h"
#include "cJSON.h"

//**************************************************
// Function Prototypes
//**************************************************

static bool is_id_valid(int id);

static esp_err_t get_digital_output_handler(httpd_req_t *req);
static esp_err_t post_digital_output_handler(httpd_req_t *req);

//**************************************************
// Globals
//**************************************************

const static char TAG[] = "web_server:digital_output";

static const httpd_uri_t s_uri_get_digital_output = {
    .uri = "/api/digital-output",
    .method = HTTP_GET,
    .handler = get_digital_output_handler,
    .user_ctx = NULL,
};

static const httpd_uri_t s_uri_post_digital_output = {
    .uri = "/api/digital-output",
    .method = HTTP_POST,
    .handler = post_digital_output_handler,
    .user_ctx = NULL,
};

//**************************************************
// Public Functions
//**************************************************

esp_err_t digital_output_register(httpd_handle_t server)
{
  if (httpd_register_uri_handler(server, &s_uri_get_digital_output) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Fail to register uri handler", __func__);
    return ESP_FAIL;
  }

  if (httpd_register_uri_handler(server, &s_uri_post_digital_output) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Fail to register uri handler", __func__);
    return ESP_FAIL;
  }

  return ESP_OK;
}

//**************************************************
// Static Functions
//**************************************************

static bool is_id_valid(int id)
{
  return (id >= 0 && id < _DIGITAL_OUTPUT_NUM_MAX);
}

/**
 * @brief Handles GET requests. Parses the 'id' query parameter and returns 
 *        the hardware state as a JSON object.
 */
static esp_err_t get_digital_output_handler(httpd_req_t *req)
{
  char query[64];
  char id_str[8];

  if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK ||
      httpd_query_key_value(query, "id", id_str, sizeof(id_str)) != ESP_OK)
  {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid query");
    return ESP_FAIL;
  }

  int id = atoi(id_str);
  if (!is_id_valid(id))
  {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid ID");
    return ESP_FAIL;
  }

  digital_output_state_t state = digital_output_get_state((digital_output_num_t)id);

  char resp[32];
  snprintf(resp, sizeof(resp), "{\"state\":%d}", state == DIGITAL_OUTPUT_ON);
  httpd_resp_set_type(req, "application/json");
  return httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
}

/**
 * @brief Handles POST requests. Parses JSON body to set the new state of a GPIO.
 *        Expected JSON: {"state": true/false}
 */
static esp_err_t post_digital_output_handler(httpd_req_t *req)
{
  char query[64], id_str[8], body[128];

  // 1. Parse Query for ID
  if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK ||
      httpd_query_key_value(query, "id", id_str, sizeof(id_str)) != ESP_OK)
  {
    return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing ID");
  }

  // 2. Read Body
  int ret = httpd_req_recv(req, body, sizeof(body) - 1);
  if (ret <= 0)
  {
    return ESP_FAIL;
  }
  body[ret] = '\0';

  // 3. Parse JSON
  cJSON *json = cJSON_Parse(body);
  const cJSON *state_item = cJSON_GetObjectItemCaseSensitive(json, "state");
  bool new_state = cJSON_IsBool(state_item) && cJSON_IsTrue(state_item);
  cJSON_Delete(json);

  // 4. Action
  int id = atoi(id_str);
  if (!is_id_valid(id) || digital_output_set_state((digital_output_num_t)id, new_state) != ESP_OK)
  {
    return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Action failed");
  }

  return httpd_resp_send(req, body, HTTPD_RESP_USE_STRLEN);
}