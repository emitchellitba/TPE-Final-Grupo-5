#include "bicicletasADT.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define DAYS_OF_WEEK 7
#define BLOCK 50

typedef struct index{
    char * name;
    size_t totalRides;
    int index;
    struct index * next;
}tIndex;

/* Nuestro TAD consiste en un vector dinamico donde se almacenan las estaciones (en orden de agregado). Dentro de cada una se almacena
información útil para los queries y un vector dinámico de los destinos de los viajes iniciado en esa estación (tambien en orden de agregado).
Dentro de los vectores de destinos, se almacena su nombre y un puntero al primer elemento de una lista de los viajes realizados entre esas 
dos estaciones, en orden cronológico. Decidimos separar por destino ya que nos facilitaba buscar el destino mas popular (query 4). 
De los vectores dinámicos, nos vamos guardando su dimension para poder recorrerlos. 
Como algunos queries nos solicitaban un orden y otros otro (ej. orden alfabetico o cantidad de viajes), decidimos guardarlos por orden
de agregado y hacer las funciones que nos devuelvan los indices ordenados segun el criterio */

typedef struct ride{
    struct tm start_date;
    struct tm end_date;
    struct ride * next;
} tRide;

typedef struct destiny {
    char * name;
    tRide * rides;
} tDestiny;

typedef struct station{
    char * name;
    size_t id;
    tDestiny * destinies;
    size_t destiniesCount;
    char * oldestDestinyName;
    struct tm oldest_date;
    size_t memberRides;
    size_t casualRides;
} tStation;

typedef struct cityCDT{
    tStation * stations;
    size_t startedRidesPerDay[DAYS_OF_WEEK];
    size_t endedRidesPerDay[DAYS_OF_WEEK];
    size_t stationCount;
} cityCDT;




cityADT newCity(void){
    cityADT new = calloc(1, sizeof(cityCDT));

    return new; //si retorna NULL, es porque dio error el calloc.
}


int addStation(cityADT city, char * name, size_t id){
    errno = 0;
    /* Primero nos fijamos que no exista, recorriendo el vector */
    int esta = 0;
    size_t i;
    for(i = 0; i < city->stationCount && !esta; i++){
        if(city->stations[i].id == id)
            esta = 1;
    }
    
    /* Si no esta, se crea */
    if(!esta){
        if(i % BLOCK == 0){
            tStation * aux = city->stations;
            aux = realloc(aux, (i + BLOCK) * sizeof(tStation));
            if(aux == NULL || errno == ENOMEM) {
                //Que hacer en este caso?
            }
        }
        errno = 0;
        city->stations[i].name = malloc(strlen(name) + 1);
        if(city->stations[i].name == NULL || errno == ENOMEM) {
            //?
        }
        strcpy(city->stations[i].name, name);
        city->stations[i].id = id;
        city->stations[i].destinies = NULL;
        city->stations[i].destiniesCount = 0;
        city->stations[i].memberRides = city->stations[i].casualRides = 0;
        city->stations[i].oldestDestinyName = NULL;
        city->stationCount++;
    }
    return !esta;
}


/* Compara las fechas. Retorna un numero negativo si la primera es anterior, positivo si es posterior
y cero si son iguales */
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


/* Agrega viaje a la lista en orden cronológico. Si hay dos viajes que salgan al mismo momento
se guardan ambos, en orden de agregado */
static
tRide * addRideRec(tRide * ride, struct tm start_date, struct tm end_date){
    int cmp;
    if(ride == NULL || (cmp = dateCompare(start_date, ride->start_date)) <= 0){
        errno = 0;
        tRide * new = malloc(sizeof(tRide));          
        if(new == NULL || errno == ENOMEM) {
            //?
        }
        new->start_date = start_date;
        new->end_date = end_date;
        new->next = ride;
        return new;
    }
    addRideRec(ride->next, start_date, end_date);
    return ride;
}


