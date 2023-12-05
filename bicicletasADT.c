#include "bicicletasADT.h"

#define DAYS_OF_WEEK 7

typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
} tDate;                            // nose si hay una mejor forma de hacer esto

typedef struct ride{
    tDate startDate;
    tDate endDate;
    size_t endStationId;
    char isMember;                  // Guardo el bikeType aunque no lo use?????? (no lo guarde por ahora)
} tRide;

typedef struct station{
    char * name;
    size_t id;
    double latitude;
    double longitude;
    tRide * rides;
    size_t memberRides;
    size_t casualRides;
    struct station * next;
} tStation;

typedef struct cityCDT{
    tStation * stations;
    size_t ridesPerDay[DAYS_OF_WEEK];
    size_t stationCount;
} cityCDT;




cityADT newCity(void){
    return calloc(1, sizeof(cityCDT));
}



void addStation(cityADT city, char * name, size_t id, double latitude, double longitude){
    int flag = 0;
    addStationRec(city->stations, name, id, latitude, longitude, &flag);
    city->stationCount += flag;
}


