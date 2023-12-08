#include "bicicletasADT.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "cTable\htmlTable.h"

#define MAX_TOKENS 100
#define PARAM_ERROR -1
#define MAX_TEXT 50
#define SIZE_NUM 10
#define SIZE_DATE 16

void query1(cityADT city);
void query2(cityADT city);
void query3(cityADT city);
void query4(cityADT city, int startYear, int endYear);


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
    query2(nyc);
    query3(nyc);
    query4(nyc, startYear, endYear);

    freeCity(nyc);

    return 0;
}

void query1(cityADT city){
    int cantStations = getStationCount(city);
    int indexVec[cantStations];

    getIndexByRank(city, indexVec);

    FILE * file;
    file = fopen("query1.csv", "w+");
    htmlTable table = newTable("query1.html", 4, "bikeStation", "memberTrips", "casualTrips", "allTrips");

    fprintf(file, "bikeStation;memberTrips;casualTrips;allTrips\n");
    char memstr[SIZE_NUM], casstr[SIZE_NUM], allnum[SIZE_NUM];

    for (int i = 0; i < cantStations; ++i) {
        unsigned long v[2];
        ridesByStationIndex(city, indexVec[i], v);
        char * name = nameByStationIndex(city, indexVec[i]);
        fprintf(file, "%s;%ld;%ld;%ld\n", name, v[0], v[1], v[0]+v[1]);
        sprintf(memstr, "%ld", v[0]);
        sprintf(casstr, "%ld", v[1]);
        sprintf(allnum, "%ld", v[0]+v[1]);
        addHTMLRow(table, name, memstr, casstr, allnum);
    }
    closeHTMLTable(table);
    fclose(file);
}

void query2(cityADT city){

    int cantStations = getStationCount(city);
    int indexVec[cantStations];

    getIndexByAlph(city, indexVec);

    FILE * file = fopen("query2.csv", "w+");

    htmlTable table = newTable("query2.html", 3, "bikeStation", "bikeEndStation", "oldestDateTime");
    char datestr[SIZE_DATE];    

    fprintf(file, "bikeStation;bikeEndStation;oldestDateTime\n");
    for (int i = 0; i < cantStations; ++i) {
        char * nameStart, * nameEnd;
        struct tm oldestTime;
        getOldest(city, indexVec[i], &nameStart, &nameEnd, &oldestTime);
        if(nameEnd != NULL){
            fprintf(file, "%s;%s;%d/%d/%d %d:%d\n", nameStart, nameEnd, oldestTime.tm_mday, oldestTime.tm_mon, oldestTime.tm_year,
               oldestTime.tm_hour, oldestTime.tm_min);
            sprintf(datestr, "%d/%d/%d %d:%d", oldestTime.tm_mday, oldestTime.tm_mon, oldestTime.tm_year, oldestTime.tm_hour, oldestTime.tm_min);
            addHTMLRow(table, nameStart, nameEnd, datestr);
        }
    }
    closeHTMLTable(table);
    fclose(file);
}

void query3(cityADT city) {
    FILE * file;
    file = fopen("query3.csv", "w+");

    char * weekVec[7] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    htmlTable table = newTable("query3.html", 3, "weekDay", "startedTrips", "endedTrips");
    char numstr1[SIZE_NUM], numstr2[SIZE_NUM];

    fprintf(file, "weekDay;startedTrips;endedTrips\n");
    for(int i = 0; i < DAYS_OF_WEEK; i++) {
        size_t cantStartedTrips = getStartedRides(city, i), cantEndedTrips = getEndedRides(city, i);
        sprintf(numstr1, "%ld", cantStartedTrips);
        sprintf(numstr2, "%ld", cantEndedTrips);
        fprintf(file, "%s;%ld;%ld\n", weekVec[i], cantStartedTrips, cantEndedTrips);
        addHTMLRow(table, weekVec[i], numstr1, numstr2);
    }
    closeHTMLTable(table);
    fclose(file);
}

void query4(cityADT city, int startYear, int endYear){

    size_t cantStations = getStationCount(city);
    int indexVec[cantStations];

    getIndexByAlph(city, indexVec);

    FILE * file = fopen("query4.csv", "w+");

    fprintf(file, "bikeStation;mostPopRouteEndStation;mostPopRouteTrips\n");
    htmlTable table = newTable("query4.html", 3, "bikeStation", "mostPopRouteEndStation", "mostPopRouteTrips");
    char numstr[SIZE_NUM];

    for (int i = 0; i < cantStations; ++i) {
        char * endName, * startName = nameByStationIndex(city, indexVec[i]);
        size_t cantRides;
        getMostPopular(city, indexVec[i], &cantRides, &endName, startYear, endYear);
        sprintf(numstr, "%ld", cantRides);
        if(endName != NULL) {
            fprintf(file, "%s;%s;%ld\n", startName, endName, cantRides);
            addHTMLRow(table, startName, endName, numstr);
        }
    }
    closeHTMLTable(table);
    fclose(file);
}
