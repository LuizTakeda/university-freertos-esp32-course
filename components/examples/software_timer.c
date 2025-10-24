#include "examples.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

//**************************************************
// Defines
//**************************************************

#define WARMING_UP_MS_TIME 1000 * 5 // 5 segundos
#define BAKING_MS_TIME 1000 * 10    // 10 segundos

//**************************************************
// Enums
//**************************************************

typedef enum
{
  OVEN_STATE_START = 0,
  OVEN_STATE_WARMING_UP,
  OVEN_STATE_BAKING,
  OVEN_STATE_DONE,
} oven_state_t;

//**************************************************
// Globals
//**************************************************

static const char TAG[] = "oven";

static TaskHandle_t oven_task_handle = NULL; // Handle da task principal do forno

//**************************************************
// Function Prototypes
//**************************************************

static void oven_task(void *_);
static void oven_timer(TimerHandle_t xTimer);
static void get_temperature();
static void set_heat_element();

//**************************************************
// Functions
//**************************************************

esp_err_t examples_software_timer()
{
  // Cria a task principal do forno
  BaseType_t res = xTaskCreate(
      oven_task,
      "oven_task",
      2048,
      NULL,
      1,
      &oven_task_handle);

  if (res != pdPASS)
  {
    ESP_LOGE(TAG, "Fail to create task");
    return ESP_FAIL;
  }

  return ESP_OK;
}

//**************************************************
// Static Functions
//**************************************************

static void oven_task(void *_)
{
  oven_state_t current_state = OVEN_STATE_START;

  while (1)
  {
    switch (current_state)
    {
    case OVEN_STATE_START:
      ESP_LOGI(TAG, "Start");
      current_state = OVEN_STATE_WARMING_UP;
      break;

    case OVEN_STATE_WARMING_UP:
    {
      ESP_LOGI(TAG, "Warming Up");

      // Cria um timer one-shot (pdFALSE) que dispara após 5s
      TimerHandle_t warming_up_timer = xTimerCreate(
          "oven_timer",
          pdMS_TO_TICKS(WARMING_UP_MS_TIME),
          pdFALSE, // one-shot, não periódico
          NULL,
          oven_timer);

      // Inicia o timer
      BaseType_t res = xTimerStart(warming_up_timer, portMAX_DELAY);
      if (res != pdTRUE)
      {
        ESP_LOGE(TAG, "Fail to start timer");
      }

      while (1)
      {
        // Espera notificação do timer (com timeout de 500ms para processar outras ações)
        if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(500)))
        {
          ESP_LOGI(TAG, "Warming up done");
          current_state = OVEN_STATE_BAKING;

          // Apaga o timer depois de usar
          res = xTimerDelete(warming_up_timer, portMAX_DELAY);
          if (res != pdTRUE)
          {
            ESP_LOGE(TAG, "Fail to delete timer");
          }
          break;
        }

        get_temperature();   // Simula leitura de temperatura
        set_heat_element();  // Simula controle da resistência
      }
    }
    break;

    case OVEN_STATE_BAKING:
    {
      ESP_LOGI(TAG, "Baking");

      // Cria um timer one-shot que dispara após 10s
      TimerHandle_t baking_timer = xTimerCreate(
          "oven_timer",
          pdMS_TO_TICKS(BAKING_MS_TIME),
          pdFALSE,
          NULL,
          oven_timer);

      // Inicia o timer
      BaseType_t res = xTimerStart(baking_timer, portMAX_DELAY);
      if (res != pdTRUE)
      {
        ESP_LOGE(TAG, "Fail to start timer");
      }

      while (1)
      {
        // Espera notificação do timer
        if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(500)))
        {
          ESP_LOGI(TAG, "Baking done");
          current_state = OVEN_STATE_DONE;

          // Apaga o timer depois de usar
          res = xTimerDelete(baking_timer, portMAX_DELAY);
          if (res != pdTRUE)
          {
            ESP_LOGE(TAG, "Fail to delete timer");
          }
          break;
        }

        get_temperature();
        set_heat_element();
      }
    }
    break;

    case OVEN_STATE_DONE:
      ESP_LOGI(TAG, "Done");
      vTaskDelete(NULL); // Encerra a task principal
      return;

    default:
      break;
    }
  }

  vTaskDelete(NULL);
}

// Callback chamado quando o timer expira
static void oven_timer(TimerHandle_t xTimer)
{
  if (oven_task_handle != NULL)
  {
    // Notifica a task principal do forno
    xTaskNotifyGive(oven_task_handle);
  }
}

static void get_temperature()
{
  ESP_LOGI(TAG, "Read temperature");
}

static void set_heat_element()
{
  ESP_LOGI(TAG, "Set heat element");
}