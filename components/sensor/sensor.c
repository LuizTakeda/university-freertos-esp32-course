#include "sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <dht.h>

//**************************************************
// Typedefs
//**************************************************

//**************************************************
// Funtion Prototypes
//**************************************************

void sensor_reader_task();

//**************************************************
// Globals
//**************************************************

static const char TAG[] = "sensor";

//**************************************************
// Public Functions
//**************************************************

esp_err_t sensor_initialize()
{
  return ESP_OK;
}

esp_err_t sensor_add_event_handler(sensor_event_handler_t handler){

  return ESP_OK;
}

//**************************************************
// Static Functions
//**************************************************

void sensor_reader_task()
{
}