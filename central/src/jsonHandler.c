#include "jsonHandler.h"

char *jParseString(cJSON *json, char *attribute){
	cJSON *item = cJSON_GetObjectItem(json, attribute);	
	return item->valuestring;
}

int jParseInt(cJSON *json, char *attribute){
	cJSON *item = cJSON_GetObjectItem(json, attribute);	
	return item->valueint;
}

char *cadastroJson(dispositivo d){
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "type", cJSON_CreateString("device"));
	cJSON_AddItemToObject(json, "topico", cJSON_CreateString(d.topico));
	cJSON_AddItemToObject(json, "saida", cJSON_CreateNumber(d.saida));
	cJSON_AddItemToObject(json, "num", cJSON_CreateNumber(d.num));
	cJSON_AddItemToObject(json, "dimerizavel", cJSON_CreateNumber(d.dimerizavel));

	char *str = cJSON_PrintUnformatted(json);
	return str;
}

char *descadastroJson(){
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "type", cJSON_CreateString("descadastro"));

	char *str = cJSON_PrintUnformatted(json);
	return str;
}

char *mudaSaidaJson(int saida){
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "type", cJSON_CreateString("mudaSaida"));
	cJSON_AddItemToObject(json, "saida", cJSON_CreateNumber(saida));

	char *str = cJSON_PrintUnformatted(json);
	return str;
}

char *readType(char *jsonString){
	cJSON *json = cJSON_Parse(jsonString);
	char *type = jParseString(json, "type");
	free(json);
	return type;
}

int readEnergy(char *jsonString){
	cJSON *json = cJSON_Parse(jsonString);
	int energia = jParseInt(json, "energia");
	free(json);
	return energia;
}