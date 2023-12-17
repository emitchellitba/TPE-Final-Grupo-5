# TPE-FINAL-GRUPO-5 - PROGRAMACION IMPERATIVA
    - 2do cuatrimestre 2023

## MIEMBROS
    - EMILIO MITCHELL   64590
    - TOMAS VARETTONI   64200
    - BRUNO TACCONNE    64888
    - ISABEL CONDE      65779

## CREACION DEL EJECUTABLE (uso de makefile)
    Para hacer los ejecutables se debe llamar a la funcion `make`, que generará ambos ejecutables al mismo tiempo (bikeSharingNYC
y bikeSharingMON). 

## EJECUCION
    Luego, para la ejecucion de los mismos se deben pasar los archivos de bicicletas y de estaciones, en ese orden. Despues de estos, se puede elegir pasar un limite de años entre los cuales se analizaran las queries 4 y 5 (puede ser ninguno, solo el de inicio o el de inicio y fin). 

Ejemplo de ejecucion para MONTREAL:
./bikeSharingMON bikesMON.csv stationsMON.csv 2000

## BORRAR EJECUTABLES
    Se debe ejecutar: `make cleanAll`