
SRCDIR =/home/breadlofe/Desktop/interconnection_networks/Simulator/simplecmesh
# This dirctory should also contain a copy of sim.h.
HOME =/home/breadlofe/Desktop/interconnection_networks/Simulator/simplecmesh
##################### Setup debug or optimized version  ########################
# Uncomment one of the following lines
GDB = -g -pg   # Uncomment to make a debug/gprof profile
#DB = -Wall -O3     # Uncomment to make an optimized version
################################################################################
#################### No user changes needed below this point ###################
################################################################################
INCL =  $(SRCDIR)/incl/simsys.h
all:	$(HOME)/yacsim $(HOME)/yacsim.o \
	$(HOME)/netsim $(HOME)/netsim.o 
clean:
	rm -f netsim netsim.o yacsim yacsim.o
	rm -f $(HOME)/obj/*.o
################################### YACSIM #####################################
yacsim: $(HOME)/yacsim $(HOME)/yacsim.o

$(HOME)/yacsim: $(SRCDIR)/src/yacsim.c
	gcc $(GDB) -DxSIMdir='"$(HOME)"' -o yacsim $(SRCDIR)/src/yacsim.c
$(HOME)/yacsim.o: $(RS6K) \
		$(HOME)/obj/evlst.o \
		$(HOME)/obj/driver.y.o \
		$(HOME)/obj/util.o \
		$(HOME)/obj/baseq.o \
		$(HOME)/obj/userq.o \
		$(HOME)/obj/act.o \
		$(HOME)/obj/stat.o 
	ld -r $(RS6K)\
		$(HOME)/obj/evlst.o \
		$(HOME)/obj/driver.y.o \
		$(HOME)/obj/util.o \
		$(HOME)/obj/baseq.o \
		$(HOME)/obj/userq.o \
		$(HOME)/obj/act.o \
		$(HOME)/obj/stat.o \
		-L/usr/lib \
#		-lm
	mv a.out yacsim.o

################################### NETSIM #####################################

netsim: $(HOME)/netsim $(HOME)/netsim.o

$(HOME)/netsim: $(SRCDIR)/src/netsim.c
	gcc $(GDB) -DxSIMdir='"$(HOME)"' -o netsim $(SRCDIR)/src/netsim.c
$(HOME)/netsim.o: $(RS6K) \
		$(HOME)/obj/evlst.o \
		$(HOME)/obj/driver.n.o \
		$(HOME)/obj/util.o \
		$(HOME)/obj/baseq.o \
		$(HOME)/obj/userq.o \
		$(HOME)/obj/act.o \
		$(HOME)/obj/stat.o \
		$(HOME)/obj/net.o
	ld -r $(RS6K) \
		$(HOME)/obj/evlst.o \
		$(HOME)/obj/driver.n.o \
		$(HOME)/obj/util.o \
		$(HOME)/obj/baseq.o \
		$(HOME)/obj/userq.o \
		$(HOME)/obj/act.o \
		$(HOME)/obj/stat.o \
		$(HOME)/obj/net.o \
		-L/usr/lib \
#		-lm
	mv a.out netsim.o

############################ YACSIM files ####################################

$(HOME)/obj/evlst.o: $(INCL) $(SRCDIR)/src/evlst.c $(SRCDIR)/incl/tr.evlst.h
	cc $(GDB)  -c $(SRCDIR)/src/evlst.c
	mv evlst.o obj/evlst.o

$(HOME)/obj/util.o: $(INCL) $(SRCDIR)/src/util.c 
	cc $(GDB)  -c $(SRCDIR)/src/util.c
	mv util.o obj/util.o

$(HOME)/obj/baseq.o: $(INCL) $(SRCDIR)/src/baseq.c \
	$(SRCDIR)/incl/tr.baseq.h
	cc $(GDB)  -c $(SRCDIR)/src/baseq.c
	mv baseq.o obj/baseq.o

$(HOME)/obj/userq.o: $(INCL) $(SRCDIR)/src/userq.c \
	$(SRCDIR)/incl/tr.userq.h
	cc $(GDB)  -c $(SRCDIR)/src/userq.c
	mv userq.o obj/userq.o

$(HOME)/obj/act.o: $(INCL) $(SRCDIR)/src/act.c $(SRCDIR)/incl/tr.act.h
	cc $(GDB)  -c $(SRCDIR)/src/act.c
	mv act.o obj/act.o

$(HOME)/obj/stat.o: $(INCL) $(SRCDIR)/src/stat.c $(SRCDIR)/incl/tr.stat.h
	cc $(GDB)  -c $(SRCDIR)/src/stat.c
	mv stat.o obj/stat.o

$(HOME)/obj/driver.y.o: $(INCL) $(SRCDIR)/src/driver.c \
	$(SRCDIR)/incl/tr.driver.h
	cc $(GDB) -DYACSIM -c $(SRCDIR)/src/driver.c
	mv driver.o obj/driver.y.o

############################ NETSIM files ####################################

$(HOME)/obj/net.o: $(INCL) $(SRCDIR)/src/net.c \
	$(SRCDIR)/incl/tr.net.h
	cc $(GDB) -c $(SRCDIR)/src/net.c
	mv net.o obj/net.o

$(HOME)/obj/driver.n.o: $(INCL) $(SRCDIR)/src/driver.c \
	$(SRCDIR)/incl/tr.driver.h
	cc $(GDB) -DNETSIM -c $(SRCDIR)/src/driver.c
	mv driver.o obj/driver.n.o



