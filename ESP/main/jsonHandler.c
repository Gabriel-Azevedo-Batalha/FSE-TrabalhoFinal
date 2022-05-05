#include "jsonHandler.h"

char *jParseString(cJSON *json, char *attribute){
	cJSON *item = cJSON_GetObjectItem(json, attribute);	
	return item->valuestring;
}

int jParseInt(cJSON *json, char *attribute){
	cJSON *item = cJSON_GetObjectItem(json, attribute);	
	return item->valueint;
}

char *cadastroJson(int energia){
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "type", cJSON_CreateString("cadastro"));
	cJSON_AddItemToObject(json, "energia", cJSON_CreateNumber(energia));
	char *str = cJSON_PrintUnformatted(json);
	return str;
}

char *readType(char *jsonString){
	cJSON *json = cJSON_Parse(jsonString);
	char *type = jParseString(json, "type");
	free(json);
	return type;
}

char *mudancaEstado(int estado, int num){
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "type", cJSON_CreateString("entrada"));
	cJSON_AddItemToObject(json, "entrada", cJSON_CreateNumber(estado));
	cJSON_AddItemToObject(json, "num", cJSON_CreateNumber(num));
	char *str = cJSON_PrintUnformatted(json);
	return str;
}

dispositivo readCadastro(char *jsonString) {

	cJSON *json = cJSON_Parse(jsonString);
	dispositivo d;

	d.topico = jParseString(json, "topico");
	d.saida = jParseInt(json, "saida");
	d.dimerizavel = jParseInt(json, "dimerizavel");
	d.num = jParseInt(json, "num");

	free(json);
	return d;
}

int readGPIO(char *jsonString) {
	cJSON *json = cJSON_Parse(jsonString);
	int GPIO = jParseInt(json, "saida");
	free(json);
	return GPIO;
}

char *dhtDataToJson(int data, char *name, int num){
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "type", cJSON_CreateString("data"));
	cJSON_AddItemToObject(json, "dataType", cJSON_CreateString(name));
	cJSON_AddItemToObject(json, name, cJSON_CreateNumber(data));
	cJSON_AddItemToObject(json, "num", cJSON_CreateNumber(num));
	char *str = cJSON_PrintUnformatted(json);
	return str;
}

char *solicitarDescadastro(int num){
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "type", cJSON_CreateString("descadastrar"));
	cJSON_AddItemToObject(json, "num", cJSON_CreateNumber(num));
	char *str = cJSON_PrintUnformatted(json);
	return str;
}