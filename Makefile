SRC=src/main.cpp src/config.cpp src/fiterator.cpp src/filehandler.cpp

all:
	g++ -O2 -o backup `xml2-config --cflags` `xml2-config --libs` $(SRC) -llogger -larchive -Wall -g -DDEBUG

release:
	g++ -O2 -o backup `xml2-config --cflags` `xml2-config --libs` $(SRC) -llogger -larchive -Wall
	strip -x -s backup

clean:
	rm -rf backup
