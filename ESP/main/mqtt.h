#ifndef MQTT_H
#define MQTT_H

void mqtt_start();
void mqtt_stop();

void mqtt_envia_mensagem(char * topico, char * mensagem, int qos);

#endif