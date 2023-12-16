#include "bicicletasADT.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "cTable/htmlTable.h"
#include <stdbool.h>

enum arguments {BIKES_FILES = 1, STATIONS_FILES, START_YEAR, END_YEAR};
enum status {OK = 0, CANT_ARG_ERROR, FILE_NOT_FOUND, INVALID_ARG, NO_MEMORY, CANT_CREATE_FILE, CANT_CREATE_TABLE, ITER_ERROR};

#define MAX_TOKENS 150
#define SIZE_NUM 10
#define END_OF_TOKEN ";"
#define END_OF_LINE "\n"
#define SIZE_DATE 18
#define MONTHS 12
#define FIRST_LINE_BIKES ""
#define FIRST_LINE_STATIONS ""

#ifdef NYC
    #undef FIRST_LINE_BIKES
    #undef FIRST_LINE_STATIONS
    #define FIRST_LINE_BIKES "started_at;start_station_id;ended_at;end_station_id;rideable_type;member_casual\n"
    #define FIRST_LINE_STATIONS "station_name;latitude;longitude;id\n"
#endif
#ifdef MON
    #undef FIRST_LINE_BIKES
    #undef FIRST_LINE_STATIONS
    #define FIRST_LINE_BIKES "start_date;emplacement_pk_start;end_date;emplacement_pk_end;is_member\n"
    #define FIRST_LINE_STATIONS "pk;name;latitude;longitude\n"
#endif


int checkParams(FILE* bikes, FILE*stations, int startYear, int endYear);
void readDate(char * s, struct tm * date);
int query1(cityADT city);
int query2(cityADT city);
int query3(cityADT city);
int query4(cityADT city, int startYear, int endYear);
int query5(cityADT city, int startYear, int endYear);
void getStation(char * aux, char ** name, unsigned long * stationId);
void getRide(char * aux, unsigned long * startStationId, struct tm * startDate, struct tm * endDate, unsigned long * endStationId, int * isMember);

