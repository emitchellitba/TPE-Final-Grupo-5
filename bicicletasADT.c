#include "bicicletasADT.h"

#define DAYS_OF_WEEK 7

typedef struct destiny {
    long id;
    tRide * rides;
} tDestiny;

typedef struct ride{
    tDate startDate;
    tDate endDate;
    struct ride * next;
} tRide;

typedef struct station{
    char * name;
    long id;
    tDestiny * destinies;
    long oldestDestiny;
    tDate oldestDate;
    long memberRides;
    long casualRides;
    struct station * next;
} tStation;

typedef struct cityCDT{
    tStation * stations;
    long ridesPerDay[DAYS_OF_WEEK];
    long stationCount;
} cityCDT;




cityADT newCity(void){
    cityADT new = calloc(1, sizeof(cityCDT));       //FALTARIA CHEQUEAR LO DE NULL        

    return new;
}

static
tStation * addStationRec(tStation * station, char * name, long id, int * flag){
    if(station == NULL || station->id > id){
        tStation * new = malloc(sizeof(tStation));         //FALTARIA CHEQUEAR NULL
        new->name = name;
        new->id = id;
        new->routes = NULL;
        new->memberRides = new->casualRides = 0;
        new->next = station;
        *flag = 1;
        return new;
    }
    if(station->id < id)
        station->next = addStationRec(station->next, name, id, flag);
    return station;
}

int addStation(cityADT city, char * name, long id){
        	                                                //FALTARIA CHEQUEAR PARAMETROS (invalido => return -1)
    int flag = 0;
    city->stations = addStationRec(city->stations, name, id, &flag);
    city->stationCount += flag;
    return flag;
}



int addRide(cityADT city, long startStationId, tDate startDate, tDate endDate, long endStationId, char isMember){
                                                    //FALTARIA CHEQUEAR PARAMETROS (invalido => return -1)
    tStation * aux = city->stations;
    tStation * station;
    int foundStart = 0;
    int foundEnd = 0;

    // queda feo con tantos && pero si lo separo recorro dos veces al pedo. como hacerlo mas lindo?

    while(aux != NULL && aux->id < startStationId && aux->id < endStationId && (!foundStart || !foundEnd)){
        if(aux->id == startStationId){
            station = aux;
            foundStart = 1;
        }
        if(aux->id == endStationId)
            foundEnd = 1;

        aux = aux->next;
    }
    if((!foundStart || !foundEnd))
        return -1;                      // el start id o el end id no existen => error!!

    int flag = 0;
    station->rides = addRideRec(station->rides, startDate, endDate, endStationId, isMember, &flag);


        

}




