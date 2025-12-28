#include "web_server_internals.h"

#include "esp_http_server.h"
#include "esp_log.h"
#include "digital_input.h"

#include "cJSON.h"

//**************************************************
// Function Prototypes
//**************************************************

static esp_err_t get_digital_input_handler(httpd_req_t *req);

static void input_event_handler(const digital_input_num_t num, const bool state);

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
  if (httpd_register_uri_handler(server, &s_uri_get_digital_input) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Fail to register uri handler", __func__);
    return ESP_FAIL;
  }

  if (digital_input_add_event_handler(input_event_handler) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Fail to add event handler", __func__);
    return ESP_FAIL;
  }

  return ESP_OK;
}

//**************************************************
// Static Functions
//**************************************************

/**
 * @brief REST API Handler to get current state of a digital input.
 * Expects query param: ?id=X
 */
static esp_err_t get_digital_input_handler(httpd_req_t *req)
{
  char query[64]; // Static buffer, no malloc needed for small queries
  char value[8];

  // 1. Get query string from URL
  if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK)
  {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing query string");
    return ESP_FAIL;
  }

  // 2. Extract 'id' parameter
  if (httpd_query_key_value(query, "id", value, sizeof(value)) != ESP_OK)
  {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "ID parameter required");
    return ESP_FAIL;
  }

  int id = atoi(value);
  if (id < 0 || id >= _DIGITAL_INPUT_NUM_MAX)
  {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid Input ID");
    return ESP_FAIL;
  }

  // 3. Get hardware state
  digital_input_state_t state = digital_input_get_state(id);
  if (state == DIGITAL_INPUT_STATE_FAIL)
  {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  // 4. Send JSON response
  char response[32];
  snprintf(response, sizeof(response), "{\"state\":%d}", state == DIGITAL_INPUT_STATE_ON);
  httpd_resp_set_type(req, "application/json");
  return httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
}

/**
 * @brief Observer callback that pushes hardware changes to the web event system (WebSocket/SSE).
 */
static void input_event_handler(const digital_input_num_t num, const bool state)
{
  event_t event = {
      .name = EVENT_NAME_DIGITAL_INPUT,
      .payload.digital_input = {
          .num = num,
          .value = state,
      },
  };

  if (events_send(&event) != ESP_OK)
  {
    ESP_LOGE(TAG, "%s:Fail to send event", __func__);
  }
}