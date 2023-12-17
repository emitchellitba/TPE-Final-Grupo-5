#include "bicicletasADT.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>


#define BLOCK 50
#define NOT_FOUND -1
#define CHECK_ERRNO if(errno == ENOMEM) return errno
#define DATE_RANGE (startYear == 0 || start_date.tm_year >= startYear) && (endYear == 0 || end_date.tm_year <= endYear)

enum orderedBy {noOrder = 0, idOrder, ridesOrder, alphOrder};

/* Nuestro TAD consiste en un vector dinamico donde se almacenan las estaciones. Dentro de cada una se almacena informacion relevante
para los queries. Particularmente, para la query 5 se tiene un vector estatico de MONTHS = 12 espacios donde se guardan la cantidad
de viajes circulares mensuales por estacion dentro del rango de años especificado. Ademas, para la query 4 se tiene un arbol binario
(no balanceado) con los destinos; en cada uno se guarda su nombre, su id y la cantidad de viajes hacia el desde la estacion original
en el rango de años indicado. 
    El vector de estaciones se almacena desordenado, pero hay funciones para ordenarlo de distintas maneras. Por ello, se tiene un flag
order para indicar el tipo de orden actual.
*/

typedef struct destiny {
    char * name;
    size_t id;
    size_t rideCount;
    struct destiny * nextLeft;
    struct destiny * nextRight;
} tDestiny;

typedef struct station{
    struct tm oldest_date;
    char * name;
    char * oldestDestinyName;
    size_t monthlyCircularRides[MONTHS];
    tDestiny * destinies;
    char * mostPopName;
    size_t mostPopRides;
    size_t id, memberRides, casualRides;
} tStation;

typedef struct cityCDT{
    tStation ** stations;
    size_t stationCount, startedRidesPerDay[DAYS_OF_WEEK], endedRidesPerDay[DAYS_OF_WEEK];
    size_t iter;
    int order;
} cityCDT;



cityADT newCity(void){
    return calloc(1, sizeof(cityCDT));          //si retorna NULL, es porque dio error el calloc.
}


