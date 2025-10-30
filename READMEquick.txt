Taken from the original README still in this directory

	"To install YACSIM, NETSIM, and DBSIM, edit the first part of the makefile in the directory 
	simdir as indicated in the file itself.  Then type "make yacsim" to make YACSIM, 
	"make netsim" to make NETSIM, or "make dbsim" to make DBSIM.  Type "make all" or just "make"
	to make all three.  Since NETSIM is an extension of YACSIM and contains everything in 
	YACSIM, it is not necessary to make both.   You could just make NETSIM and use it for all your
	simulations.  The only reason to make and use YACSIM without the NETSIM extensions is that the
	object files made with YACSIM are a little smaller than those made with NETSIM."


To run Netsim:
1) Set your home and src variables in the upper level directory makefile
2) Type make
3) If yacsim and netsim both compile go to the testn directory
   - "dont know how to make target" error indicates the directories are not set right in the makefile
4) Type make
5) To run use the executable with these parameters:
   - ./cmesh 4 load10s(value 1-9) load100s(value1-9) cycles traffic(0-8)
   - Example: ./cmesh 4 1 5 10000 0
     - load of 0.15 for 10000 cycles and traffic pattern 0 which is uniform random
     - more information on these can be derrived from the topology file in /testn/src UserMain()