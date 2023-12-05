#include <stdio.h>

#define ERROR -1

int main(int argc, char * argv[]){

    int startYear = 0, endYear = 0;
    char* bikes, stations;

    if(argc < 2 || argc > 4) {
        printf("Cantidad invalida de parametros.\n");
        return ERROR;
    } else {
        bikes = argv[1];
        stations = argv[2];
        if(argc > 2) {
            startYear = argv[3];
            if(argc == 4) {
                endYear = argv[4];
            }
        }
    }

   cityADT nyc = newCity();

    FILE * bikesCsv = fopen(bikes, "r");
    FILE * stationsCsv = fopen(stations, "r");

    char * lineaActual;
    unsigned long startStationId, endStationId;
    struct tm startDate, endDate;
    char * memberState;
    
    while(fscanf(bikesCsv, "%d-%d-%d %d:%d:%d;%ld;%d-%d-%d %d:%d:%d;%ld;%*s;%s", startDate.tm_year, startDate.tm_mon, startDate.tm_mday, 
    startDate.tm_hour, startDate.tm_min, startDate.tm_sec, startStationId, endDate.tm_year, endDate.tm_mon, endDate.tm_mday, endDate.tm_hour,
    endDate.tm_min, endDate.tm_sec, endStationId, memberState) != EOF){                     //TAMBIEN PUEDE SER DISTINTO DE CERO
        addRide(nyc, startStationId, startDate, endDate, endStationId, (char) strcmp("member", memberState) = 0);
    }

    char * name;
    unsigned long stationId;

    while(fscanf(bikesCsv, "%s;%*f;%*f;%ld", name, stationId) != EOF){                     //TAMBIEN PUEDE SER DISTINTO DE CERO
        addStation(nyc, name, stationId);
    }

    fclose(bikesCsv);
    fclose(stationsCsv);
    return 0;
}
