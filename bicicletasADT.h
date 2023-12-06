#ifndef BICICLETASADT_H
#define BICICLETASADT_H

#include <time.h>

typedef struct cityCDT * cityADT;


cityADT newCity(void);

// Agrega una estacion con los datos dados, vacia en viajes
// devuelve 1 si agrega, 0 si no, -1 en caso de error en los parametros                               
int addStation(cityADT city, char * name, unsigned long id);

// Agrega un viaje con los datos dados, dentro de la lista de stations en la estacion de inicio
void addRide(cityADT city, unsigned long startStationId, struct tm startDate, struct tm endDate, unsigned long endStationId, int isMember);

void getIdexByRank(cityADT city, int idexVec[]);

char * nameByStationIndex(cityADT city, int idex);

int getStationCount(cityADT city);

void ridesByStationIndex(cityADT city,int idex, unsigned long rides[2]);

void freeCity(cityADT city);


#endif