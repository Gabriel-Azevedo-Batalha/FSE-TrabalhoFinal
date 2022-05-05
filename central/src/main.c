#include <string.h>
#include <ncurses.h>
#include <mosquitto.h>
#include "jsonHandler.h"
#include "csv.h"
#include "queue.h"
#include <pthread.h>

dispositivo *dispositivos;
int numDispositivos = 0;
int cadastrando = 0;
struct mosquitto *mosq;
fila *descadastrados;

int alarmeArmado = 0;
int alarmeAcionado = 0;

void mqttPublish(struct mosquitto *mosq, char *topic, char *msg){
    mosquitto_publish(mosq, NULL, topic, strlen(msg), msg, 1, false);
}

void descadastrar(int num){
    char *jsonMsg = descadastroJson();
    dispositivos[num].ativo = 0;
    insereFila(descadastrados, num);
    mqttPublish(mosq, dispositivos[num].id, jsonMsg);
    char action[50];
    sprintf(action, "Descadastrado: ID %d", num);
    saveCSV(action);
}

void encerra(int s){
    endwin();
    mosquitto_loop_stop(mosq, true);
    for(int i=0;i<numDispositivos;i++){
        descadastrar(i);
    }
	mosquitto_disconnect(mosq);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
    free(dispositivos);
    exit(0);
}

void ativarAlarme(int num){
    alarmeAcionado = 1;
    char action[30];
    sprintf(action, "Alarme ativado: ID %d", num);
    saveCSV(action);
    beep();
}

void cadastrar(const struct mosquitto_message *msg){

    int num = filaVazia(descadastrados) ? numDispositivos : removeFila(descadastrados);
    cadastrando = 1;
    dispositivo novoDispositivo;
    novoDispositivo.num = num;
    novoDispositivo.ativo = 1;
    novoDispositivo.id = malloc(100);
    strcpy(novoDispositivo.id, msg->topic);
    novoDispositivo.entrada = 0;
    novoDispositivo.saida = 0;
    novoDispositivo.energia = readEnergy(msg->payload);
    novoDispositivo.comodo = malloc(30);
    novoDispositivo.topico = malloc(100);
    novoDispositivo.nomeEntrada = malloc(30);
    novoDispositivo.temperatura = 0;
    novoDispositivo.umidade = 0;

    clear();
    timeout(-1);

    // Comodo
    printw("Digite o comodo\n");
    refresh();
    scanw(" %s", novoDispositivo.comodo);
    refresh();

    // Entrada
    printw("Digite o nome da entrada\n");
    refresh();
    scanw(" %s", novoDispositivo.nomeEntrada);
    refresh();
    printw("A entrada aciona alarme?(0/1)\n");
    refresh();
    scanw(" %d", &novoDispositivo.acionaAlarme);
    refresh();

    // Saida
    if(novoDispositivo.energia){
        novoDispositivo.nomeSaida = malloc(30);
        printw("Digite o nome da saida\n");
        refresh();
        scanw(" %s", novoDispositivo.nomeSaida);
        refresh();
        printw("A saída é dimerizável? (0/1)\n");
        refresh();
        scanw(" %d", &novoDispositivo.dimerizavel);
        refresh();
    }
    else
        novoDispositivo.dimerizavel = 0;

    char top[100];
    sprintf(top, "fse2021/180100840/%s", novoDispositivo.comodo);
    strcpy(novoDispositivo.topico, top);

    char *jsonMsg = cadastroJson(novoDispositivo);
    mqttPublish(mosq, msg->topic, jsonMsg);

    sprintf(top, "%s/#", top);
    mosquitto_subscribe(mosq, NULL, top, 1);
    refresh();

    if(num == numDispositivos){
        dispositivos = realloc(dispositivos, (numDispositivos+1) * sizeof(dispositivo));
        dispositivos[numDispositivos++] = novoDispositivo;
    }
    else
        dispositivos[num] = novoDispositivo;
    cadastrando = 0;
    timeout(1000);
}

void printAll(){
    if(!cadastrando){
        clear();
        if(numDispositivos == 0)
            printw("Sem dispositivos cadastrados\n");
        else{
            printw("======= %s=======\n", alarmeArmado ? "Alarme Armado ===" : "Alarme Desarmado ");
            if(alarmeAcionado)
                printw("ALARME ACIONADO !!!!! - (A) para desarmar\n");
            for(int i=0;i<numDispositivos;i++){
                if (!dispositivos[i].ativo)
                    continue;
                if(dispositivos[i].energia){
                    printw("%d) %s\n---> Entrada: %s -> %d\n---> Saida: %s -> %d %s\n", i, dispositivos[i].comodo, dispositivos[i].nomeEntrada, dispositivos[i].entrada, 
                    dispositivos[i].nomeSaida, dispositivos[i].saida, dispositivos[i].dimerizavel ? "(Dimerizável)" : "");
                    printw("---> Temperatura -> %d\n", dispositivos[i].temperatura);
                    printw("---> Umidade -> %d\n", dispositivos[i].umidade);
                }
                else
                    printw("%d) %s\n---> Entrada: %s -> %d\n", i, dispositivos[i].comodo, dispositivos[i].nomeEntrada, dispositivos[i].entrada);
            }
            printw("Pressione o número para mudar a Saída (Apenas Energia)\n");
            printw("Pressione (A) para armar/desarmar o Alarme\n");
            printw("Pressione (D) para descadastrar um dispositivo\n");
        }
        refresh();
    }
}

