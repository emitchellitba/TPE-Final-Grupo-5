#include "bicicletasADT.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>


#define BLOCK 50

/* Nuestro TAD consiste en un vector dinamico donde se almacenan las estaciones (en orden de agregado). Dentro de cada una se almacena
información útil para los queries y un vector dinámico de los destinos de los viajes iniciado en esa estación (tambien en orden de agregado).
Dentro de los vectores de destinos, se almacena su nombre y un puntero al primer elemento de una lista de los viajes realizados entre esas 
dos estaciones, en orden cronológico. Decidimos separar por destino ya que nos facilitaba buscar el destino mas popular (query 4). 
De los vectores dinámicos, nos vamos guardando su dimension para poder recorrerlos. 
Como algunos queries nos solicitaban un orden y otros otro (ej. orden alfabetico o cantidad de viajes), decidimos guardarlos por orden
de agregado y hacer las funciones que nos devuelvan los indices ordenados segun el criterio */

typedef struct ride{
    struct tm start_date, end_date;
    struct ride * next;
} tRide;

typedef struct destiny {
    char * name;
    tRide * rides;
} tDestiny;

typedef struct station{
    struct tm oldest_date;
    char * name;
    char * oldestDestinyName;
    tDestiny * destinies;
    size_t destiniesCount, id, memberRides, casualRides;
} tStation;

typedef struct cityCDT{
    tStation ** stations;
    size_t stationCount, startedRidesPerDay[DAYS_OF_WEEK], endedRidesPerDay[DAYS_OF_WEEK];
    bool ordered:1;
} cityCDT;


/* Estructura usada para hacer listas ordenadas segun la cantidad de viajes o alfabeticamente, para poder crear vectores
con los indices ordenados (para las funciones getIndexByRank y getIndexByAlph)*/
typedef struct index{
    char * name;
    struct index * next;
    size_t totalRides, index;
}tIndex;



cityADT newCity(void){
    return calloc(1, sizeof(cityCDT)); //si retorna NULL, es porque dio error el calloc.
}


