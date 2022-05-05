#include "nvsHandler.h"


void le_valor_nvs(int *num, char *topic, char *macAdr, int *botao, int *ledState, int *dimerizavel){
    // Inicia o acesso à partição padrão nvs
    ESP_ERROR_CHECK(nvs_flash_init());

    // Inicia o acesso à partição personalizada
    //ESP_ERROR_CHECK(nvs_flash_init_partition("NVS"));

    int32_t valor = 0;
    nvs_handle particao_padrao_handle;
    
    // Abre o acesso à partição nvs
    esp_err_t res_nvs = nvs_open("armazenamento", NVS_READONLY, &particao_padrao_handle);
    
    // Abre o acesso à partição DadosNVS
    // esp_err_t res_nvs = nvs_open_from_partition("DadosNVS", "armazenamento", NVS_READONLY, &particao_padrao_handle);
    


    if(res_nvs == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGE("NVS", "Namespace: armazenamento, não encontrado");
    }
    else
    {
        esp_err_t res = nvs_get_i32(particao_padrao_handle, "num", num);
        ESP_LOGI("NVS", "NUM %d", *num);

        res = nvs_get_i32(particao_padrao_handle, "botao", botao);
        ESP_LOGI("NVS", "BOTAO %d", *botao);

        res = nvs_get_i32(particao_padrao_handle, "ledState", ledState);
        ESP_LOGI("NVS", "LEDSTATE %d", *ledState);

        res = nvs_get_i32(particao_padrao_handle, "dimerizavel", dimerizavel);
        ESP_LOGI("NVS", "DIMERIZAVEL %d", *dimerizavel);

        size_t length;
        nvs_get_str(particao_padrao_handle, "topic", 0, &length);
        res = nvs_get_str(particao_padrao_handle, "topic", topic, &length);
        ESP_LOGI("NVS", "TOPIC %s", topic);

        nvs_get_str(particao_padrao_handle, "macAdr", 0, &length);
        res = nvs_get_str(particao_padrao_handle, "macAdr", macAdr, &length);
        ESP_LOGI("NVS", "MACADR %s", macAdr);


        switch (res)
        {
        case ESP_OK:
            printf("Valor armazenado: %d\n", valor);
            break;
        case ESP_ERR_NOT_FOUND:
            ESP_LOGE("NVS", "Valor não encontrado");
            break;
        default:
            ESP_LOGE("NVS", "Erro ao acessar o NVS (%s)", esp_err_to_name(res));
            break;
        }

        nvs_close(particao_padrao_handle);
    }
}

void grava_value_nvs(int num, char *topic, char *macAdr, int botao, int ledState, int dimerizavel) {
    ESP_ERROR_CHECK(nvs_flash_init());
    nvs_handle particao_padrao_handle;

    //esp_err_t res_nvs = nvs_open_from_partition("NVS", "armazenamento", NVS_READWRITE, &particao_padrao_handle);
    esp_err_t res_nvs = nvs_open("armazenamento", NVS_READWRITE, &particao_padrao_handle);
    if (res_nvs != ESP_ERR_NVS_NOT_FOUND) {
        esp_err_t res;
        res = nvs_set_i32(particao_padrao_handle, "num", num);
        res = nvs_set_i32(particao_padrao_handle, "botao", botao);
        res = nvs_set_i32(particao_padrao_handle, "dimerizavel", dimerizavel);
        res = nvs_set_i32(particao_padrao_handle, "ledState", ledState);
        res = nvs_set_str(particao_padrao_handle, "topic", topic);
        res = nvs_set_str(particao_padrao_handle, "macAdr", macAdr);
        if (res != ESP_OK) 
            ESP_LOGE("NVS", "Não foi possível escrever no NVS (%s)", esp_err_to_name(res));
        nvs_commit(particao_padrao_handle);
        nvs_close(particao_padrao_handle);
    }
}