int main(int argc, char * argv[]){
    errno = 0;
    int status = OK;

    /* Si no se pasan años como parametro, permanecen iguales a 0 */
    int startYear = 0, endYear = 0;
    char* bikes, *stations;

    if(argc < 3 || argc > 5) {
        fprintf(stderr, "Invalid amount of arguments");
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


    FILE * bikesCsv = fopen(bikes, "r");
    FILE * stationsCsv = fopen(stations, "r");
    /* Se chequea que los parametros sean los esperados */
    if((status = checkParams(bikesCsv, stationsCsv, startYear, endYear)) != OK) {
        if(status == INVALID_ARG) {
            fprintf(stderr, "Invalid arguments");
            return INVALID_ARG;
        }
        if(status == CANT_ARG_ERROR) {
            fprintf(stderr, "Invalid amount of arguments");
            return INVALID_ARG;
        }
    }

    cityADT city = newCity();


    char aux[MAX_TOKENS];
    while(fgets(aux, MAX_TOKENS, stationsCsv) != NULL) {
        char * name;
        unsigned long stationId;
        getStation(aux, &name, &stationId);
        if(addStation(city, name, stationId) == ENOMEM) {
            perror("Error. Can't allocate station");
            return NO_MEMORY;
        }
    }

    unsigned long startStationId, endStationId;
    struct tm startDate, endDate;
    int isMember;
    while(fgets(aux, MAX_TOKENS, bikesCsv) != NULL) {
        getRide(aux, &startStationId, &startDate, &endDate, &endStationId, &isMember);
        if (addRide(city, startStationId, startDate, endDate, endStationId, isMember) == ENOMEM) {
            perror("Error. Can't allocate destiny/ride");
            return NO_MEMORY;
        }
    }

    fclose(bikesCsv);
    fclose(stationsCsv);

    if((status = query1(city)) != OK || (status = query2(city)) != OK || (status = query3(city)) != OK || (status = query4(city, startYear, endYear)) != OK || (status = query5(city, startYear, endYear)) != OK) {
        freeCity(city);
        return status;
    }
    
    freeCity(city);
    return status;
}

void getStation(char * aux, char ** name, unsigned long * stationId) {
    #ifdef NYC
        *name = strtok(aux, END_OF_TOKEN);
        for (int i = 0; i < 2; i++)
            strtok(NULL, END_OF_TOKEN);                  //ignoramos la longitud y la latitud
        *stationId = atoi(strtok(NULL, END_OF_LINE));
    #endif
    
    #ifdef MON
        *stationId = atoi(strtok(aux, END_OF_TOKEN));
        *name = strtok(NULL, END_OF_TOKEN);
    #endif

}

void getRide(char * aux, unsigned long * startStationId, struct tm * startDate, struct tm * endDate, unsigned long * endStationId, int * isMember) {
    #ifdef NYC
        readDate(strtok(aux, END_OF_TOKEN), startDate);
        *startStationId = atoi(strtok(NULL, END_OF_TOKEN));
        readDate(strtok(NULL, END_OF_TOKEN), endDate);
        *endStationId = atoi(strtok(NULL, END_OF_TOKEN));
        strtok(NULL, END_OF_TOKEN); //ignoramos rideable
        *isMember = (strcmp(strtok(NULL, "\n"), "member") == 0);
    #endif

    #ifdef MON
        readDate(strtok(aux, END_OF_TOKEN), startDate);
        *startStationId = atoi(strtok(NULL, END_OF_TOKEN));
        readDate(strtok(NULL, END_OF_TOKEN), endDate);
        *endStationId = atoi(strtok(NULL, END_OF_TOKEN));
        *isMember = atoi(strtok(NULL, "\n"));
    #endif
}


int checkParams(FILE* bikes, FILE*stations, int startYear, int endYear){
    char aux[MAX_TOKENS];

    fgets(aux, MAX_TOKENS, stations);
    if(strcmp(aux, FIRST_LINE_STATIONS) != 0)
        return INVALID_ARG;
    fgets(aux, MAX_TOKENS, bikes);
    if(strcmp(aux, FIRST_LINE_BIKES) != 0)
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

    FILE * file = fopen("query1.csv", "w+");

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

    orderByRides(city);
    toBegin(city);
    while(hasNext (city)){
        tData data = next(city);
        fprintf(file, "%s;%ld;%ld;%ld\n", data.name, data.memberRides, data.casualRides, data.memberRides + data.casualRides);
        sprintf(memstr, "%ld", data.memberRides);
        sprintf(casstr, "%ld", data.casualRides);
        sprintf(allnum, "%ld", data.memberRides + data.casualRides);
        addHTMLRow(table, data.name, memstr, casstr, allnum);
    }
    closeHTMLTable(table);
    fclose(file);

    return OK;
}

int query2(cityADT city){
 
    FILE * file = fopen("query2.csv", "w+");
    
    if(file == NULL) {
        perror("Error. Cant create file 'query2.csv'");
        return CANT_CREATE_FILE;
    }

    htmlTable table = newTable("query2.html", 3, "bikeStation", "bikeEndStation", "oldestDateTime");

    if(table == NULL) {
        perror("Error. Cant create file 'query2.html'");
        return CANT_CREATE_TABLE;
    }

    char datestr[SIZE_DATE];
    fprintf(file, "bikeStation;bikeEndStation;oldestDateTime\n");

    orderByAlph(city);
    toBegin(city);
    while(hasNext(city)){
        tData data = next(city);
        /* Solo se imprime si la estacion tiene viajes que no sean circulares */
        if(data.oldestDestinyName != NULL){
            fprintf(file, "%s;%s;%d/%d/%d %02d:%02d\n", data.name, data.oldestDestinyName, data.oldest_date.tm_mday, data.oldest_date.tm_mon,
                    data.oldest_date.tm_year, data.oldest_date.tm_hour, data.oldest_date.tm_min);
            sprintf(datestr, "%d/%d/%d %02d:%02d", data.oldest_date.tm_mday, data.oldest_date.tm_mon, data.oldest_date.tm_year,
                    data.oldest_date.tm_hour, data.oldest_date.tm_min);
            addHTMLRow(table, data.name, data.oldestDestinyName, datestr);
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

    orderByAlph(city);
    toBegin(city);
    while(hasNext(city)){
        tMostPopular mostPopular = nextMostPopular(city, startYear, endYear);
        sprintf(numstr, "%ld", mostPopular.cantRides);
        /* Solo se imprime si la estacion tiene viajes */
        if(mostPopular.endName != NULL) {
            fprintf(file, "%s;%s;%ld\n", mostPopular.name, mostPopular.endName, mostPopular.cantRides);
            addHTMLRow(table, mostPopular.name, mostPopular.endName, numstr);
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
            getTop3ByMonth(city, j, &first, &second, &third, startYear, endYear);
            fprintf(file, "%s;%s;%s;%s", months[j], first, second, third);
            addHTMLRow(table, months[j], first, second, third);
        }
    closeHTMLTable(table);
    fclose(file);

    return OK;

}