int addStation(cityADT city, char * name, size_t id){
    errno = 0;
    bool orderFlag = idOrder;
    bool found = 0;
    /* Primero nos fijamos que no exista, recorriendo el vector */
    for(size_t i = 0; i < city->stationCount && !found; i++){
        if(city->stations[i]->id == id)
            found = 1;
        /* Si se encuentra que el vector de estaciones esta desordenado en funcion de sus ids o se va a desordenar al agregar
        la estacion nueva, notifica que el vector esta desordenado */
        if(orderFlag != noOrder && (city->stations[i]->id > id || (i >= 1 && city->stations[i-1]->id > city->stations[i]->id)))
            orderFlag = noOrder;
    }

    /* Si no esta, se crea */
    if(!found){
        if(city->stationCount % BLOCK == 0){
            tStation ** aux1 = city->stations;
            aux1 = realloc(aux1, (city->stationCount + BLOCK) * sizeof(tStation *));
            CHECK_ERRNO;
            city->stations = aux1;
        }
        tStation * aux2 = malloc(sizeof(tStation));
        CHECK_ERRNO;
        city->stations[city->stationCount] = aux2;

        int len = strlen(name) + 1;
        city->stations[city->stationCount]->name = malloc(len);
        if(city->stations[city->stationCount]->name == NULL || errno == ENOMEM) {
            return errno;
        }
        strcpy(city->stations[city->stationCount]->name, name);
        city->stations[city->stationCount]->id = id;
        city->stations[city->stationCount]->destinies = NULL;
        city->stations[city->stationCount]->memberRides = city->stations[city->stationCount]->casualRides = 0;
        city->stations[city->stationCount]->oldestDestinyName = city->stations[city->stationCount]->mostPopName = NULL;
        for(int m = 0; m < MONTHS; m++){
            city->stations[city->stationCount]->monthlyCircularRides[m] = 0;
        }
        city->stationCount++;
        city->order = orderFlag;        // noOrder si las estaciones no se agregaron ordenadas en funcion de sus ids
    }
    return 0;
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

/* Recibe dos punteros a estacion y compara sus id's. Retorna un numero positivo si el primero
es mayor, negativo si es menor y 0 si son iguales */
int compareID(const void * station1, const void * station2) {
    return (*((tStation**)station1))->id - (*((tStation**)station2))->id;
}

/* Busca una estacion segun su id en un vector de estaciones con busqueda binaria. 
Retorna el indice de la estacion si lo encuentra, NOT_FOUND si no */
static
size_t searchId(tStation ** stations, size_t stationCount, size_t id){
    size_t inf, half, sup;
    inf = 0;
    sup = stationCount - 1;
    half = (inf + sup) / 2;

    while(inf <= sup){
        if(stations[half]->id == id){
            return half;
        }
        if(stations[half]->id < id){
            inf = half + 1;
        }
        else{
            sup = half - 1;
        }
        half = (inf + sup) / 2;
    }
    return NOT_FOUND;
}

/* Recorro el arbol binario de destinos, comparando con el id del destino del nuevo viaje. 
Si no esta, lo agrego y me guardo su direccion de memoria en el parametro de salida destinyOut. 
Si lo encuentro, me guardo la direccion de memoria de donde lo encuentro */
tDestiny * checkDestiny(tDestiny * destiny, size_t id, char * endName, tDestiny ** destinyOut){
    if(destiny == NULL){
        tDestiny * new = malloc(sizeof(tDestiny));
        if(new == NULL || errno == ENOMEM)
            return destiny;
        new->id = id;
        new->name = malloc(strlen(endName) + 1);
        if(new->name == NULL || errno == ENOMEM) {
            free(new);
            return destiny;
        }
        strcpy(new->name, endName);
        new->rideCount = 0;
        new->nextRight = NULL;
        new->nextLeft = NULL;
        *destinyOut = new;
        return new;
    }
    if(destiny->id == id){
        *destinyOut = destiny;
        return destiny;
    }
    if(destiny->id < id){
        destiny->nextRight = checkDestiny(destiny->nextRight, id, endName, destinyOut);
        return destiny;
    }
    destiny->nextLeft = checkDestiny(destiny->nextLeft, id, endName, destinyOut);
    return destiny;
}

/* Cambia en station el oldestDestinyName por name y oldestDate por date */
static
void changeOldest(tStation * station, char * name, struct tm date){
    char * aux = realloc(station->oldestDestinyName, strlen(name) + 1);
    if(aux == NULL || errno == ENOMEM) {
        return;
    }
    station->oldestDestinyName = aux;
    strcpy(station->oldestDestinyName, name);
    station->oldest_date = date;
}

/* Cambia en station el mostPopName por name y mostPopRides por rides */
static
void changeMostPop(tStation * station, char * name, size_t rides){
    char * aux = realloc(station->mostPopName, strlen(name) + 1);
    if(aux == NULL || errno == ENOMEM) {
        return;
    }
    station->mostPopName = aux;
    strcpy(station->mostPopName, name);
    station->mostPopRides = rides;
}

/* Usa la formula de Zeller, que usando el dia, el mes y el año que se le pasa, saca el dia de 
la semana en el formato Lunes = 0, Martes = 1, ... Domingo = 6 */
int dateToDayOfWeek(int year, int month, int day) {
    int dayOfWeek;
    dayOfWeek = (day + (13*(month + 1) / 5) + (year % 100) + ((int) (year % 100) / 4) + ((int) (year / 100) / 4) - 2*((int) year / 100));
    return (dayOfWeek + 5) % 7;
}

int addRide(cityADT city, size_t startStationId, struct tm start_date, struct tm end_date, size_t endStationId, int isMember, int startYear, int endYear){
    errno = 0;
    /*Si el vector no esta ordenado por id, lo ordenamos*/
    if(city->order != idOrder) {
        qsort(city->stations, city->stationCount, sizeof(tStation *), compareID);
        city->order = idOrder;
    }

    /* Revisamos que las estaciones de origen y fin existan. Si encontramos ambas, nos guardamos los datos necesarios */
    size_t startIdx, endIdx;
    tStation * station;
    char * endName;
    if((startIdx = searchId(city->stations, city->stationCount, startStationId)) == NOT_FOUND){
        return 0;
    }
    if((endIdx = searchId(city->stations, city->stationCount, endStationId)) == NOT_FOUND){
        return 0;
    }
    station = city->stations[startIdx];
    endName = city->stations[endIdx]->name; 

    /* Si el viaje no es circular, entra */
    if(startStationId != endStationId){
        tDestiny * destiny;
        station->destinies = checkDestiny(station->destinies, endStationId, endName, &destiny);
        CHECK_ERRNO;
        /* Si el viaje esta dentro del rango de años de startYear y endYear, suma en el conteo de viajes del destino */
        if(DATE_RANGE){
            destiny->rideCount++;
        }

        /* Comparo el viaje con el mas viejo registrado (a menos que sea el primero) y si es anterior lo reemplazo */
        if((station->memberRides + station->casualRides) == 0 || dateCompare(start_date, station->oldest_date) < 0){
            changeOldest(station, endName, start_date);
            CHECK_ERRNO;
        }
        /* Comparo la cantidad de viajes del destino actual con el los del mas popular registrado
        (a menos que sea el primero) y si es mayor lo cambio */
        if((station->memberRides + station->casualRides) == 0 || destiny->rideCount > station->mostPopRides){
            changeMostPop(station, endName, destiny->rideCount);
            CHECK_ERRNO;
        }
    }
    /* Si es un viaje circular dentro del rengo de años y con el mismo mes de inicio y se fin, se suma uno a la cantidad
    de viajes circulares de la estacion en ese mes */
    else if(DATE_RANGE && start_date.tm_mon == end_date.tm_mon){
        station->monthlyCircularRides[start_date.tm_mon]++;
    }

    /* Separamos la suma de viajes totales segun miembro o no y sumamos adecuadamente */
    if(isMember)
        station->memberRides++;
    else
        station->casualRides++;
    
    /* Registramos que dia de la semana se inicio y termino el viaje */
    start_date.tm_wday = dateToDayOfWeek(start_date.tm_year, start_date.tm_mon, start_date.tm_mday);
    city->startedRidesPerDay[start_date.tm_wday]++;
    end_date.tm_wday = dateToDayOfWeek(end_date.tm_year, end_date.tm_mon, end_date.tm_mday);
    city->endedRidesPerDay[end_date.tm_wday]++;
    
    return errno;
}



static 
void freeDestinies(tDestiny * destiny){
    if(destiny == NULL)
        return;
    freeDestinies(destiny->nextLeft);
    freeDestinies(destiny->nextRight);
    free(destiny->name);
    free(destiny);
}

void freeCity(cityADT city){
    for(int i = 0; i < city->stationCount; i++){
        free(city->stations[i]->name);
        free(city->stations[i]->oldestDestinyName);
        free(city->stations[i]->mostPopName);
        freeDestinies(city->stations[i]->destinies);
        free(city->stations[i]);
    }
    free(city->stations);
    free(city);
}


void toBegin(cityADT city) {
    city->iter = 0;
}

int hasNext(cityADT city) {
    return city->iter < city->stationCount;
}

tTotalRides nextTotalRides(cityADT city) {
    if(!hasNext(city)) {
        exit(1);
    }
    tTotalRides aux; 
    aux.name = city->stations[city->iter]->name;
    aux.memberRides = city->stations[city->iter]->memberRides;
    aux.casualRides = city->stations[city->iter]->casualRides;
    city->iter++;
    return aux;
}

tOldest nextOldest(cityADT city) {
    if(!hasNext(city)) {
        exit(1);
    }
    tOldest aux; 
    aux.name = city->stations[city->iter]->name;
    aux.destinyName = city->stations[city->iter]->oldestDestinyName;
    aux.date = city->stations[city->iter]->oldest_date;
    city->iter++;
    return aux;
}


/* Funcion de comparacion que retorna un numero negativo si la station1 tiene mas viajes que la station2 y un numero
positivo si tiene menos. Si son iguales en cantidad de viajes, retorna un numero negativo si la primera es alfabeticamente
menor, positivo si es mayor y cero si son iguales*/
static
int compareTotalRides(const void * station1, const void * station2){
    size_t total1, total2;
    if((total1 = (*((tStation**)station1))->casualRides + (*((tStation**)station1))->memberRides) != (total2 = (*((tStation**)station2))->casualRides + (*((tStation**)station2))->memberRides)){
        return total2 - total1;
    }
    return strcasecmp((*((tStation**)station1))->name, (*((tStation**)station2))->name);
}

void orderByRides(cityADT city){
    /* Si ya estaba ordenada segun el criterio, no hace nada */
    if(city->order != ridesOrder){    
        qsort(city->stations, city->stationCount, sizeof(tStation *), compareTotalRides);
        city->order = ridesOrder;
    }
}

/* Funcion de comparacion que retorna un numero negativo si la primera es alfabeticamente menor, positivo si es mayor y 
cero si son iguales */
static
int compareAlph(const void * station1, const void * station2){
    return strcasecmp((*((tStation**)station1))->name, (*((tStation**)station2))->name); 
}

void orderByAlph(cityADT city){
    /* Si ya estaba ordenada segun el criterio, no hace nada */
    if(city->order != alphOrder){
        qsort(city->stations, city->stationCount, sizeof(tStation *), compareAlph);
        city->order = alphOrder;
    }
}

size_t getStartedRides(cityADT city, int index) {
    return city->startedRidesPerDay[index];
}

size_t getEndedRides(cityADT city, int index) {
    return city->endedRidesPerDay[index];
}

tMostPopular nextMostPopular(cityADT city){
    if(!hasNext(city)) {
        exit(1);
    }
    tMostPopular mostPop;
    mostPop.name = city->stations[city->iter]->name;
    mostPop.endName = city->stations[city->iter]->mostPopName;
    mostPop.cantRides = city->stations[city->iter]->mostPopRides;
    city->iter++;
    return mostPop;
}

void getTop3ByMonth(cityADT city, int month, char ** first, char ** second, char ** third){
    if(month < 0 || month > 11)
        return;

    size_t cantTop1 = 0, cantTop2 = 0, cantTop3 = 0;
    char * top1, *top2, *top3;

    /* Recorre las estaciones obteniendo la cantidad de viajes circulares en el mes indicado y las va comparando
    con las registradas para armar el top 3 */
    for (int i = 0; i < city->stationCount; ++i) {
        size_t cantAux = city->stations[i]->monthlyCircularRides[month];
        char * aux = city->stations[i]->name;

        /* Si hay viajes circulares en ese mes, entra */
        if(cantAux > 0){
            if(cantAux > cantTop1 || (cantAux == cantTop1 && strcasecmp(top1, aux) > 0)){
                cantTop3 = cantTop2;
                cantTop2 = cantTop1;
                cantTop1 = cantAux;
                top3 = top2;
                top2 = top1;
                top1 = aux;
            }else if(cantAux > cantTop2 || (cantAux == cantTop2 && strcasecmp(top2, aux) > 0)){
                cantTop3 = cantTop2;
                cantTop2 = cantAux;
                top3 = top2;
                top2 = aux;
            }else if(cantAux >= cantTop3 || (cantAux == cantTop3 && strcasecmp(top3, aux) > 0)){
                cantTop3 = cantAux;
                top3 = aux;
            }
        }

    }
    /* Si alguna de las tres quedo con 0 viajes, retorna Empty en las tres posiciones */
    if(cantTop1 == 0 || cantTop2 == 0 || cantTop3 == 0){
        *first = "Empty";
        *second = "Empty";
        *third = "Empty";
    }
    /* Si no, guarda los nombres del top 3 */
    else{
        *first = top1;
        *second = top2;
        *third = top3;
    }
}