void addRide(cityADT city, size_t startStationId, struct tm start_date, struct tm end_date, size_t endStationId, int isMember){
    tStation * station;
    size_t i;
    char * endName;
    int foundStart = 0;
    int foundEnd = 0;

    
    /* Revisamos que las estaciones de origen y final existan. Si las encontramos, nos guardamos los datos necesarios */
    for(i = 0; i < city->stationCount && (!foundStart || !foundEnd); i++){
        if(city->stations[i].id == startStationId){
            station = &(city->stations[i]);
            foundStart = 1;
        }
        if(city->stations[i].id == endStationId){
            endName = city->stations[i].name;                       
            foundEnd = 1;
        }
    }
    // Si ambas existen, agrego el viaje
    if((foundStart && foundEnd)){
        
        // Si el destino ya existe dentro de la estacion de origen, lo agrego a esa lista
        int foundDestiny = 0;
        for(i = 0; i < station->destiniesCount && !foundDestiny; i++){
            if(strcmp(station->destinies[i].name, endName) == 0){
                station->destinies[i].rides = addRideRec(station->destinies[i].rides, start_date, end_date);
                foundDestiny = 1;
            }
        }
        
        // Si no, creo un destino nuevo e inicializo la lista
        if(!foundDestiny){
            if(i % BLOCK == 0){
                errno = 0;
                tStation * aux = station->destinies;
                aux = realloc(aux, (i + BLOCK) * sizeof(tDestiny));
                if(aux == NULL || errno == ENOMEM) {
                    //?
                }
            }
            errno = 0; //dependiendo que hagamos arriba esto se saca o se deja!!!
            station->destinies[i].name = malloc(strlen(endName) + 1);
            if(station->destinies[i].name == NULL || errno == ENOMEM) {
                //?
            }
            strcpy(station->destinies[i].name, endName); 
            station->destinies[i].rides = addRideRec(NULL, start_date, end_date);
            station->destiniesCount++;
        }
        
        /* Nos guardamos el viaje mas viejo para el query 3
        Chequeo que el viaje no sea circular. Si no lo es, lo comparo con el mas viejo registrado (a menos que sea el primero)
         y si es anterior lo reemplazo */
        if(startStationId != endStationId && ((station->memberRides + station->casualRides) == 0 || dateCompare(start_date, station->oldest_date) < 0)){
            errno = 0;
            char * aux = station->oldestDestinyName;
            aux = realloc(station->oldestDestinyName, strlen(endName) + 1);
            if(aux == NULL || errno == ENOMEM) {
                //?
            }
            strcpy(station->oldestDestinyName, endName);
            station->oldest_date = start_date;
        }

        /* Separamos la suma de viajes totales segun miembro o no para el query 1 y sumamos adecuadamente */
        if(isMember)
            station->memberRides++;
        else
            station->casualRides++;
    
        /* Para el query 2, registramos que dia de la semana se inicio y termino el viaje */
        mktime(&start_date);
        city->startedRidesPerDay[start_date.tm_wday]++;
        mktime(&end_date);
        city->endedRidesPerDay[end_date.tm_wday]++;

    }
}

static
void freeRides(tRide * ride){
    if(ride == NULL)
        return;
    freeRides(ride->next);
    free(ride);
}

void freeCity(cityADT city){
    for(int i = 0; i < city->stationCount; i++){
        free(city->stations[i].name);
        free(city->stations[i].oldestDestinyName);
        for(int j = 0; j < city->stations[i].destiniesCount; j++){
            free(city->stations[i].destinies[j].name);
            freeRides(city->stations[i].destinies[j].rides);
        }free(city->stations[i].destinies);
    }
    free(city->stations);
    free(city);
}

// FUNCIONES QUERY 1

void ridesByStationIndex(cityADT city, int index, size_t rides[2]){
    rides[0] = city->stations[index].memberRides;
    rides[1] = city->stations[index].casualRides;
}

int getStationCount(cityADT city){
    return city->stationCount;
}

char * nameByStationIndex(cityADT city, int index){
    return city->stations[index].name;
}

static
void listToArray(tIndex * list, size_t size, int indexVec[]){
    for (int i = 0; i < size; ++i) {
        indexVec[i] = list->index;
        list = list->next;
    }
}

static
void freeList(tIndex * lista){
    tIndex * aux;
    while(lista != NULL) {
        aux = lista->next;
        free(lista->name);
        free(lista);
        lista = aux;
    }
}