int addStation(cityADT city, char * name, size_t id){

    /* Primero nos fijamos que no exista, recorriendo el vector */
    bool found = 0;
    size_t i;
    bool orderFlag = 1;
    for(i = 0; i < city->stationCount && !found; i++){
        if(city->stations[i]->id == id)
            found = 1;
        /* Si se encuentra que el vector de estaciones esta desordenado o se va a desordenar al agregar
        la estacion nueva, notifica que el vector esta desordenado */
        if(city->stations[i]->id > id || (i >= 1 && city->stations[i-1]->id > city->stations[i]->id))
            orderFlag = 0;
    }

    /* Si no esta, se crea */
    if(!found){
        if(i % BLOCK == 0){
            tStation ** aux = city->stations;
            aux = realloc(aux, (i + BLOCK) * sizeof(tStation *));
            if(aux == NULL || errno == ENOMEM) {
                return errno;
            }
            city->stations = aux;
        }
        int len = strlen(name) + 1;
        city->stations[i]->name = malloc(len);
        if(city->stations[i]->name == NULL || errno == ENOMEM) {
            return errno;
        }
        strcpy(city->stations[i]->name, name);
        city->stations[i]->id = id;
        city->stations[i]->destinies = NULL;
        city->stations[i]->destiniesCount = 0;
        city->stations[i]->memberRides = city->stations[i]->casualRides = 0;
        city->stations[i]->oldestDestinyName = NULL;
        city->stationCount++;
        //si se crea una nueva estacion, se apaga el flag de ordered
        //a discutir si sumamos comparaciones para ver que el id sea menor que la ultima estacion del vector
        if(!orderFlag)
            city->ordered = orderFlag; // Si ya estaba desordenado, no tiene porque hacer esta asignacion
    }
    return !found;
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

/*Recibe dos punteros a estacion y compara sus id's*/
size_t compareID(tStation * station1, tStation * station2) {
    return station1->id - station2->id;
}

/* Agrega viaje a la lista en orden cronológico. Si hay dos viajes que salgan al mismo momento
se guardan ambos, en orden de agregado */
static
tRide * addRideRec(tRide * ride, struct tm start_date, struct tm end_date){
    if(ride == NULL || dateCompare(start_date, ride->start_date)){ // tendria q ser !dateCompare(start_date, ride->start_date)?
        tRide * new = malloc(sizeof(tRide));          
        // Si no hay espacio, no se agrega
        if(new == NULL || errno == ENOMEM)
            return ride; 
        new->start_date = start_date;
        new->end_date = end_date;
        new->next = ride;
        return new;
    }
    ride->next = addRideRec(ride->next, start_date, end_date);
    return ride;
}


int addRide(cityADT city, size_t startStationId, struct tm start_date, struct tm end_date, size_t endStationId, int isMember){
    tStation * station;
    size_t i;
    char * endName;
    bool foundStart = 0;
    bool foundEnd = 0;
    
    /*Si el vector no está ordenado, lo ordenamos*/
    if(!city->ordered) {
        qsort(city->stations, city->stationCount, sizeof(tStation *), compareID);
        city->ordered = 1;
    }

    /* Revisamos que las estaciones de origen y final existan. Si las encontramos, nos guardamos los datos necesarios */
    for(i = 0; i < city->stationCount && (!foundStart || !foundEnd); i++){
        if(city->stations[i]->id == startStationId){
            station = &(city->stations[i]);
            foundStart = 1;
        }
        if(city->stations[i]->id == endStationId){
            endName = city->stations[i]->name;                       
            foundEnd = 1;
        }
    }
    // Si ambas existen, agrego el viaje
    if(foundStart && foundEnd){
        
        // Si el destino ya existe dentro de la estacion de origen, lo agrego a esa lista
        bool foundDestiny = 0;
        for(i = 0; i < station->destiniesCount && !foundDestiny; i++){
            if(strcmp(station->destinies[i].name, endName) == 0){
                station->destinies[i].rides = addRideRec(station->destinies[i].rides, start_date, end_date);
                foundDestiny = 1;
            }
        }

        // Si no, creo un destino nuevo e inicializo la lista
        if(!foundDestiny){
            if(i % BLOCK == 0){
                tDestiny * aux = station->destinies;
                aux = realloc(aux, (i + BLOCK) * sizeof(tDestiny));
                if(aux == NULL || errno == ENOMEM) {
                    return errno;
                }
                station->destinies = aux;
            }
            station->destinies[i].name = malloc(strlen(endName) + 1);
            if(station->destinies[i].name == NULL || errno == ENOMEM) {
                return errno;
            }
            strcpy(station->destinies[i].name, endName); 
            station->destinies[i].rides = addRideRec(NULL, start_date, end_date);
            station->destiniesCount++;
        }
        
        /* Si no se pudo agregar el viaje, sea en destino anterior o nuevo, se retorna ENOMEM */
        if(errno == ENOMEM)
            return errno;
        
        /* Nos guardamos el viaje mas viejo para el query 3
        Chequeo que el viaje no sea circular. Si no lo es, lo comparo con el mas viejo registrado (a menos que sea el primero)
         y si es anterior lo reemplazo */
        if(startStationId != endStationId && ((station->memberRides + station->casualRides) == 0 || dateCompare(start_date, station->oldest_date) < 0)){
            char * aux = station->oldestDestinyName;
            aux = realloc(station->oldestDestinyName, strlen(endName) + 1);
            if(aux == NULL || errno == ENOMEM) {
                return errno;
            }
            station->oldestDestinyName = aux;
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
    return errno;
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
        free(city->stations[i]->name);
        free(city->stations[i]->oldestDestinyName);
        for(int j = 0; j < city->stations[i]->destiniesCount; j++){
            free(city->stations[i]->destinies[j].name);
            freeRides(city->stations[i]->destinies[j].rides);
        }free(city->stations[i]->destinies);
    }
    free(city->stations);
    free(city);
}


void ridesByStationIndex(cityADT city, int index, size_t rides[2]){
    rides[0] = city->stations[index]->memberRides;
    rides[1] = city->stations[index]->casualRides;
}

int getStationCount(cityADT city){
    return city->stationCount;
}

char * nameByStationIndex(cityADT city, int index){
    return city->stations[index]->name;
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
tIndex * newNode(tIndex * actual, char * name, size_t totalRides, int index) {
    tIndex * new = malloc(sizeof(tIndex));
    if(new == NULL || errno == ENOMEM)
        return actual;

    new->name = malloc(strlen(name) + 1);
    if(new->name == NULL || errno == ENOMEM) {
        free(new);
        return actual;
    }
    strcpy(new->name, name);
    new->totalRides = totalRides;
    new->index = index;
    new->next = actual;
    return new;
}

static
tIndex * addIndexRankRec(tIndex * actual, char * name, size_t totalRides, int index){
    if(actual == NULL || actual->totalRides <= totalRides) {
        if(actual != NULL && actual->totalRides == totalRides){
            if(strcmp(actual->name, name) < 0){
                actual->next = addIndexRankRec(actual->next, name, totalRides, index);
                return actual;
            }
        }
        return newNode(actual, name, totalRides, index);
    }else{
        actual->next = addIndexRankRec(actual->next, name, totalRides, index);
        return actual;
    }
}


/* Deja en indexVec los indices ordenados según la cantidad total de viajes que tiene esa estación, descendentemente; si tienen 
la misma cantidad de viajes, se pone primero el alfabeticamente menor (requisito query 1). Primero recorremos el vector de estaciones, 
y paralelamente nos armamos una lista ordenada segun el criterio. Cada nodo de la lista almacena la cantidad de viajes y el nombre
de las estacion para realizar las comparaciones correspondientes, y tambien el indice para luego hacer el vector.
Luego pasamos esa lista a un vector y la retornamos. */

int getIndexByRank(cityADT city, int indexVec[]){
    tIndex * lista = NULL;
    for (int i = 0; i < city->stationCount ; i++) {
        lista = addIndexRankRec(lista, city->stations[i]->name, city->stations[i]->casualRides + city->stations[i]->memberRides, i);
    }
    listToArray(lista, city->stationCount, indexVec);
    freeList(lista);
    return errno;
}

static
tIndex * addIndexAlphRec(tIndex * actual, char * name, int index){
    if(actual == NULL || strcmp(actual->name, name) >= 0) {
        return newNode(actual, name, 0, index);
    }else{
        actual->next = addIndexAlphRec(actual->next, name, index);
        return actual;
    }
}

/*Idem getIndexByRank pero con orden aflabetico*/
int getIndexByAlph(cityADT city, int indexVec[]){
    tIndex * lista = NULL;
    for (int i = 0; i < city->stationCount ; i++) {
        lista = addIndexAlphRec(lista, city->stations[i]->name, i);
    }
    listToArray(lista, city->stationCount, indexVec);
    freeList(lista);
    return errno;
}

void getOldest(cityADT city, int index, char ** nameStart, char ** nameEnd, struct tm * oldestTime){

    *nameStart = city->stations[index]->name;
    *nameEnd = city->stations[index]->oldestDestinyName;
    oldestTime->tm_mday = city->stations[index]->oldest_date.tm_mday;
    oldestTime->tm_mon = city->stations[index]->oldest_date.tm_mon;
    oldestTime->tm_year = city->stations[index]->oldest_date.tm_year;
    oldestTime->tm_hour = city->stations[index]->oldest_date.tm_hour;
    oldestTime->tm_min = city->stations[index]->oldest_date.tm_min;
}

size_t getStartedRides(cityADT city, int index) {
    return city->startedRidesPerDay[index];
}

size_t getEndedRides(cityADT city, int index) {
    return city->endedRidesPerDay[index];
}

/*Recibe una lista con los viajes entre una estacion y un destino y retorna la cantidad de viajes entre startYear y endYear*/
static
size_t getRidesBetween(tRide * ride, size_t startYear, size_t endYear){ 

    if(ride == NULL)
        return 0;
    return (startYear == 0 || (ride->start_date.tm_year >= startYear && (endYear == 0 || ride->start_date.tm_year <= endYear)))
        + getRidesBetween(ride->next, startYear, endYear);
}

/*Se guardan en las variables de salida el nombre y cantidad de viajes del destino más popular*/
// void getMostPopular(cityADT city, size_t stationIndex, size_t * ridesOut, char ** endName, int startYear, int endYear){
//     if(city->stations[stationIndex]->destiniesCount > 0){
//         tStation station = city->stations[stationIndex]; //aca no se como hacer, si pasar la variable a puntero o que, pero creo que esta capaz la borramos entonces np
//         /*Se setean las variables con los valores del primer destino*/
//         size_t maxRides = getRidesBetween(station.destinies[0].rides, startYear, endYear);
//         char * maxName = station.destinies[0].name;
      
//         /*Se recorren todos los destinos y se compara con el máximo registrado*/
//         for (int i = 1; i < station.destiniesCount; ++i) {
//             size_t rides =  getRidesBetween(station.destinies[i].rides, startYear, endYear);
//             if(rides > maxRides) {
//                 maxRides = rides;
//                 maxName = station.destinies[i].name;
//             }else if(rides == maxRides){
//                 if(strcmp(maxName, station.destinies[i].name) > 0){
//                     maxRides = rides;
//                     maxName = station.destinies[i].name;
//                 }
//             }
//         }
//         *ridesOut = maxRides;
//         *endName = maxName;
//     }else{
//         /*Si no llega a haber destinos se inicializan en 0 ambas variables*/
//         *ridesOut = 0;
//         *endName = NULL;
//     }
// }
