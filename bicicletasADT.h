#ifndef BICICLETASADT_H
#define BICICLETASADT_H

#include <time.h>

#define DAYS_OF_WEEK 7

typedef struct cityCDT * cityADT;


cityADT newCity(void);

/* Agrega una estacion con los datos dados, vacia en viajes. Devuelve 1 si agrega, 0 si no */                         
int addStation(cityADT city, char * name, size_t id);

/* Agrega un viaje con los datos dados. Si las estaciones de incio o fin no existen, se ignora el viaje */
int addRide(cityADT city, size_t startStationId, struct tm startDate, struct tm endDate, size_t endStationId, int isMember);

/* Se libera la memoria reservada */
void freeCity(cityADT city);

int getIndexByRank(cityADT city, int idexVec[]);

char * nameByStationIndex(cityADT city, int idex);

int getStationCount(cityADT city);

void ridesByStationIndex(cityADT city,int idex, size_t rides[2]);

int getIndexByRank(cityADT city, int indexVec[]);

int getIndexByAlph(cityADT city, int indexVec[]);

void getOldest(cityADT city, int index, char ** nameStart, char ** nameEnd, struct tm * oldestTime);

size_t getStartedRides(cityADT city, int index);

size_t getEndedRides(cityADT city, int index);

void getMostPopular(cityADT city, size_t stationIndex, size_t * ridesOut, char ** endName, int startYear, int endYear);



#endif