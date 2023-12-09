#ifndef BICICLETASADT_H
#define BICICLETASADT_H

#include <time.h>

#define DAYS_OF_WEEK 7

typedef struct cityCDT * cityADT;

/* Reserva el espacio para una nueva ciudad, sin estaciones ni viajes */
cityADT newCity(void);

/* Agrega una estacion con los datos dados, vacia en viajes. Devuelve 1 si agrega, 0 si no */                         
int addStation(cityADT city, char * name, size_t id);

/*Optimiza el espacio utilizado por el vector de estaciones*/
void accomodateStation(cityADT city);

/* Agrega un viaje con los datos dados. Si las estaciones de incio o fin no existen, se ignora el viaje */
int addRide(cityADT city, size_t startStationId, struct tm startDate, struct tm endDate, size_t endStationId, int isMember);

/*Optimiza el espacio utilizado por los vectores de destinos*/
void accomodateDestiny(cityADT city);

/* Se libera la memoria reservada */
void freeCity(cityADT city);

/* Deja en indexVec los indices de las estaciones ordenados descendentemente segun cantidad de viajes (con desempate alfabetico) */
int getIndexByRank(cityADT city, int indexVec[]);

/* Retorna el nombre de la estacion con indice index */
char * nameByStationIndex(cityADT city, int index);

/* Retorna la cantidad de estaciones */
int getStationCount(cityADT city);

/* Deja en rides[0] la cantidad de memberRides y en rides[1] la cantidad de casualRides de la estacion de indice index */
void ridesByStationIndex(cityADT city,int index, size_t rides[2]);

/* Deja en indexVec los indices de las estaciones ordenados alfabeticamente */
int getIndexByAlph(cityADT city, int indexVec[]);

/* Retorna en variables de salida el nombre de la estacion de indice index, la fecha el viaje mas antiguo de la misma
y el nombre del destino de ese viaje */
void getOldest(cityADT city, int index, char ** nameStart, char ** nameEnd, struct tm * oldestTime);

/* Retorna la cantidad de viajes que se inciaron en el dia indicado por dayOfWeek */
size_t getStartedRides(cityADT city, int dayOfWeek);

/* Retorna la cantidad de viajes que se terminaron en el dia indicado por dayOfWeek */
size_t getEndedRides(cityADT city, int dayOfWeek);

/* Retorna en variables de salida el nombre y la cantidad de viajes del destino con mas viajes saliendo desde la estacion
de indice stationIndex */
void getMostPopular(cityADT city, size_t stationIndex, size_t * ridesOut, char ** endName, int startYear, int endYear);



#endif