SRC=src/main.cpp src/config.cpp src/fiterator.cpp src/rfilehandler.cpp src/cfilehandler.cpp

V_MAJ=0
V_MIN=1

all:
	g++ -O2 -o backup `xml2-config --cflags` `xml2-config --libs` $(SRC) -llogger -larchive -Wall -Wno-delete-non-virtual-dtor -DV_MAJ=$(V_MAJ) -DV_MIN=$(V_MIN) -g -DDEBUG 

profiling:
	g++ -O2 -o backup `xml2-config --cflags` `xml2-config --libs` $(SRC) -llogger -larchive -Wall -Wno-delete-non-virtual-dtor -DV_MAJ=$(V_MAJ) -DV_MIN=$(V_MIN) -g -pg -DPROFILING

release:
	g++ -O2 -o backup `xml2-config --cflags` `xml2-config --libs` $(SRC) -llogger -larchive -Wall -Wno-delete-non-virtual-dtor -DV_MAJ=$(V_MAJ) -DV_MIN=$(V_MIN)
	strip -x -s backup

clean:
	rm -rf backup
