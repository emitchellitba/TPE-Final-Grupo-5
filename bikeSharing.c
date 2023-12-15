#include "bicicletasADT.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "cTable/htmlTable.h"
#include <stdbool.h>

enum arguments {BIKES_FILES = 1, STATIONS_FILES, START_YEAR, END_YEAR};
enum status {OK = 0, CANT_ARG_ERROR, FILE_NOT_FOUND, INVALID_ARG, NO_MEMORY, CANT_CREATE_FILE, CANT_CREATE_TABLE};

#define MAX_TOKENS 150
#define SIZE_NUM 10
#define LONG_PUNYCOMA 2
#define SIZE_DATE 18
#define PRIMERA_LINEA_BIKES_NYC "started_at;start_station_id;ended_at;end_station_id;rideable_type;member_casual"
#define PRIMERA_LINEA_STATIONS_NYC "station_name;latitude;longitude;id"
#define PRIMERA_LINEA_BIKES_MON "start_date;emplacement_pk_start;end_date;emplacement_pk_end;is_member"
#define PRIMERA_LINEA_STATIONS_MON "pk;name;latitude;longitude"


int checkParams(FILE* bikes, FILE*stations, int startYear, int endYear);
void readDate(char * s, struct tm * date);
int query1(cityADT city);
int query2(cityADT city);
int query3(cityADT city);
int query4(cityADT city, int startYear, int endYear);
int addStationsMon(cityADT city, FILE * stationsCsv);
int addStationsNyc(cityADT city, FILE * stationsCsv);
int addBikesMon(cityADT city, FILE * bikesCsv);
int addBikesNyc(cityADT city, FILE * bikesCsv);




int main(int argc, char * argv[]){
    errno = 0;
    int status = OK;

    /* Si no se pasan años como parametro, permanecen iguales a 0 */
    int startYear = 0, endYear = 0;
    char* bikes, *stations;

    /* Se chequea que se pasen dos (sin años), tres (solo con año de inicio) o cuatro (con ambos años) argumentos */
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

    FILE * bikesCsv = fopen(bikes, "r");
    FILE * stationsCsv = fopen(stations, "r");
    /* Se chequea que los parametros sean los esperados */
    if(!checkParams(bikesCsv, stationsCsv, startYear, endYear)) {
        puts("Invalid arguments");
        return INVALID_ARG;
    }

    cityADT montreal = newCity();
    if(montreal == NULL) {
        puts("Cant allocate city");
        perror("Error");
        return NO_MEMORY;
    }

    if(MON && (status = addStationsMon(montreal, stationsCsv)) != OK) {
        freeCity(montreal);
        return status;
    }

    if(NYC && (status = addStationsNyc(montreal, stationsCsv)) != OK) {
        freeCity(montreal);
        return status;
    }

    if(MON && (status = addBikesMon(montreal, bikesCsv))) != OK) {
        freeCity(montreal);
        return status;
    }

    if(NYC && (status = addBikesNyc(montreal, bikesCsv))) != OK) {
        freeCity(montreal);
        return status;
    }

    fclose(bikesCsv);
    fclose(stationsCsv);

    if((status = query1(montreal)) != OK) {
        freeCity(montreal);
        return status;
    }
    if((status = query2(montreal)) != OK) {
        freeCity(montreal);
        return status;
    }
    if((status = query3(montreal)) != OK) {
        freeCity(montreal);
        return status;
    }
    if((status = query4(montreal, startYear, endYear)) != OK) {
        freeCity(montreal);
        return status;
    }

    freeCity(montreal);
    return status;
}

int addStationsMon(cityADT city, FILE * stationsCsv){

    char aux[MAX_TOKENS];
    char s[LONG_PUNYCOMA] = ";";

    /* Se leen y almacenan las estaciones, omitiendo la primera linea con el encabezado */
    while(fgets(aux, MAX_TOKENS, stationsCsv) != NULL) {
        char * name;
        unsigned long stationId;

        stationId = atoi(strtok(aux, s));
        name = strtok(NULL, s);
            if(addStation(city, name, stationId) == ENOMEM) {
                freeCity(city);
                puts("Cant allocate station");
                perror("Error");
                return NO_MEMORY;
            }
    }

    return OK;


}


int checkParams(FILE* bikes, FILE*stations, int startYear, int endYear){

    char aux[MAX_TOKENS];
    fgets(aux, MAX_TOKENS, stations);
    if(MON && strcmp(aux, PRIMERA_LINEA_STATIONS_MON) != 0)
        return 0;

    if(NYC && strcmp(aux, PRIMERA_LINEA_STATIONS_NYC) != 0)
        return 0;

    fgets(aux, MAX_TOKENS, bikes);
    if(MON && strcmp(aux, PRIMERA_LINEA_BIKES_MON) != 0)
        return 0;

    if(NYC && strcmp(aux, PRIMERA_LINEA_BIKES_MON) != 0)
        return 0;


    if(endYear < 0 || startYear < 0)
        return 0;
    if(startYear != 0 && endYear != 0)
        if(endYear < startYear)
            return 0;


    return 1;
}

/* Genera un string a partir de una fecha en struct tm */
void readDate(char * s, struct tm * date) {
    sscanf(s, "%d-%d-%d %d:%d:%d", &(date->tm_year), &(date->tm_mon), &(date->tm_mday), &(date->tm_hour), &(date->tm_min), &(date->tm_sec));
}

