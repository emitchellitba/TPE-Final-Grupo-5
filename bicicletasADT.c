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

enum orderedBy {noOrder = 0, idOrder, ridesOrder, alphOrder};

/* Nuestro TAD consiste en un vector dinamico donde se almacenan las estaciones (en orden de agregado). Dentro de cada una se almacena
información útil para los queries y un vector dinámico de los destinos de los viajes iniciado en esa estación (tambien en orden de agregado).
Dentro de los vectores de destinos, se almacena su nombre y un puntero al primer elemento de una lista de los viajes realizados entre esas 
dos estaciones, en orden cronológico. Decidimos separar por destino ya que nos facilitaba buscar el destino mas popular (query 4). 
De los vectores dinámicos, nos vamos guardando su dimension para poder recorrerlos. 
Como algunos queries nos solicitaban un orden y otros otro (ej. orden alfabetico o cantidad de viajes), decidimos guardarlos por orden
de agregado y hacer las funciones que nos devuelvan los indices ordenados segun el criterio */

typedef struct ride{
    struct tm start_date, end_date;
    struct ride * nextLeft;
    struct ride * nextRight;
} tRide;

typedef struct destiny {
    char * name;
    size_t id;
    tRide * rides;
    struct destiny * nextLeft;
    struct destiny * nextRight;
} tDestiny;

typedef struct station{
    struct tm oldest_date;
    char * name;
    char * oldestDestinyName;
    tRide * circularRides;
    tDestiny * destinies;
    size_t id, memberRides, casualRides;
} tStation;

typedef struct cityCDT{
    tStation ** stations;
    size_t stationCount, startedRidesPerDay[DAYS_OF_WEEK], endedRidesPerDay[DAYS_OF_WEEK];
    size_t iter;
    int order;
} cityCDT;



cityADT newCity(void){
    return calloc(1, sizeof(cityCDT)); //si retorna NULL, es porque dio error el calloc.
}


