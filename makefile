COMPILER=gcc
CFLAGS=-pedantic -std=c99 -Wall -fsanitize=address -g
OUTPUT_FILE_NYC=bikeSharingNYC
OUTPUT_FILE_MON=bikeSharingMON
FRONT=bikeSharing.c


all: makeExecutables clean

makeExecutables: back makeExecutableNYC makeExecutableMON

makeExecutableNYC:
	$(COMPILER) -o $(OUTPUT_FILE_NYC) $(FRONT) bicicletasADT.o htmlTable.o $(CFLAGS) -DNYC

makeExecutableMON:
	$(COMPILER) -o $(OUTPUT_FILE_MON) $(FRONT) bicicletasADT.o htmlTable.o $(CFLAGS) -DMON

back: bicicletasADT.o htmlTable.o

bicicletasADT.o: bicicletasADT.c
	$(COMPILER) -c bicicletasADT.c

htmlTable.o: cTable/htmlTable.c 
	$(COMPILER) -c cTable/htmlTable.c

clean:
	rm -f *.o

cleanAll:
	rm -r $(OUTPUT_FILE_NYC) $(OUTPUT_FILE_MON)