void on_connect(struct mosquitto *mosq, void *obj, int rc) {
	if(rc) {
		printf("Error with result code: %d\n", rc);
		exit(-1);
	}
	mosquitto_subscribe(mosq, NULL, "fse2021/180100840/dispositivos/#", 1);
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    char *type = readType(msg->payload);

    if(strcmp(type, "cadastro") == 0){
        cadastrar(msg); 
        char action[30];
        sprintf(action, "Cadastrado: ID %d", numDispositivos-1);
        saveCSV(action);
    }
    else if(strcmp(type, "entrada") == 0){
        cJSON *json = cJSON_Parse(msg->payload);
        int num = jParseInt(json, "num");
        dispositivos[num].entrada = jParseInt(json, "entrada");
        
        if(dispositivos[num].acionaAlarme && alarmeArmado)
            ativarAlarme(num);
        else{
            char action[50];
            sprintf(action, "Entrada alterada: ID %d, Val %d", num, dispositivos[num].entrada);
            saveCSV(action);
        }
        free(json);
    }
    else if(strcmp(type, "data") == 0){
        cJSON *json = cJSON_Parse(msg->payload);
        char *dataType = jParseString(json, "dataType");
        int num = jParseInt(json, "num");
        int data = jParseInt(json, dataType);
        if (strcmp(dataType, "temperatura") == 0)
            dispositivos[num].temperatura = data;
        else
            dispositivos[num].umidade = data;
        free(json);
    }
    else if(strcmp(type, "descadastrar") == 0){
        cJSON *json = cJSON_Parse(msg->payload);
        int num = jParseInt(json, "num");
        descadastrar(num);
        free(json);
    }
}

void readPercentage(int num){
    clear();
    timeout(-1);
    printw("Digite a porcentagem de ativação (0-100)\n");
    refresh();
    scanw(" %d", &dispositivos[num].saida);
    refresh();
    timeout(1000);
}

int readDevice(){
    int choice;
    timeout(-1);
    printw("\nDigite o numero do dispositivo\n");
    refresh();
    scanw(" %d", &choice);
    refresh();
    timeout(1000);
    return choice;
}

void *update(void *args) {
	while(1){
        if(cadastrando) 
            continue;
        char ch = 126;
		printAll();
		ch = getch();
		refresh();
        if(ch-48 < numDispositivos && ch-48 >= 0){
			if(dispositivos[(int) ch-48].energia){
                if(dispositivos[(int) ch-48].dimerizavel){
                    readPercentage((int) ch-48);
                }
                else{
                    dispositivos[(int) ch-48].saida = !dispositivos[(int) ch-48].saida;
                }
                char *msg = mudaSaidaJson(dispositivos[(int) ch-48].saida);
				mqttPublish(mosq, dispositivos[(int) ch-48].id, msg);
                char action[50];
                sprintf(action, "Saida alterada: ID %d, Val %d", (int) ch-48, dispositivos[(int) ch-48].saida);
                saveCSV(action);
            }
		}
        else if (ch == 'a'){
            alarmeArmado = !alarmeArmado;
            if(alarmeAcionado)
                alarmeAcionado = 0;
            char action[30];
            sprintf(action, "Alarme alterado: %d", alarmeArmado);
            saveCSV(action);
        }
        else if(ch == 'd'){
            descadastrar(readDevice());  
        }
    }
}

int main() {
	int rc, id;

    // MQTT START
	mosquitto_lib_init();
	mosq = mosquitto_new("subscribe-test", true, &id);
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);
	rc = mosquitto_connect(mosq, "test.mosquitto.org", 1883, 10);
	if(rc) {
		printf("Could not connect to Broker with return code %d\n", rc);
		return -1;
	}
    
    // INITS
    csvStart();
    descadastrados = filaInit();
    dispositivos = malloc(1);
    initscr();
    signal(SIGINT, encerra);
	timeout(1000);
    pthread_t thread_update;

    // THREAD START
    pthread_create(&thread_update, NULL, &update, NULL);
    mosquitto_loop_forever(mosq, -1, 1); //mosquitto_loop_start(mosq);

	pthread_join(thread_update, NULL);

    return 0;
}