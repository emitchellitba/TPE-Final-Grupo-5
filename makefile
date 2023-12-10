COMPILER=gcc
CFLAGS=-pedantic -std=c99 -Wall -fsanitize=address -g
OUTPUT_FILE_NYC=bikeSharingNYC
FRONT_NYC=bikeSharingNYC.c
OUTPUT_FILE_MON=bikeSharingMON
FRONT_MON=bikeSharingMON.c


all: makeExecutables clean

makeExecutables: back makeExecutableNYC makeExecutableMON

makeExecutableNYC:
	$(COMPILER) -o $(OUTPUT_FILE_NYC) $(FRONT_NYC) bicicletasADT.o htmlTable.o $(CFLAGS)

makeExecutableMON:
	$(COMPILER) -o $(OUTPUT_FILE_MON) $(FRONT_MON) bicicletasADT.o htmlTable.o $(CFLAGS)

back: bicicletasADT.o htmlTable.o

bicicletasADT.o: bicicletasADT.c
	$(COMPILER) -c bicicletasADT.c

htmlTable.o: cTable/htmlTable.c 
	$(COMPILER) -c cTable/htmlTable.c

clean:
	rm -f *.o

cleanAll:
	rm -r $(OUTPUT_FILE_NYC) $(OUTPUT_FILE_MON)