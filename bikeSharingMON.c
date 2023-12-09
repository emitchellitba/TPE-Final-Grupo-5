#include "bicicletasADT.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "cTable\htmlTable.h"
#include <stdbool.h>

enum arguments {BIKES_FILES = 1, STATIONS_FILES, START_YEAR, END_YEAR};
enum status {OK = 0, CANT_ARG_ERROR, FILE_NOT_FOUND, INVALID_ARG, NO_MEMORY, CANT_CREATE_FILE, CANT_CREATE_TABLE};

#define MAX_TOKENS 100
#define SIZE_NUM 10
#define SIZE_DATE 16

int checkParams(char* bikes, char*stations, int startYear, int endYear);
void readDate(char * s, struct tm * date);
void query1(cityADT city);
void query2(cityADT city);
void query3(cityADT city);
void query4(cityADT city, int startYear, int endYear);

int main(int argc, char * argv[]){
    errno = 0;
    int startYear = 0, endYear = 0;
    char* bikes, *stations;

    if(argc < 3 || argc > 5) {
        puts("Invalid amount of arguments");
        return CANT_ARG_ERROR;
    } else {
        bikes = argv[BIKES_FILES];
        stations = argv[STATIONS_FILES];
        if(argc > 3) {
            startYear = atoi(argv[START_YEAR]);
            if(argc == 5) {
                endYear = atoi(argv[END_YEAR]);
            }
        }
    }
    if(!checkParams(bikes, stations, startYear, endYear)) {
        puts("Invalid arguments");
        return INVALID_ARG;
    }

    cityADT montreal = newCity();
    if(montreal == NULL) {
        puts("Cant allocate city");
        perror("Error");
        return NO_MEMORY;
    }

    FILE * bikesCsv = fopen(bikes, "r");
    FILE * stationsCsv = fopen(stations, "r");

    char aux[MAX_TOKENS];
    bool first = 1;
    
    while(fgets(aux, MAX_TOKENS, stationsCsv) != NULL) {
        char * name;
        unsigned long stationId;

        if(first) {
            first = 0;
        } else {
            stationId = atoi(strtok(aux, ";"));
            name = strtok(NULL, ";");
            if(addStation(montreal, name, stationId) == ENOMEM) {
                puts("Cant allocate station");
                perror("Error");
                return NO_MEMORY;
            }
        }    
    }
    
    accomodateStation(montreal);
    
    first = 1;
    while(fgets(aux, MAX_TOKENS, bikesCsv) != NULL) {
        unsigned long startStationId, endStationId;
        struct tm startDate, endDate;
        int isMember;

        if(first) {
            first = 0;
        } else {
            readDate(strtok(aux, ";"), &startDate);
            startStationId = atoi(strtok(NULL, ";"));
            readDate(strtok(NULL, ";"), &endDate);
            endStationId = atoi(strtok(NULL, ";"));
            isMember = atoi(strtok(NULL, "\n"));
            if(addRide(montreal, startStationId, startDate, endDate, endStationId, isMember) == ENOMEM) {
                puts("Cant allocate destiny/ride");
                perror("Error");
                return NO_MEMORY;
            }  
        }
    }

    accomodateDestiny(montreal);

    fclose(bikesCsv);
    fclose(stationsCsv);

    query1(montreal);
    query2(montreal);
    query3(montreal);
    query4(montreal, startYear, endYear);

    freeCity(montreal);
    return 0;
}

int checkParams(char* bikes, char*stations, int startYear, int endYear){
    if(endYear < 0 || startYear < 0) return 0;
    if(startYear != 0 && endYear != 0){
        if(endYear < startYear) return 0;
    }
    if(strcmp(bikes, "biketest.txt") != 0) return 0;
    if(strcmp(stations,"stationtest.txt") != 0) return 0;
    return 1;
}

void readDate(char * s, struct tm * date) {
    sscanf(s, "%d-%d-%d %d:%d:%d", &(date->tm_year), &(date->tm_mon), &(date->tm_mday), &(date->tm_hour), &(date->tm_min), &(date->tm_sec));
}

void query1(cityADT city){
    int cantStations = getStationCount(city);
    int indexVec[cantStations];

    if(getIndexByRank(city, indexVec) == ENOMEM) {
        perror("Error");
        return NO_MEMORY;
    }

    FILE * file;
    file = fopen("query1.csv", "w+");

    if(file == NULL) {
        puts("Cant create file 'query1.csv'");
        perror("Error");
        return CANT_CREATE_FILE;
    }

    htmlTable table = newTable("query1.html", 4, "bikeStation", "memberTrips", "casualTrips", "allTrips");

    if(table == NULL) {
        puts("Cant create file 'query1.html'");
        perror("Error");
        return CANT_CREATE_TABLE;
    }

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

    if(getIndexByAlph(city, indexVec) == ENOMEM) {
        perror("Error");
        return NO_MEMORY;
    }

    FILE * file = fopen("query2.csv", "w+");

    if(file == NULL) {
        puts("Cant create file 'query2.csv'");
        perror("Error");
        return CANT_CREATE_FILE;
    }

    htmlTable table = newTable("query2.html", 3, "bikeStation", "bikeEndStation", "oldestDateTime");
    
    if(table == NULL) {
        puts("Cant create file 'query2.html'");
        perror("Error");
        return CANT_CREATE_TABLE;
    }

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

    if(file == NULL) {
        puts("Cant create file 'quer3.csv'");
        perror("Error");
        return CANT_CREATE_TABLE;
    }

    char * weekVec[7] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    htmlTable table = newTable("query3.html", 3, "weekDay", "startedTrips", "endedTrips");
    
    if(table == NULL) {
        puts("Cant create file 'query3.html'");
        perror("Error");
        return CANT_CREATE_TABLE;
    }

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

    if(getIndexByAlph(city, indexVec) == ENOMEM) {
        perror("Error");
        return NO_MEMORY;
    }

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
