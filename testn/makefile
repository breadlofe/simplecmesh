SRC = src
FLAGS = -g -pg # -g is for GDB, -pg is for gprof

all: cmesh hypercube flat_butterfly sanity

cmesh: $(SRC)/cmesh.c ../netsim.o
	gcc $(FLAGS) -o cmesh $(SRC)/cmesh.c ../netsim.o -lm
	rm -f cmesh.o

hypercube: $(SRC)/hypercube.c ../netsim.o
	gcc $(FLAGS) -o hypercube $(SRC)/hypercube.c ../netsim.o -lm
	rm -f hypercube.o

flat_butterfly: $(SRC)/flat_butterfly.c ../netsim.o
	gcc $(FLAGS) -o flat_butterfly $(SRC)/flat_butterfly.c ../netsim.o -lm
	rm -f flat_butterfly.o

sanity: $(SRC)/sanity.c ../netsim.o
	gcc $(FLAGS) -o sanity $(SRC)/sanity.c ../netsim.o -lm
	rm -f sanity.o

clean:
	rm -f *out* *core* *dump* *mesh*
