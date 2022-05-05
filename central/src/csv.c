#include "csv.h"

FILE* csv;

void csvStart(){
	char filename[50];
	struct tm *timenow;
	time_t now = time(NULL);
	timenow = gmtime(&now);
	strftime(filename, sizeof(filename), "log/LOG_%Y-%m-%d_%H:%M:%S.csv", timenow);
	csv = fopen(filename, "w");
	if (csv == NULL){
		perror("Error");
		raise(SIGINT);
	}
	fprintf(csv, "Data, Hora, Ação\n");
}

void saveCSV(char *action){
    // Data e Hora
	struct tm *timenow;
    time_t now = time(NULL);
    timenow = gmtime(&now);
    char dateString[20];
    char timeString[20];
    strftime(dateString, sizeof(dateString), "%Y-%m-%d", timenow);
    strftime(timeString, sizeof(timeString), "%H:%M:%S", timenow);

    fprintf(csv, "%s, %s, %s\n", dateString, timeString, action);
}