int query1(cityADT city){
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

    if(table == NULL || errno == ENOMEM) {
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

    return OK;
}

int query2(cityADT city){

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
        puts("Cant create file 'query3.html'");
        perror("Error");
        return CANT_CREATE_TABLE;
    }

    char datestr[SIZE_DATE];

    fprintf(file, "bikeStation;bikeEndStation;oldestDateTime\n");
    for (int i = 0; i < cantStations; ++i) {
        char * nameStart, * nameEnd;
        struct tm oldestTime;
        getOldest(city, indexVec[i], &nameStart, &nameEnd, &oldestTime);
        /* Solo se imprime si la estacion tiene viajes que no sean circulares */
        if(nameEnd != NULL){
            fprintf(file, "%s;%s;%d/%d/%d %d:%d\n", nameStart, nameEnd, oldestTime.tm_mday, oldestTime.tm_mon, oldestTime.tm_year,
                    oldestTime.tm_hour, oldestTime.tm_min);
            sprintf(datestr, "%d/%d/%d %d:%d", oldestTime.tm_mday, oldestTime.tm_mon, oldestTime.tm_year, oldestTime.tm_hour, oldestTime.tm_min);
            addHTMLRow(table, nameStart, nameEnd, datestr);
        }
    }
    closeHTMLTable(table);
    fclose(file);

    return OK;
}

int query3(cityADT city) {
    FILE * file;
    file = fopen("query3.csv", "w+");

    if(file == NULL) {
        puts("Cant create file 'query3.csv'");
        perror("Error");
        return CANT_CREATE_FILE;
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

    return OK;
}

int query4(cityADT city, int startYear, int endYear){

    size_t cantStations = getStationCount(city);
    int indexVec[cantStations];

    if(getIndexByAlph(city, indexVec) == ENOMEM) {
        perror("Error");
        return NO_MEMORY;
    }

    FILE * file = fopen("query4.csv", "w+");

    if(file == NULL) {
        puts("Cant create file 'query4.csv'");
        perror("Error");
        return CANT_CREATE_FILE;
    }

    fprintf(file, "bikeStation;mostPopRouteEndStation;mostPopRouteTrips\n");
    htmlTable table = newTable("query4.html", 3, "bikeStation", "mostPopRouteEndStation", "mostPopRouteTrips");

    if(table == NULL) {
        puts("Cant create file 'query4.html'");
        perror("Error");
        return CANT_CREATE_TABLE;
    }

    char numstr[SIZE_NUM];

    for (int i = 0; i < cantStations; ++i) {
        char * endName, * startName = nameByStationIndex(city, indexVec[i]);
        size_t cantRides;
        getMostPopular(city, indexVec[i], &cantRides, &endName, startYear, endYear);
        sprintf(numstr, "%ld", cantRides);
        /* Solo se imprime si la estacion tiene viajes */
        if(endName != NULL) {
            fprintf(file, "%s;%s;%ld\n", startName, endName, cantRides);
            addHTMLRow(table, startName, endName, numstr);
        }
    }
    closeHTMLTable(table);
    fclose(file);

    return OK;
}

int addStationsNyc(cityADT city, FILE * stationsCsv){

    char aux[MAX_TOKENS];
    char s[LONG_PUNYCOMA] = ";";

    /* Se leen y almacenan las estaciones, omitiendo la primera linea con el encabezado */
    while(fgets(aux, MAX_TOKENS, stationsCsv) != NULL) {
        char *name;
        unsigned long stationId;

        name = strtok(aux, ";");
        for (int i = 0; i < 2; i++)
            strtok(NULL, ";");                  //ignoramos la longitud y la latitud
        stationId = atoi(strtok(NULL, "\n"));
        if (addStation(city, name, stationId) == ENOMEM) {
            freeCity(city);
            perror("Error. Can't allocate Station\n");
            return NO_MEMORY;
        }

    }
    return OK;
}

int addBikesMon(cityADT city, FILE * bikesCsv){

    char aux[MAX_TOKENS];
    char s[LONG_PUNYCOMA] = ";";
    unsigned long startStationId, endStationId;
    struct tm startDate, endDate;
    int isMember;

    /* Se leen y almacenan los viajes*/
    while(fgets(aux, MAX_TOKENS, bikesCsv) != NULL) {

        readDate(strtok(aux, s), &startDate);
        startStationId = atoi(strtok(NULL, s));
        readDate(strtok(NULL, s), &endDate);
        endStationId = atoi(strtok(NULL, s));
        isMember = atoi(strtok(NULL, "\n"));
        if (addRide(city, startStationId, startDate, endDate, endStationId, isMember) == ENOMEM) {
            freeCity(city);
            puts("Can't allocate destiny/ride");
            perror("Error");
            return NO_MEMORY;
        }
    }
    return OK;

}

int addBikesNyc(cityADT city, FILE * bikesCsv){

    unsigned long startStationId, endStationId;
    struct tm startDate, endDate;
    char * memberState;

    while(fgets(aux, MAX_TOKENS, bikesCsv) != NULL) {
        readDate(strtok(aux, ";"), &startDate);
        startStationId = atoi(strtok(NULL, ";"));
        readDate(strtok(NULL, ";"), &endDate);
        endStationId = atoi(strtok(NULL, ";"));
        strtok(NULL, ";"); //ignoramos rideable
        memberState = strtok(NULL, "\n");
        if(addRide(city, startStationId, startDate, endDate, endStationId, strcmp("member", memberState) == 0) == ENOMEM) {
              freeCity(city);
              perror("Error. Cant allocate destiny/ride");
              return NO_MEMORY;
            }
    }
    return OK;
}

