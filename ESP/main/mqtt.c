#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp32/rom/uart.h"
#include "esp_netif.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_sleep.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "mqtt.h"
#include "jsonHandler.h"
#include "nvsHandler.h"

#define TAG "MQTT"
#define LED 2
#define BOTAO 0
#define MODE_ENERGY CONFIG_MODO_ENERGIA

extern xSemaphoreHandle conexaoMQTTSemaphore;
extern xSemaphoreHandle cadastradoSemaphore;
esp_mqtt_client_handle_t client;
extern char topic[100];
extern int num;
extern char macAdr[20];
extern int botaoAtivo;
extern int ledState;
extern int dimerizavel;

void initLedC(){
    // Configuração do Timer
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK
        };
    ledc_timer_config(&timer_config);

    // Configuração do Canal
    ledc_channel_config_t channel_config = {
        .gpio_num = LED,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&channel_config);
}

void cadastrar(char *json){
    dispositivo d;
    d = readCadastro(json);
    strcpy(topic, d.topico);
    dimerizavel = d.dimerizavel;
    num = d.num;
    if(dimerizavel)
        initLedC();
    xSemaphoreGive(cadastradoSemaphore);
}

void descadastrar(){
    num = -1;
    grava_value_nvs(num, topic, macAdr, botaoAtivo, ledState, dimerizavel);
    esp_restart();
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event){
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            if(num == -1){
                char top[60];
                uint8_t mac[6];
                sprintf(macAdr, MACSTR, MAC2STR(mac));
                esp_read_mac(mac, 0);
                sprintf(top,"fse2021/180100840/dispositivos/%s", macAdr);
                msg_id = esp_mqtt_client_subscribe(client, top, 0);
                xSemaphoreGive(conexaoMQTTSemaphore);
                if(!xSemaphoreTake(cadastradoSemaphore, 0)){
                    char *mensagem = cadastroJson(MODE_ENERGY);
                    mqtt_envia_mensagem(top, mensagem, 2);
                }
            }
            else if(MODE_ENERGY){
                char top[60];
                sprintf(top,"fse2021/180100840/dispositivos/%s", macAdr);
                msg_id = esp_mqtt_client_subscribe(client, top, 0);
                initLedC();
                if(!dimerizavel)
                    gpio_set_level(LED, ledState);
                else{
                    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, (ledState*255)/100);
                    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
                }
                xSemaphoreGive(conexaoMQTTSemaphore);
                xSemaphoreGive(cadastradoSemaphore);
            }
            else{
                ESP_LOGI(TAG, "IF 3---");
                char top[150];
                sprintf(top, "%s/estado", topic);
                esp_mqtt_client_subscribe(client, top, 0);
                char *mensagem = mudancaEstado(botaoAtivo, num);
                mqtt_envia_mensagem(top, mensagem, 1);
            }
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");

            char *data = malloc(event->data_len + 2);
            sprintf(data, "%.*s",  event->data_len, event->data);
            ESP_LOGI(TAG, "===> %s", data);
            char *type = readType(data);
            
            if(strcmp("mudaSaida", type) == 0){
                ESP_LOGI(TAG, "Muda led para %d", ledState);
                ledState = readGPIO(data);
                if(MODE_ENERGY){
                    grava_value_nvs(num, topic, macAdr, botaoAtivo, ledState, dimerizavel);
                    if(!dimerizavel)
                        gpio_set_level(LED, ledState);
                    else{
                        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, (ledState*255)/100);
                        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
                    }
                }
                
            }
            else if(strcmp("device", type) == 0){
                ESP_LOGI(TAG, "Cadastro");
                cadastrar(data);
            }
            else if(strcmp("descadastro", type) == 0){
                ESP_LOGI(TAG, "Descadastro");
                descadastrar();
            }
             else if(strcmp("entrada", type) == 0 && !MODE_ENERGY){
                xSemaphoreGive(cadastradoSemaphore);
            }
            free(data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

void mqtt_start(){
    esp_mqtt_client_config_t mqtt_config = {
        .uri = "mqtt://test.mosquitto.org",
    };
    client = esp_mqtt_client_init(&mqtt_config);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

void mqtt_stop(){
    esp_mqtt_client_stop(client);
}

void mqtt_envia_mensagem(char *topico, char *mensagem, int qos){
    ESP_LOGI(TAG, "Topico: %s, Mensagem %s", topico, mensagem);
    int message_id = esp_mqtt_client_publish(client, topico, mensagem, 0, qos, 0);
    ESP_LOGI(TAG, "Mesnagem enviada, ID: %d", message_id);
}
