#include "bicicletasADT.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_TOKENS 100
#define PARAM_ERROR -1
#define MAX_TEXT 50

void query1(cityADT city);

int checkParams(char* bikes, char*stations, int startYear, int endYear){

    if(endYear < 0 || startYear < 0) return 0;
    if(startYear != 0 && endYear != 0){
        if(endYear < startYear) return 0;
    }
    if(strcmp(bikes, "bikesNYC.csv") != 0) return 0;
    if(strcmp(stations,"stationsNYC.csv") != 0) return 0;
    return 1;
}

static void
readDate(char * s, struct tm * date) {
    sscanf(s, "%d-%d-%d %d:%d:%d", &(date->tm_year), &(date->tm_mon), &(date->tm_mday), &(date->tm_hour), &(date->tm_min), &(date->tm_sec));
}

int main(int argc, char * argv[]){

    int startYear = 0, endYear = 0;
    char* bikes, *stations;

    if(argc < 3 || argc > 5) {
        printf("Cantidad invalida de parametros.\n");
        return PARAM_ERROR;
    } else {
        bikes = argv[1];
        stations = argv[2];
        if(argc > 3) {
            startYear = atoi(argv[3]);
            if(argc == 5) {
                endYear = atoi(argv[4]);
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

    char aux[MAX_TOKENS];
    int first = 1;
    while(fgets(aux, MAX_TOKENS, stationsCsv) != NULL) {
        char * name;
        unsigned long stationId;

        if(first) {
            first = 0;
        } else {
            name = strtok(aux, ";");
            for(int i = 0; i < 2; i++)
                strtok(NULL, ";");                  //ignoramos la longitud y la latitud
            stationId = atoi(strtok(NULL, "\n"));
            addStation(nyc, name, stationId);
        }
    }
    
    
    first = 1;
    while(fgets(aux, MAX_TOKENS, bikesCsv) != NULL) {
        unsigned long startStationId, endStationId;
        struct tm startDate, endDate;
        char * memberState;

        if(first) {
            first = 0;
        } else {
            readDate(strtok(aux, ";"), &startDate);
            startStationId = atoi(strtok(NULL, ";"));
            readDate(strtok(NULL, ";"), &endDate);
            endStationId = atoi(strtok(NULL, ";"));
            strtok(NULL, ";"); //ignoramos rideable
            memberState = strtok(NULL, "\n");
            addRide(nyc, startStationId, startDate, endDate, endStationId, strcmp("member", memberState) == 0);
        }
    }


    fclose(bikesCsv);
    fclose(stationsCsv);

    query1(nyc);

    freeCity(nyc);

    return 0;
}

void query1(cityADT city){
    int cantStations = getStationCount(city);
    int indexVec[cantStations];

    getIdexByRank(city, indexVec);

    FILE * file;
    file = fopen("query1.csv", "w+");

    fprintf(file, "bikeStation;memberTrips;casualTrips;allTrips\n");

    for (int i = 0; i < cantStations; ++i) {
        unsigned long v[2];
        ridesByStationIndex(city, indexVec[i], v);
        char * name = nameByStationIndex(city, indexVec[i]);
        fprintf(file, "%s;%ld;%ld;%ld\n", name, v[0], v[1], v[0]+v[1]);
    }
    fclose(file);
}
