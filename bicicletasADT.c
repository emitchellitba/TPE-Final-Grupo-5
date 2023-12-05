#include "bicicletasADT.h"

#define DAYS_OF_WEEK 7


typedef struct ride{
    tDate startDate;
    tDate endDate;
    char isMember;                      // me interesa guardar el isMember en cada una?(solo lo uso al registrar uno nuevo)
                                        // Guardo el bikeType aunque no lo use?????? (no lo guarde por ahora)
    struct ride * next;
} tRide;


typedef struct route{
    long endStationId;
    tRide * rides;
    long rideCount;
    struct route * next;                                                  
} tRoute;

typedef struct station{
    char * name;
    long id;
    double latitude;                   // Guardo la lat y la long aunque no las use??? (las guarde por ahora)
    double longitude;
    tRoute * routes;
    long memberRides;
    long casualRides;
    struct station * next;
} tStation;

typedef struct cityCDT{
    tStation * stations;
    long ridesPerDay[DAYS_OF_WEEK];
    long stationCount;
} cityCDT;




cityADT newCity(void){
    cityADT new = calloc(1, sizeof(cityCDT));       //FALTARIA CHEQUEAR LO DE NULL        

    return new;
}

static
tStation * addStationRec(tStation * station, char * name, long id, double latitude, double longitude, int * flag){
        	                                                //FALTARIA CHEQUEAR PARAMETROS (invalido => return -1)
    if(station == NULL || station->id > id){
        tStation * new = malloc(sizeof(tStation));         //FALTARIA CHEQUEAR NULL
        new->name = name;
        new->id = id;
        new->latitude = latitude;
        new->longitude = longitude;
        new->routes = NULL;
        new->memberRides = new->casualRides = 0;
        new->next = station;
        *flag = 1;
        return new;
    }
    if(station->id < id)
        station->next = addStationRec(station->next, name, id, latitude, longitude, flag);
    return station;
}

int addStation(cityADT city, char * name, long id, double latitude, double longitude){
    int flag = 0;
    city->stations = addStationRec(city->stations, name, id, latitude, longitude, &flag);
    city->stationCount += flag;
    return flag;
}

static
tRoute * addRideRec(tRoute * route, tDate startDate, tDate endDate, long endStationId, char isMember, int * flag){
    if(route->endStationId = endStationId){
        
    }   
}

int addRide(cityADT city, long startStationId, tDate startDate, tDate endDate, long endStationId, char isMember){
                                                    //FALTARIA CHEQUEAR PARAMETROS (invalido => return -1)
    tStation * aux = city->stations;
    tStation * station;
    int foundStart = 0;
    int foundEnd = 0;

    // queda feo con tantos && pero si lo separo recorro dos veces al pedo. como hacerlo mas lindo?

    while(aux != NULL && aux->id < startStationId && aux->id < endStationId && (!foundStart || !foundEnd)){
        if(aux->id == startStationId){
            station = aux;
            foundStart = 1;
        }
        if(aux->id == endStationId)
            foundEnd = 1;

        aux = aux->next;
    }
    if((!foundStart || !foundEnd))
        return -1;                      // el start id o el end id no existen => error!!

    int flag = 0;
    station->routes = addRideRec(station->routes, startDate, endDate, endStationId, isMember, &flag);

    
    tRoute auxRoute = station->routes;
    
    if(auxRoute->endStationId = endStationId){

    }

        

}

typedef struct ride{
    tDate startDate;
    tDate endDate;
    char isMember;                      // me interesa guardar el isMember en cada una?(solo lo uso al registrar uno nuevo)
                                        // Guardo el bikeType aunque no lo use?????? (no lo guarde por ahora)
    struct ride * next;
} tRide;


typedef struct route{
    long endStationId;
    tRide * rides;
    long rideCount;
    struct route * next;                                                  
} tRoute;




