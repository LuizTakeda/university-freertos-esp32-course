#include "examples.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "string.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "esp_err.h"

//**************************************************
// Defines
//**************************************************

#define UART_PORT UART_NUM_2       // Porta UART utilizada (UART2)
#define UART_BAUD 115200           // Baudrate da UART
#define UART_TX_PIN GPIO_NUM_17    // Pino de TX
#define UART_RX_PIN GPIO_NUM_16    // Pino de RX
#define BUF_SIZE 1024              // Tamanho do buffer de recepção

//**************************************************
// Globals
//**************************************************

static const char TAG[] = "task";  // Tag usada para logs

//**************************************************
// Function Prototypes
//**************************************************

void uart_initialize();            // Inicialização da UART
void uart_rx_task(void *pvParameters); // Task para receber dados via UART

//**************************************************
// Functions
//**************************************************

esp_err_t examples_task()
{
  // Inicializa UART2 com a configuração definida
  uart_initialize();

  // Cria a task de recepção da UART
  BaseType_t res = xTaskCreate(
      uart_rx_task,  // Função da task
      "uart_rx",     // Nome da task
      2048,          // Stack em bytes (ajustado para comportar logs e buffer)
      NULL,          // Parâmetro passado (não usado)
      1,             // Prioridade (maior que idle)
      NULL           // Handle da task (não usado aqui)
  );

  // Verifica se a task foi criada com sucesso
  if (res != pdTRUE)
  {
    ESP_LOGE(TAG, "Fail to create task");
    return ESP_FAIL;
  }

  return ESP_OK;
}

//**************************************************
// Static Functions
//**************************************************

void uart_initialize()
{
  // Estrutura de configuração da UART
  const uart_config_t uart_config = {
      .baud_rate = UART_BAUD,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };

  // Instala driver da UART (buffer de RX em dobro, sem TX buffer, sem fila de eventos)
  ESP_ERROR_CHECK(uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0));

  // Aplica configuração da UART
  ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));

  // Define os pinos de TX/RX da UART2
  ESP_ERROR_CHECK(uart_set_pin(UART_PORT,
                               UART_TX_PIN,
                               UART_RX_PIN,
                               UART_PIN_NO_CHANGE,
                               UART_PIN_NO_CHANGE));
}

void uart_rx_task(void *pvParameters)
{
  // Aloca buffer para armazenar dados recebidos
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE + 1);

  while (1)
  {
    // Lê dados recebidos na UART (timeout de 20 ms)
    int len = uart_read_bytes(UART_PORT, data, BUF_SIZE, pdMS_TO_TICKS(20));

    if (len <= 0)
    {
      // Nenhum dado recebido → volta ao início do loop
      continue;
    }

    // Garante terminação de string para log
    data[len] = '\0';

    // Exibe dados recebidos via log
    ESP_LOGI(TAG, "Read \"%s\"", data);

    // Reenvia (eco) os mesmos dados pela UART
    uart_write_bytes(UART_PORT, (const char *)data, len);
  }
}
