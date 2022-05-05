#ifndef NVS_HANDLER_H
#define NVS_HANDLER_H
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

void le_valor_nvs(int *num, char *topic, char *macAdr, int *botao, int *ledState, int *dimerizavel);

void grava_value_nvs(int num, char *topic, char *macAdr, int botao, int ledState, int dimerizavel);

#endif