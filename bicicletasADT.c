#include "bicicletasADT.h"
#include <stdlib.h>
#include <stddef.h>
#include <time.h>

#define DAYS_OF_WEEK 7
#define BLOCK 50


typedef struct ride{
    struct tm start_date;
    struct tm end_date;
    struct ride * next;
} tRide;

typedef struct destiny {
    unsigned long index;
    tRide * rides;
} tDestiny;

typedef struct station{
    char * name;
    unsigned long id;
    tDestiny * destinies;
    unsigned long destiniesCount;
    unsigned long oldestDestinyIdx;
    struct tm oldest_date;
    unsigned long memberRides;
    unsigned long casualRides;
} tStation;

typedef struct cityCDT{
    tStation * stations;
    unsigned long ridesPerDay[DAYS_OF_WEEK];
    unsigned long stationCount;
} cityCDT;




cityADT newCity(void){
    cityADT new = calloc(1, sizeof(cityCDT));       //FALTARIA CHEQUEAR LO DE NULL        

    return new;
}


int addStation(cityADT city, char * name, unsigned long id){
    int esta = 0;
    unsigned long i;
    for(i = 0; i < city->stationCount && !esta; i++){
        if(city->stations[i].id == id)
            esta = 1;
    }
    if(!esta){
        if(i % BLOCK == 0){
            city->stations = realloc(city->stations, (i + BLOCK) * sizeof(tStation));
            //FALTARIA CHEQEUAR NULL
        }
        city->stations[i].name = name;
        city->stations[i].id = id;
        city->stations[i].destinies = NULL;
        city->stations[i].destiniesCount = 0;
        city->stations[i].memberRides = city->stations[i].casualRides = 0;
        city->stationCount++;
    }
    return !esta;
}


static
int dateCompare(struct tm d1, struct tm d2){
    int diff;
    if((diff = d1.tm_year - d2.tm_year) == 0)
        if((diff = d1.tm_mon - d2.tm_mon) == 0)
            if((diff = d1.tm_mday - d2.tm_mday) == 0)
                if((diff = d1.tm_hour - d2.tm_hour) == 0)
                    if((diff = d1.tm_min - d2.tm_min) == 0)
                        diff = d1.tm_sec - d2.tm_sec;
    return diff;
}

static
tRide * addRideRec(tRide * ride, struct tm start_date, struct tm end_date){
    int cmp;
    if(ride == NULL || (cmp = dateCompare(start_date, ride->start_date)) < 0){
        tRide * new = malloc(sizeof(tRide));                    //FALTARIA CHEQUEAR NULL
        new->start_date = start_date;
        new->end_date = end_date;
        new->next = ride;
        return new;
    }
    if(cmp > 0)
        addRideRec(ride->next, start_date, end_date);
    return ride;
}


// Queda muy larga. Podriamos modularizar? Quizas no tiene sentido igual pq no lo volveriamos a usar
int addRide(cityADT city, unsigned long startStationId, struct tm start_date, struct tm end_date, unsigned long endStationId, char isMember){
                                                    //FALTARIA CHEQUEAR PARAMETROS (invalido => return -1) (fechas, is Member maybe)
                                                    //FALTARIA CONSIDERAR CASO QUE SALGA Y VUELVA AL MISMO LUGAR (o directo en las funciones?)
    tStation station;
    unsigned long i, endIndex;
    int foundStart = 0;
    int foundEnd = 0;

    for(i = 0; i < city->stationCount && (!foundStart || !foundEnd); i++){
        if(city->stations[i].id == startStationId){
            station = city->stations[i];
            foundStart = 1;
        }
        if(city->stations[i].id == endStationId){
            endIndex = i;
            foundEnd = 1;
        }
    }
    if((foundStart && foundEnd)){
        int foundDestiny = 0;
        for(i = 0; i < station.destiniesCount && !foundDestiny; i++){
            if(station.destinies[i].index == endIndex){
                station.destinies[i].rides = addRideRec(station.destinies[i].rides, start_date, end_date);
                foundDestiny = 1;
            }
        }
        if(!foundDestiny){
            if(i % BLOCK == 0){
                station.destinies = realloc(station.destinies, (i + BLOCK) * sizeof(tDestiny));
                //FALTARIA CHEQUeAR NULL
            }
            station.destinies[i].index = endIndex;
            station.destinies[i].rides = addRideRec(NULL, start_date, end_date);
            station.destiniesCount++;
        }
        
        if(station.memberRides + station.casualRides == 0 || dateCompare(start_date, station.oldest_date) < 0){
            station.oldestDestinyIdx = endIndex;
            station.oldest_date = start_date;
        }

        if(isMember)
            station.memberRides++;
        else
            station.casualRides++;
        
        mktime(&start_date);
        city->ridesPerDay[start_date.tm_wday]++;
    }
    return 0;
}