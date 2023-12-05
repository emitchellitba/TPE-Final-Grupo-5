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

    FILE * bikesCsv = fopen(bikes, "r");
    FILE * stationsCsv = fopen(stations, "r");




    fclose(bikesCsv);
    fclose(stationsCsv);
    return 0;
}