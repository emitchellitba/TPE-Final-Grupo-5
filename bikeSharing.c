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
#define END_OF_TOKEN ";"
#define SIZE_DATE 18
#define MONTHS 12
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
int query5(cityADT city, int startYear, int endYear);
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

    FILE * bikesCsv = fopen(bikes, "r");
    FILE * stationsCsv = fopen(stations, "r");
    /* Se chequea que los parametros sean los esperados */
    if(status = checkParams(bikesCsv, stationsCsv, startYear, endYear, argc, argv) != OK) {
        if(status == INVALID_ARG) 
            fprintf(stderr, "Invalid arguments");
        if(status == CANT_ARG_ERROR)
            fprintf(stderr, "Invalid amount of arguments")
    }

    cityADT city = newCity();

    if(city == NULL) {
        perror("Error. Can't allocate city");
        return NO_MEMORY;
    }

    /* Aca se verifica si habia memoria para guardar las estaciones y los viajes/destinos */
    if(MON) 
        if(status = addStationsMON(city, stationsCsv) != OK || status = addBikesMON(city, bikesCsv) != OK) {
            freeCity(city);
            return status;
    }

    if(NYC) 
        if(status = addStationsNYC(city, stationsCsv) != OK || status = addBikesNYC(city, bikesCsv) != OK) {
            freeCity(city);
            return status;
    }

    fclose(bikesCsv);
    fclose(stationsCsv);

    if((status = query1(city)) != OK || (status = query2(city)) != OK || (status = query3(city)) != OK || (status = query4(city, startYear, endYear)) != OK) {
        freeCity(city);
        return status;
    }
    
    freeCity(city);
    return status;
}

int addStationsMON(cityADT city, FILE * stationsCsv){

    char aux[MAX_TOKENS];

    /* Se leen y almacenan las estaciones */
    while(fgets(aux, MAX_TOKENS, stationsCsv) != NULL) {
        char * name;
        unsigned long stationId;

        stationId = atoi(strtok(aux, END_OF_TOKEN));
        stationId = atoi(strtok(aux, END_OF_TOKEN));
        name = strtok(NULL, END_OF_TOKEN);
        if(addStation(city, name, stationId) == ENOMEM) {
            perror("Error. Can't allocate station");
            return NO_MEMORY;
        }
    }
    return OK;
}

int addStationsNYC(cityADT city, FILE * stationsCsv){

    char aux[MAX_TOKENS];

    /* Se leen y almacenan las estaciones, omitiendo la primera linea con el encabezado */
    while(fgets(aux, MAX_TOKENS, stationsCsv) != NULL) {
        char *name;
        unsigned long stationId;

        name = strtok(aux, END_OF_TOKEN);
        for (int i = 0; i < 2; i++)
            strtok(NULL, END_OF_TOKEN);                  //ignoramos la longitud y la latitud
        stationId = atoi(strtok(NULL, "\n"));
        if (addStation(city, name, stationId) == ENOMEM) {
            perror("Error. Can't allocate Station");
            return NO_MEMORY;
        }

    }
    return OK;
}

int addBikesMON(cityADT city, FILE * bikesCsv){

    char aux[MAX_TOKENS];
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
            perror("Error. Can't allocate destiny/ride");
            return NO_MEMORY;
        }
    }
    return OK;

}

int addBikesNYC(cityADT city, FILE * bikesCsv){

    char aux[MAX_TOKENS];
    unsigned long startStationId, endStationId;
    struct tm startDate, endDate;
    char * memberState;

    while(fgets(aux, MAX_TOKENS, bikesCsv) != NULL) {
        readDate(strtok(aux, END_OF_TOKEN), &startDate);
        startStationId = atoi(strtok(NULL, END_OF_TOKEN));
        readDate(strtok(NULL, END_OF_TOKEN), &endDate);
        endStationId = atoi(strtok(NULL, END_OF_TOKEN));
        strtok(NULL, END_OF_TOKEN); //ignoramos rideable
        memberState = strtok(NULL, "\n");
        if(addRide(city, startStationId, startDate, endDate, endStationId, strcmp("member", memberState) == 0) == ENOMEM) {
              perror("Error. Cant allocate destiny/ride");
              return NO_MEMORY;
            }
    }
    return OK;
}

