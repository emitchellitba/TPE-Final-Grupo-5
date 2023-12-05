#ifndef BICICLETASADT_H
#define BICICLETASADT_H

typedef struct cityCDT * cityADT;

typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;                      // vale ponerlo en el .h ????????
} tDate;                            // nose si hay una mejor forma de hacer esto



cityADT newCity();

// Agrega una estacion con los datos dados, vacia en viajes
// devuelve 1 si agrega, 0 si no, -1 en caso de error en los parametros                               
int addStation(cityADT city, char * name, size_t id);

// Agrega un viaje con los datos dados, dentro de la lista de stations en la estacion de inicio
// devuelve 1 si agrega, 0 si no, -1 en caso de error en los parametros  
int addRide(cityADT city, size_t startStationId, tDate startDate, tDate endDate, size_t endStationId, char isMember);



#endif


