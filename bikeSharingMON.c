#include <stdio.h>
#include "bicicletasADT.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include "bicicletasADT.h"
#define ERROR -1

int checkParams(char* bikes, char*stations, int startYear, int endYear){

    if(endYear < 0 || startYear < 0) return 0;
    if(startYear != 0 && endYear != 0){
        if(endYear < startYear) return 0;
    }
    if(strcmp(bikes, "bikesMON.csv") != 0) return 0;
    if(strcmp(stations,"stationsMON.csv") != 0) return 0;
    return 1;
}


int main(int argc, char * argv[]){

    int startYear = 0, endYear = 0;
    char* bikes, stations;

    bikes = argv[1];
    stations = argv[2];

    if(argc == 3 || argc == 4){
            startYear = atoi(argv[3]);
            if(argc == 4) endYear = atoi(arg[4]);
    } else{
        printf("Error! Cantidad invalida de parametros \n");
        return ERROR;
    }
    if((int a = checkParams(bikes, stations, startYear, endYear) == 0)){
        printf("Error! Parametros Invalidos\n");
        return ERROR;
    }

    cityADT montreal = newCity();

    FILE * bikesCsv = fopen(bikes, "r");
    FILE * stationsCsv = fopen(stations, "r");

    unsigned long stationId;
    char *name;
    while (fscanf(stationsCsv, "%ld;%s;%*f;%*f"), stationId, name){
        addStation(montreal, name, stationId);
    }
    
    char * lineaActual;
    unsigned long startStationId, endStationId;
    struct tm startDate, endDate;
    char isMember;
    while(fscanf(bikesCsv, "%d-%d-%d %d:%d:%d;%ld;%d-%d-%d %d:%d:%d;%ld;%c", startDate.tm_year, startDate.tm_mon, startDate.tm_mday, 
    startDate.tm_hour, startDate.tm_min, startDate.tm_sec, startStationId, endDate.tm_year, endDate.tm_mon, endDate.tm_mday, endDate.tm_hour,
    endDate.tm_min, endDate.tm_sec, endStationId, isMember) != EOF){                     //TAMBIEN PUEDE SER DISTINTO DE CERO
        addRide(montreal, startStationId, startDate, endDate, endStationId, isMember);
    }





    fclose(bikesCsv);
    fclose(stationsCsv);
    return 0;
}