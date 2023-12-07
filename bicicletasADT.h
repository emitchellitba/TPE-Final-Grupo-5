#ifndef BICICLETASADT_H
#define BICICLETASADT_H

#include <time.h>

#define DAYS_OF_WEEK 7

typedef struct cityCDT * cityADT;


cityADT newCity(void);

// Agrega una estacion con los datos dados, vacia en viajes
// devuelve 1 si agrega, 0 si no, -1 en caso de error en los parametros                               
int addStation(cityADT city, char * name, size_t id);

// Agrega un viaje con los datos dados, dentro de la lista de stations en la estacion de inicio
void addRide(cityADT city, size_t startStationId, struct tm startDate, struct tm endDate, size_t endStationId, int isMember);

void getIdexByRank(cityADT city, int idexVec[]);

char * nameByStationIndex(cityADT city, int idex);

int getStationCount(cityADT city);

void ridesByStationIndex(cityADT city,int idex, size_t rides[2]);

void getIndexByRank(cityADT city, int indexVec[]);

void getIndexByAlph(cityADT city, int indexVec[]);

void getOldest(cityADT city, int index, char * nameStart, char* nameEnd, struct tm * oldestTime);

void freeCity(cityADT city);


#endif