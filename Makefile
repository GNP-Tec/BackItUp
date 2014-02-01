SRC=src/main.cpp src/BackItUp.cpp src/config.cpp src/regular.cpp src/compressed.cpp

V_MAJ=0
V_MIN=2

CFLAGS=`xml2-config --cflags` -Wall -Wno-delete-non-virtual-dtor -DV_MAJ=$(V_MAJ) -DV_MIN=$(V_MIN)
LDFLAGS=`xml2-config --libs` -llogger -larchive 

all:
	g++ -o backup $(CFLAGS) $(LDFLAGS) $(SRC) -g -DDEBUG 

profiling:
	g++ -O2 -o backup $(CFLAGS) $(LDFLAGS) $(SRC) -g -pg -DPROFILING

release:
	g++ -O2 -o backup $(CFLAGS) $(LDFLAGS) $(SRC) 
	strip -x -s backup

clean:
	rm -rf backup
	find . | grep "~" | xargs rm

devel:
	gedit Makefile test.xml &
	find ./inc/ | grep .h | xargs gedit &
	find ./src/ | grep .cpp | xargs gedit &
