#ifndef BIKESHARINGADT_H
#define BIKESHARINGADT_H

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

/* Reserva el espacio para una nueva ciudad, sin estaciones ni viajes. Retorna NULL si no se pudo alocar memoria */
cityADT newCity(void);

/* Agrega una estacion con los datos dados, vacia. Retorna ENOMEM si no se pudo alocar memoria  */                         
int addStation(cityADT city, char * name, size_t id);

/* Agrega un viaje desde la estacion de id = startStationId a la estacion de id = endStationId con los datos dados. 
Si las estaciones de inicio o fin no existen, se ignora el viaje. Retorna ENOMEM si no se pudo alocar memoria */
int addRide(cityADT city, size_t startStationId, struct tm startDate, struct tm endDate, size_t endStationId, int isMember, int startYear, int endYear);

/* Se libera la memoria reservada */
void freeCity(cityADT city);

// FUNCIONES DE ITERACION
/* Setea el iterador en el primer elemento de stations */
void toBegin(cityADT city);

/* Retorna 1 si aun quedan estaciones por recorrer, 0 si no */
int hasNext(cityADT city);

/* Retorna un struct tTotalRides que contiene el nombre de la estacion actual, la cantidad de memberRides y de casualRides, 
 y avanza el iterador */
tTotalRides nextTotalRides(cityADT city);

/* Retorna un struct tOldest que contiene el nombre de la estacion actual, la fecha del viaje mas viejo y el nombre de su destino, 
y avanza el iterador */
tOldest nextOldest(cityADT city);

/* Retorna un struct tMostPopular con el nombre de la estacion actual, el nombre del destino mas popular y su cantidad de viajes,
* y avanza el iterador */
tMostPopular nextMostPopular(cityADT city);
//

/* Ordena las estaciones segun su cantidad total de viajes, descendentemente con desempate alfabetico */
void orderByRides(cityADT city);

/* Ordena las estaciones alfabeticamente segun sus nombres */
void orderByAlph(cityADT city);

/* Retorna la cantidad de viajes que se inciaron en el dia indicado por dayOfWeek */
size_t getStartedRides(cityADT city, int dayOfWeek);

/* Retorna la cantidad de viajes que se terminaron en el dia indicado por dayOfWeek */
size_t getEndedRides(cityADT city, int dayOfWeek);

/* Retorna en parametros de salida los nombres del top 3 estaciones con mas viajes circulares en el mes month;
* en caso de que no halla 3 estaciones con viajes circulares, guarda "empty" en todos los campos */
void getTop3ByMonth(cityADT city, int month, char ** first, char ** second, char ** third);


#endif