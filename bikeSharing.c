#include "bikeSharingADT.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "cTable/htmlTable.h"
#include <stdbool.h>

/*enums para mejorar la lectura de los errores y los argumentos*/
enum arguments {BIKES_FILES = 1, STATIONS_FILES, START_YEAR, END_YEAR};
enum status {OK = 0, CANT_ARG_ERROR, FILE_NOT_FOUND, INVALID_ARG, NO_MEMORY, CANT_CREATE_FILE, CANT_CREATE_TABLE, ITER_ERROR};

#define MAX_TOKENS 150
#define SIZE_NUM 50
#define END_OF_TOKEN ";"
#define END_OF_LINE "\n"
#define SIZE_DATE 18
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
int saveStations(cityADT city, FILE * stationsCsv);
int saveRides(cityADT city, FILE * bikesCsv, int startYear, int endYear);
void getStation(char * aux, char ** name, unsigned long * stationId);
void getRide(char * aux, unsigned long * startStationId, struct tm * startDate, struct tm * endDate, unsigned long * endStationId, int * isMember);
int query1(cityADT city);
int query2(cityADT city);
int query3(cityADT city);
int query4(cityADT city);
int query5(cityADT city);

int main(int argc, char * argv[]){
    errno = 0;
    int status = OK;

    /* Si no se pasan años como parametro, permanecen iguales a 0 */
    int startYear = 0, endYear = 0;
    char* bikes, *stations;

    /* Chequeamos la cantidad de parametros*/
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
    if(city == NULL){
        return NO_MEMORY;
    }

    /*Se guardan las estaciones*/
    if((status = saveStations(city, stationsCsv)) == NO_MEMORY){
        return status;
    }

    /*Se guardan los viajes*/
    if((status = saveRides(city, bikesCsv, startYear, endYear)) == NO_MEMORY){
        return status;
    }

    fclose(bikesCsv);
    fclose(stationsCsv);

    /*Aca se invocan las queries, y tambien se verifica que todas hayan funcionado; sino se libera la ciudad y retorna status*/
    if((status = query1(city)) != OK || (status = query2(city)) != OK || (status = query3(city)) != OK || 
        (status = query4(city)) != OK || (status = query5(city)) != OK) {
        freeCity(city);
        return status;
    }
    
    freeCity(city);
    return status;
}

/* Ciclo para guardar las estaciones */
int saveStations(cityADT city, FILE * stationsCsv){
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
    return OK;
}

/* Ciclo para guardar los viajes */
int saveRides(cityADT city, FILE * bikesCsv, int startYear, int endYear){
    char aux[MAX_TOKENS];
    unsigned long startStationId, endStationId;
    struct tm startDate, endDate;
    int isMember;
    while(fgets(aux, MAX_TOKENS, bikesCsv) != NULL) {
        getRide(aux, &startStationId, &startDate, &endDate, &endStationId, &isMember);
        if (addRide(city, startStationId, startDate, endDate, endStationId, isMember, startYear, endYear) == ENOMEM) {
            perror("Error. Can't allocate destiny/ride");
            return NO_MEMORY;
        }
    }
    return OK;
}

/*Lee la estacion, recopilando unicamente los datos relevantes*/
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

/*Idem para viajes*/
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

/*Se chequea que los parametros fueron ingresados correctamente*/
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
    /*Se copia a mano la primera linea siguiendo la estructura del archivo. Esto se sigue para el resto de las queries*/
    fprintf(file, "bikeStation;memberTrips;casualTrips;allTrips\n");
    char memstr[SIZE_NUM], casstr[SIZE_NUM], allnum[SIZE_NUM];

    orderByRides(city);
    toBegin(city);
    while(hasNext (city)){
        tTotalRides rides = nextTotalRides(city);
        fprintf(file, "%s;%ld;%ld;%ld\n", rides.name, rides.memberRides, rides.casualRides, rides.memberRides + rides.casualRides);
        sprintf(memstr, "%ld", rides.memberRides);
        sprintf(casstr, "%ld", rides.casualRides);
        sprintf(allnum, "%ld", rides.memberRides + rides.casualRides);
        addHTMLRow(table, rides.name, memstr, casstr, allnum);
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
        tOldest oldest = nextOldest(city);
        /* Solo se imprime si la estacion tiene viajes que no sean circulares */
        if(oldest.destinyName != NULL){
            fprintf(file, "%s;%s;%d/%02d/%02d %02d:%02d\n", oldest.name, oldest.destinyName, oldest.date.tm_mday, oldest.date.tm_mon,
                    oldest.date.tm_year, oldest.date.tm_hour, oldest.date.tm_min);
            sprintf(datestr, "%d/%02d/%02d %02d:%02d", oldest.date.tm_mday, oldest.date.tm_mon, oldest.date.tm_year,
                    oldest.date.tm_hour, oldest.date.tm_min);
            addHTMLRow(table, oldest.name, oldest.destinyName, datestr);
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

int query4(cityADT city){

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
        tMostPopular mostPopular = nextMostPopular(city);
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

int query5(cityADT city){

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
            getTop3ByMonth(city, j, &first, &second, &third);
            fprintf(file, "%s;%s;%s;%s\n", months[j], first, second, third);
            addHTMLRow(table, months[j], first, second, third);
        }
    closeHTMLTable(table);
    fclose(file);

    return OK;

}