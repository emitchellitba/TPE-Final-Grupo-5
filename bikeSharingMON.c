#include <stdio.h>
#include "bicicletasADT.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include "bicicletasADT.h"
#define ERROR -1

int checkParams(char* bikes, char*stations, int startYear, int endYear){

    if(endYear < 0 || startYear < 0) return 0;
    if(startYear != 0 && endYear != 0){
        if(endYear < startYear) return 0;
    }
    if(strcmp(bikes, "bikes.csv") != 0) return 0;
    if(strcmp(stations,"stations.csv") != 0) return 0;
    return 1;
}


int main(int argc, char * argv[]){

    int startYear = 0, endYear = 0;
    char* bikes, stations;

    bikes = argv[1];
    stations = argv[2];

    if(argc > 2){
        startYear = atoi(argv[3]);
        if(argc == 4) endYear = atoi(arg[4]);
    }

    int a;
    if((a = checkParams(bikes, stations, startYear, endYear)) == 0){
        printf("Error! Los parametros son incorrectos \n");
        return ERROR;
    }

    FILE * bikesCsv = fopen(bikes, "r");
    FILE * stationsCsv = fopen(stations, "r");





    fclose(bikesCsv);
    fclose(stationsCsv);
    return 0;
}