int addStation(cityADT city, char * name, size_t id){
    errno = 0;
    /* Primero nos fijamos que no exista, recorriendo el vector */
    bool orderFlag = idOrder;
    bool found = 0;
    size_t i;
    for(i = 0; i < city->stationCount && !found; i++){
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
        city->stations[city->stationCount]->circularRides = NULL;
        city->stations[city->stationCount]->memberRides = city->stations[i]->casualRides = 0;
        city->stations[city->stationCount]->oldestDestinyName = NULL;
        city->stationCount++;
        //si se crea una nueva estacion, se apaga el flag de ordered
        //a discutir si sumamos comparaciones para ver que el id sea menor que la ultima estacion del vector
        city->order = orderFlag;
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

/* Si no esta, lo agrego y me guardo esa direc; si lo encuentro, me guardo la direc de donde lo encuentro */
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
        new->rides = NULL;
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

/* Usa la formula de Zeller, que usando el dia, el mes y el año que se le pasa, saca el dia de 
la semana en el formato Lunes = 0, Martes = 1, ... Domingo = 6 */
int dateToDayOfWeek(int year, int month, int day) {
    int dayOfWeek;
    dayOfWeek = (day + (13*(month + 1) / 5) + (year % 100) + ((int) (year % 100) / 4) + ((int) (year / 100) / 4) - 2*((int) year / 100));
    return (dayOfWeek + 5) % 7;
}

/* Agrega viaje a la lista en orden cronológico. Si hay dos viajes que salgan al mismo momento
se guardan ambos, en orden de agregado */
static
tRide * addRideRec(tRide * ride, struct tm start_date, struct tm end_date){
    if(ride == NULL){
        tRide * new = malloc(sizeof(tRide));          
        // Si no hay espacio, no se agrega
        if(new == NULL || errno == ENOMEM)
            return ride; 
        new->start_date = start_date;
        new->end_date = end_date;
        new->nextLeft = new->nextRight = NULL;
        return new;
    }
    if(dateCompare(start_date, ride->start_date) <= 0){
        ride->nextLeft = addRideRec(ride->nextLeft, start_date, end_date);
        return ride;
    }
    ride->nextRight = addRideRec(ride->nextRight, start_date, end_date);
    return ride;
}

int addRide(cityADT city, size_t startStationId, struct tm start_date, struct tm end_date, size_t endStationId, int isMember){
    errno = 0;
    /*Si el vector no está ordenado, lo ordenamos*/
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

    if(startStationId != endStationId){
        tDestiny * destiny;
        station->destinies = checkDestiny(station->destinies, endStationId, endName, &destiny);
        CHECK_ERRNO;
        destiny->rides = addRideRec(destiny->rides, start_date, end_date);
        CHECK_ERRNO;

        /* Nos guardamos el viaje mas viejo (no circular) para el query 3
        Comparo el viaje con el mas viejo registrado (a menos que sea el primero) y si es anterior lo reemplazo */
        if((station->memberRides + station->casualRides) == 0 || dateCompare(start_date, station->oldest_date) < 0){
            changeOldest(station, endName, start_date);
            CHECK_ERRNO;
        }
    }else{
        station->circularRides = addRideRec(station->circularRides, start_date, end_date);
        CHECK_ERRNO;
    }

    /* Separamos la suma de viajes totales segun miembro o no para el query 1 y sumamos adecuadamente */
    if(isMember)
        station->memberRides++;
    else
        station->casualRides++;
    
    /* Para el query 2, registramos que dia de la semana se inicio y termino el viaje */
    start_date.tm_wday = dateToDayOfWeek(start_date.tm_year, start_date.tm_mon, start_date.tm_mday);
    city->startedRidesPerDay[start_date.tm_wday]++;
    end_date.tm_wday = dateToDayOfWeek(end_date.tm_year, end_date.tm_mon, end_date.tm_mday);
    city->endedRidesPerDay[end_date.tm_wday]++;
    
    return errno;
}


static
void freeRides(tRide * ride){
    if(ride == NULL)
        return;
    freeRides(ride->nextLeft);
    freeRides(ride->nextRight);
    free(ride);
}

static 
void freeDestinies(tDestiny * destiny){
    if(destiny == NULL)
        return;
    freeDestinies(destiny->nextLeft);
    freeDestinies(destiny->nextRight);
    freeRides(destiny->rides);
    free(destiny->name);
    free(destiny);
}

void freeCity(cityADT city){
    for(int i = 0; i < city->stationCount; i++){
        free(city->stations[i]->name);
        free(city->stations[i]->oldestDestinyName);
        freeDestinies(city->stations[i]->destinies);
        freeRides(city->stations[i]->circularRides);
        free(city->stations[i]);
    }
    free(city->stations);
    free(city);
}

// Funciones de iteracion
void toBegin(cityADT city) {
    city->iter = 0;
}

int hasNext(cityADT city) {
    return city->iter < city->stationCount;
}

tData next(cityADT city) {
    if(!hasNext(city)) {
        exit(1);
    }
    tData aux; 
    aux.name = city->stations[city->iter]->name;
    aux.memberRides = city->stations[city->iter]->memberRides;
    aux.casualRides = city->stations[city->iter]->casualRides;
    aux.oldestDestinyName = city->stations[city->iter]->oldestDestinyName;
    aux.oldest_date = city->stations[city->iter]->oldest_date;
    city->iter++;
    return aux;
}


static
int compareTotalRides(const void * station1, const void * station2){
    size_t total1, total2;
    if((total1 = (*((tStation**)station1))->casualRides + (*((tStation**)station1))->memberRides) != (total2 = (*((tStation**)station2))->casualRides + (*((tStation**)station2))->memberRides)){
        return total2 - total1;
    }
    return strcasecmp((*((tStation**)station1))->name, (*((tStation**)station2))->name);
}

void orderByRides(cityADT city){
    if(city->order != ridesOrder){    
        qsort(city->stations, city->stationCount, sizeof(tStation *), compareTotalRides);
        city->order = ridesOrder;
    }
}

static
int compareAlph(const void * station1, const void * station2){
    return strcasecmp((*((tStation**)station1))->name, (*((tStation**)station2))->name); 
}

void orderByAlph(cityADT city){
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


/*Recibe una lista con los viajes entre una estacion y un destino y retorna la cantidad de viajes entre startYear y endYear*/
static
size_t getRidesBetween(tRide * ride, size_t startYear, size_t endYear){ 
    if(ride == NULL){
        return 0;
    }
    if((startYear == 0 || ride->start_date.tm_year >= startYear) && (endYear == 0 || ride->start_date.tm_year <= endYear)){
        return getRidesBetween(ride->nextLeft, startYear, endYear) + getRidesBetween(ride->nextRight, startYear, endYear) +
            ((startYear == 0 || ride->end_date.tm_year >= startYear) && (endYear == 0 || ride->end_date.tm_year <= endYear));
    }
    if(ride->start_date.tm_year > endYear){
        return getRidesBetween(ride->nextLeft, startYear, endYear);
    }
    return getRidesBetween(ride->nextRight, startYear, endYear);
}


void getMostPopularRec(tDestiny * destiny, int startYear, int endYear, char ** maxName, size_t * maxRides) {
    if(destiny == NULL)
        return;
    getMostPopularRec(destiny->nextLeft, startYear, endYear, maxName, maxRides);
    getMostPopularRec(destiny->nextRight, startYear, endYear, maxName, maxRides);
    size_t rides =  getRidesBetween(destiny->rides, startYear, endYear);
    if(rides > *maxRides || (rides == *maxRides && strcasecmp(*maxName, destiny->name) > 0)) {
        *maxRides = rides;
        *maxName = destiny->name;
    }
}

/*Se guardan en las variables de salida el nombre y cantidad de viajes del destino más popular*/
static
tMostPopular getMostPopular(tStation * station, int startYear, int endYear){
    tMostPopular mostPopular;
    mostPopular.name = station->name;
    if(station->destinies != NULL){
        /*Se setean las variables con los valores del primer destino*/
        size_t maxRides = getRidesBetween(station->destinies->rides, startYear, endYear);
        char * maxName = station->destinies->name;

        /*Se recorren todos los destinos y se compara con el máximo registrado*/
        getMostPopularRec(station->destinies, startYear, endYear, &maxName, &maxRides);
      
        mostPopular.cantRides = maxRides;
        mostPopular.endName = maxName;
    }else{
         /*Si no llega a haber destinos se inicializan en 0 ambas variables*/
        mostPopular.cantRides = 0;
        mostPopular.endName = NULL;
    }
    return mostPopular;
}

tMostPopular nextMostPopular(cityADT city, int startYear, int endYear){
    if(!hasNext(city)) {
        exit(1);
    }
    tMostPopular aux = getMostPopular(city->stations[city->iter], startYear, endYear);
    city->iter++;
    return aux;
}


static 
int getCircularRidesBetween(tRide * ride, int month, int startYear, int endYear){
     if(ride == NULL){
        return 0;
    }
    if((startYear == 0 || ride->start_date.tm_year >= startYear) && (endYear == 0 || ride->start_date.tm_year <= endYear)){
        return getCircularRidesBetween(ride->nextLeft, month, startYear, endYear) + getCircularRidesBetween(ride->nextRight, month, startYear, endYear) +
            ((startYear == 0 || ride->end_date.tm_year >= startYear) && (endYear == 0 || ride->end_date.tm_year <= endYear) &&
            ride->start_date.tm_mon == ride->end_date.tm_mon && ride->start_date.tm_mon == month);
        // se suma 1 si el año esta entre endYear y StartYear y si el mes es igual al mes de inicio y final.
    }
    if(ride->start_date.tm_year > endYear){
        return getCircularRidesBetween(ride->nextLeft, month, startYear, endYear);
    }
    return getCircularRidesBetween(ride->nextRight, month, startYear, endYear);
}

void getTop3ByMonth(cityADT city, int month, char ** first, char ** second, char ** third, int startYear, int endYear){
    if(month < 0 || month > 11)
        return;

    int cantTop1 = 0, cantTop2 = 0, cantTop3 = 0;
    char * top1, *top2, *top3;

    for (int i = 0; i < city->stationCount; ++i) {
        int cantAux = getCircularRidesBetween(city->stations[i]->circularRides, month, startYear, endYear);
        char * aux = city->stations[i]->name;

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

    if(cantTop1 == 0 || cantTop2 == 0 || cantTop3 == 0){
        *first = "Empty";
        *second = "Empty";
        *third = "Empty";
    }else{
        *first = top1;
        *second = top2;
        *third = top3;
    }
}