static
tIndex * addIndexRec(tIndex * actual, char * name, size_t totalRides, int index){
    if(actual == NULL || actual->totalRides <= totalRides) {
        if(actual != NULL && actual->totalRides == totalRides){
            if(strcmp(actual->name, name) < 0){
                actual->next = addIndexRec(actual->next, name, totalRides, index);
                return actual;
            }
        }
        errno = 0;
        tIndex * new = malloc(sizeof(tIndex));
        if(new == NULL || errno == ENOMEM) {
            //?
        }
        new->name = malloc(strlen(name) + 1);
        if(new->name == NULL || errno == ENOMEM) {
            //?
        }
        strcpy(new->name, name);
        new->totalRides = totalRides;
        new->index = index;
        new->next = actual;
        return new;
    }else{
        actual->next = addIndexRec(actual->next, name, totalRides, index);
        return actual;
    }
}

void getIndexByRank(cityADT city, int indexVec[]){

    tIndex * lista = NULL;
    for (int i = 0; i < city->stationCount ; i++) {
        lista = addIndexRec(lista, city->stations[i].name, city->stations[i].casualRides + city->stations[i].memberRides, i);
    }
    listToArray(lista, city->stationCount, indexVec);
    freeList(lista);
}

static
tIndex * addIndexAlphRec(tIndex * actual, char * name, int index){
    if(actual == NULL || strcmp(actual->name, name) >= 0) {
        if(actual != NULL && strcmp(actual->name, name) == 0){
            if(index  > actual->index){
                actual->next = addIndexAlphRec(actual->next, name, index);
                return actual;
            }
        }
        errno = 0;
        tIndex * new = malloc(sizeof(tIndex));
        if(new == NULL || errno == ENOMEM) {
            //?
        }
        new->name = malloc(strlen(name) + 1);
        // errno = 0;
        if(new->name == NULL || errno == ENOMEM) {
            //?
        }
        strcpy(new->name, name);
        new->index = index;
        new->next = actual;
        return new;
    }else{
        actual->next = addIndexAlphRec(actual->next, name, index);
        return actual;
    }
}

void getIndexByAlph(cityADT city, int indexVec[]){

    tIndex * lista = NULL;
    for (int i = 0; i < city->stationCount ; i++) {
        lista = addIndexAlphRec(lista, city->stations[i].name, i);
    }
    listToArray(lista, city->stationCount, indexVec);
    freeList(lista);
}

void getOldest(cityADT city, int index, char ** nameStart, char ** nameEnd, struct tm * oldestTime){

    *nameStart = city->stations[index].name;
    *nameEnd = city->stations[index].oldestDestinyName;
    oldestTime->tm_mday = city->stations[index].oldest_date.tm_mday;
    oldestTime->tm_mon = city->stations[index].oldest_date.tm_mon;
    oldestTime->tm_year = city->stations[index].oldest_date.tm_year;
    oldestTime->tm_hour = city->stations[index].oldest_date.tm_hour;
    oldestTime->tm_min = city->stations[index].oldest_date.tm_min;
}

size_t getStartedRides(cityADT city, int index) {
    return city->startedRidesPerDay[index];
}

size_t getEndedRides(cityADT city, int index) {
    return city->endedRidesPerDay[index];
}

static size_t getRidesBetween(tRide * ride, size_t startYear, size_t endYear){

    if(ride == NULL)
        return 0;
    return (startYear == 0 || (ride->start_date.tm_year >= startYear && (endYear == 0 || ride->start_date.tm_year <= endYear)))
        + getRidesBetween(ride->next, startYear, endYear);
}

void getMostPopular(cityADT city, size_t stationIndex, size_t * ridesOut, char ** endName, int startYear, int endYear){
    if(city->stations[stationIndex].destiniesCount > 0){
        tStation station = city->stations[stationIndex];
        size_t maxRides = getRidesBetween(station.destinies[0].rides, startYear, endYear);
        char * maxName = station.destinies[0].name;

        for (int i = 1; i < station.destiniesCount; ++i) {
            size_t rides =  getRidesBetween(station.destinies[i].rides, startYear, endYear);
            if(rides > maxRides) {
                maxRides = rides;
                maxName = station.destinies[i].name;
            }else if(rides == maxRides){
                if(strcmp(maxName, station.destinies[i].name) > 0){
                    maxRides = rides;
                    maxName = station.destinies[i].name;
                }
            }
        }
        *ridesOut = maxRides;
        *endName = maxName;
    }else{
        *ridesOut = 0;
        *endName = NULL;
    }
}