int checkParams(FILE* bikes, FILE*stations, int startYear, int endYear, int argc, char * argv[]){

    char aux[MAX_TOKENS];

    if(argc < 3 || argc > 5) {
        return CANT_ARG_ERROR;
    } else {
        bikes = argv[BIKES_FILES];
        stations = argv[STATIONS_FILES];
        if(argc > 3) {
            startYear = atoi(argv[START_YEAR]);
            if(argc == 5) 
                endYear = atoi(argv[END_YEAR]);
        }
    }

    fgets(aux, MAX_TOKENS, stations);
    if(MON && strcmp(aux, PRIMERA_LINEA_STATIONS_MON) != 0 || (NYC && strcmp(aux, PRIMERA_LINEA_STATIONS_NYC) != 0))
        return INVALID_ARG;

    fgets(aux, MAX_TOKENS, bikes);
    if((MON && strcmp(aux, PRIMERA_LINEA_BIKES_MON) != 0) || (NYC && strcmp(aux, PRIMERA_LINEA_BIKES_NYC) != 0))
        return INVALID_ARG;

    if(endYear < 0 || startYear < 0)
        return INVALID_ARG;
    if(startYear != 0 && endYear != 0)
        if(endYear < startYear)
            return INVALID_ARG;

    return OK;
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
        perror("Error. Cant create file 'query1.csv'");
        return CANT_CREATE_FILE;
    }

    htmlTable table = newTable("query1.html", 4, "bikeStation", "memberTrips", "casualTrips", "allTrips");

    if(table == NULL || errno == ENOMEM) {
        perror("Error. Cant create file 'query1.html'");
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
        perror("Error. Cant create file 'query2.csv'");
        return CANT_CREATE_FILE;
    }

    htmlTable table = newTable("query2.html", 3, "bikeStation", "bikeEndStation", "oldestDateTime");

    if(table == NULL) {
        perror("Error. Cant create file 'query3.html'");
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
        perror("Error. Cant create file 'query3.csv'");
        return CANT_CREATE_FILE;
    }

    char * weekVec[7] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    htmlTable table = newTable("query3.html", 3, "weekDay", "startedTrips", "endedTrips");

    if(table == NULL) {
        perror("Error. Cant create file 'query3.html'");
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
        perror("Error. Cant create file 'query4.csv'");
        return CANT_CREATE_FILE;
    }

    fprintf(file, "bikeStation;mostPopRouteEndStation;mostPopRouteTrips\n");
    htmlTable table = newTable("query4.html", 3, "bikeStation", "mostPopRouteEndStation", "mostPopRouteTrips");

    if(table == NULL) {
        perror("Error. Cant create file 'query4.html'");
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

int query5(cityADT city, int startYear, int endYear){

    char * months[12] = {"January", "February", "March", "April", "May", "June",
                         "July", "August", "September", "October", "November", "December"};

    FILE * file = fopen("query5.csv", "w+");

    if(file == NULL) {
        perror("Error. Cant create file 'query5.csv'");
        return CANT_CREATE_FILE;
    }

    fprintf(file, "month;loopsTop1St;loopsTop2St;loopsTop3St\n");

    htmlTable table = newTable("query5.html", 4, "month", "loopsTop1St", "loopsTop2St", "loopsTop3St");

    if(table == NULL) {
        perror("Error. Cant create file 'query5.html'");
        return CANT_CREATE_TABLE;
    }

    char * first, *second, *third;
        for (int j = 0; j < MONTHS; ++j) {
            getTop3ByMonth(j, &first, &second, &third, startYear, endYear);
            fprintf(file, "%s;%s;%s;%s", months[j], first, second, third);
            addHTMLRow(table, months[j], first, second, third);
        }
    closeHTMLTable(table);
    fclose(file);

    return OK;

}
