#include "bicicletasADT.h"
#include <stdio.h>

#define ERROR -1

int checkParams(char* bikes, char*stations, int startYear, int endYear){

    if(endYear < 0 || startYear < 0) return 0;
    if(startYear != 0 && endYear != 0){
        if(endYear < startYear) return 0;
    }
    if(strcmp(bikes, "bikesNYC.csv") != 0) return 0;
    if(strcmp(stations,"stationsNYC.csv") != 0) return 0;
    return 1;
}

int main(int argc, char * argv[]){

    int startYear = 0, endYear = 0;
    char* bikes, stations;

    if(argc < 3 || argc > 5) {
        
        printf("Cantidad invalida de parametros.\n");
        return PARAM_ERROR;
    } else {
        bikes = argv[1];
        stations = argv[2];
        if(argc > 3) {
            startYear = argv[3];
            if(argc == 5) {
                endYear = argv[4];
            }
        }
    }
    if(!checkParams(bikes, stations, startYear, endYear)) {
        printf("Error en los parametros\n");
        return PARAM_ERROR;
    }

   cityADT nyc = newCity();

    FILE * bikesCsv = fopen(bikes, "r");
    FILE * stationsCsv = fopen(stations, "r");

    char * lineaActual;
    unsigned long startStationId, endStationId;
    struct tm startDate, endDate;
    char * memberState;
    
    while(fscanf(bikesCsv, "%d-%d-%d %d:%d:%d;%ld;%d-%d-%d %d:%d:%d;%ld;%*s;%s", &startDate.tm_year, &startDate.tm_mon, &startDate.tm_mday, 
    &startDate.tm_hour, &startDate.tm_min, &startDate.tm_sec, &startStationId, &endDate.tm_year, &endDate.tm_mon, &endDate.tm_mday, &endDate.tm_hour,
    &endDate.tm_min, &endDate.tm_sec, &endStationId, memberState) != EOF){                     //TAMBIEN PUEDE SER DISTINTO DE CERO
        addRide(nyc, startStationId, startDate, endDate, endStationId, (char) strcmp("member", memberState) = 0);
    }

    char * name;
    unsigned long stationId;

    while(fscanf(bikesCsv, "%s;%*f;%*f;%ld", name, &stationId) != EOF){                     //TAMBIEN PUEDE SER DISTINTO DE CERO
        addStation(nyc, name, stationId);
    }

    fclose(bikesCsv);
    fclose(stationsCsv);
    return 0;
}
