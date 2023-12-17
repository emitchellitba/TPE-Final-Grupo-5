#ifndef BICICLETASADT_H
#define BICICLETASADT_H

#include <time.h>

#define DAYS_OF_WEEK 7
#define MONTHS 12

/* Estructuras para obtener la informacion necesaria al iterar */
typedef struct{
    char * name;
    size_t memberRides, casualRides;
} tTotalRides;

typedef struct{
    char * name;
    char * destinyName;
    struct tm date;
} tOldest;

typedef struct{
    char * name;
    char * endName;
    size_t cantRides;
} tMostPopular;

typedef struct cityCDT * cityADT;

/* Reserva el espacio para una nueva ciudad, sin estaciones ni viajes */
cityADT newCity(void);

/* Agrega una estacion con los datos dados, vacia.  */                         
int addStation(cityADT city, char * name, size_t id);

/* Agrega un viaje desde la estacion de id = startStationId a la estacion de id = endStationId con los datos dados. 
Si las estaciones de incio o fin no existen, se ignora el viaje. */
int addRide(cityADT city, size_t startStationId, struct tm startDate, struct tm endDate, size_t endStationId, int isMember, int startYear, int endYear);

/* Se libera la memoria reservada */
void freeCity(cityADT city);

void toBegin(cityADT city);

int hasNext(cityADT city);

tTotalRides nextTotalRides(cityADT city);

tOldest nextOldest(cityADT city);

void orderByRides(cityADT city);

void orderByAlph(cityADT city);

/* Retorna la cantidad de viajes que se inciaron en el dia indicado por dayOfWeek */
size_t getStartedRides(cityADT city, int dayOfWeek);

/* Retorna la cantidad de viajes que se terminaron en el dia indicado por dayOfWeek */
size_t getEndedRides(cityADT city, int dayOfWeek);

tMostPopular nextMostPopular(cityADT city);

/* Retorna en variables de salida los nombres del top 3 estaciones con mas viajes circulares
 * en caso de que no halla 3 estaciones con mas viajes circulares, retorna "empty" en todos los campos*/
void getTop3ByMonth(cityADT city, int month, char ** first, char ** second, char ** third);


#endif