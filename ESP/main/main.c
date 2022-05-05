#include <stdio.h>
#include <string.h>
#include "nvs.h"

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "esp_sleep.h"
#include "esp32/rom/uart.h"

#include "dht11.h"
#include "wifi.h"
#include "http_client.h"
#include "mqtt.h"
#include "jsonHandler.h"
#include "nvsHandler.h"

#define BOTAO 0
#define LED 2
#define MODE_ENERGY CONFIG_MODO_ENERGIA

xSemaphoreHandle conexaoWifiSemaphore;
xSemaphoreHandle conexaoMQTTSemaphore;
xSemaphoreHandle cadastradoSemaphore;
char topic[100]; 
int botaoAtivo = 0;
int num;
char macAdr[20];
int ledState = 0;
int dimerizavel = 0;

xQueueHandle filaDeInterrupcao;

static void IRAM_ATTR gpio_isr_handler(void *args){
  int pino = (int)args;
  xQueueSendFromISR(filaDeInterrupcao, &pino, NULL);
}

void trataInterrupcaoBotao(void *params){
  int pino;
  
  while(true){
    if(xQueueReceive(filaDeInterrupcao, &pino, portMAX_DELAY) && MODE_ENERGY){
      // De-bouncing

      int estado = gpio_get_level(pino);
      if(estado == 1){
        gpio_isr_handler_remove(pino);
        botaoAtivo = !botaoAtivo;
        grava_value_nvs(num, topic, macAdr, botaoAtivo, ledState, dimerizavel);
        printf("O botão foi acionado, Ativo: %d\n", botaoAtivo);

        char *mensagem = mudancaEstado(botaoAtivo, num);
        char top[150];
        sprintf(top, "%s/estado", topic);
        mqtt_envia_mensagem(top, mensagem, 1);

        while(gpio_get_level(pino) == estado){
          vTaskDelay(50 / portTICK_PERIOD_MS);
        }

        // Habilitar novamente a interrupção
        vTaskDelay(50 / portTICK_PERIOD_MS);
        gpio_isr_handler_add(pino, gpio_isr_handler, (void *) pino);
      }
    }
  }
}


void conectadoWifi(void * params){
  while(true)
  {
    if(xSemaphoreTake(conexaoWifiSemaphore, portMAX_DELAY))
    {
      // Processamento Internet
      mqtt_start();
    }
  }
}

int somaFila(xQueueHandle fila, int *media){
  int soma = 0;
  int valor;
  *media = 0;
  for(int i=0; i<5; i++){
    xQueueReceive(fila, &valor, 0);
    if(valor == -1)
      valor = 0;
    else
      (*media)++;
    soma += valor;
  }
  return soma;
}

void trataComunicacaoComServidor(void * params){
  char mensagem[50];
  if(MODE_ENERGY && xSemaphoreTake(conexaoMQTTSemaphore, portMAX_DELAY) && 
    xSemaphoreTake(cadastradoSemaphore, portMAX_DELAY)){
    grava_value_nvs(num, topic, macAdr, botaoAtivo, ledState, dimerizavel);
    xQueueHandle tempFila;
    xQueueHandle humFila;
    tempFila = xQueueCreate(5, sizeof(int));
    humFila = xQueueCreate(5, sizeof(int));
    int count = 0;
    xSemaphoreGive(cadastradoSemaphore);
    while(true){
      struct dht11_reading reading = DHT11_read();
      ESP_LOGI("TEMP", "TEMPERATURA %d, Humidade %d", reading.temperature, reading.humidity);
      

      xQueueSend(tempFila, &reading.temperature, 0);
      xQueueSend(humFila, &reading.humidity, 0);
      count++;
      
      if(count == 5){
        count = 0;
        int totalMedia;
        
        int tempMed = somaFila(tempFila, &totalMedia);
        tempMed /= totalMedia;
        
        int humMed = somaFila(humFila, &totalMedia);
        humMed /= totalMedia;

        char top[150];
        sprintf(top, "%s/temperatura", topic);
        mqtt_envia_mensagem(top, dhtDataToJson(tempMed, "temperatura", num), 1);

        sprintf(top, "%s/umidade", topic);
        mqtt_envia_mensagem(top, dhtDataToJson(humMed, "umidade", num), 1);
      }
      vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
  }
}

void GPIOinit(){
  gpio_pad_select_gpio(LED);
  gpio_set_direction(LED, GPIO_MODE_OUTPUT);
  gpio_set_level(LED, 0);

  gpio_pad_select_gpio(BOTAO);
  gpio_set_direction(BOTAO, GPIO_MODE_INPUT);
  gpio_pulldown_en(BOTAO);
  gpio_pullup_dis(BOTAO);
  gpio_set_intr_type(BOTAO, GPIO_INTR_POSEDGE); 

  filaDeInterrupcao = xQueueCreate(10, sizeof(int));
  xTaskCreate(trataInterrupcaoBotao, "TrataBotao", 2048, NULL, 1, NULL);
  gpio_install_isr_service(0);
  gpio_isr_handler_add(BOTAO, gpio_isr_handler, (void *) BOTAO);
}

void app_main(void){
    // Inicializa o GPIO
    GPIOinit();

    // Inicializa o DHT
    DHT11_init(GPIO_NUM_16);

    // Inicializa o NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    conexaoWifiSemaphore = xSemaphoreCreateBinary();
    conexaoMQTTSemaphore = xSemaphoreCreateBinary();
    cadastradoSemaphore = xSemaphoreCreateBinary();

    num = -1;
    le_valor_nvs(&num, topic, macAdr, &botaoAtivo, &ledState, &dimerizavel);

    wifi_start();

    xTaskCreate(&conectadoWifi, "Conexão ao MQTT", 4096, NULL, 1, NULL);
    xTaskCreate(&trataComunicacaoComServidor, "Comunicação com Broker", 4096, NULL, 1, NULL);

    if(!MODE_ENERGY && xSemaphoreTake(cadastradoSemaphore, portMAX_DELAY)){
      // Configurando o Sleep
      gpio_wakeup_enable(BOTAO, GPIO_INTR_LOW_LEVEL);
      esp_sleep_enable_gpio_wakeup();
      //wifi_stop();
      //mqtt_stop();
      ESP_LOGI("SYS", "SLEEPING");
      uart_tx_wait_idle(CONFIG_ESP_CONSOLE_UART_NUM);
      //mqtt_stop();
      //wifi_stop();
      esp_light_sleep_start();
      grava_value_nvs(num, topic, macAdr, !botaoAtivo, ledState, dimerizavel);
      esp_restart();
      //esp_wifi_start();
      // Configurando o Sleep Timer (em microsegundos)
      //esp_sleep_enable_timer_wakeup(5 * 1000000);
    }
}
