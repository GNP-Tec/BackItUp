SRC=src/main.cpp src/config.cpp src/fiterator.cpp

all:
	g++ -o backup `xml2-config --cflags` `xml2-config --libs` $(SRC) -llogger -Wall -g -DDEBUG

release:
	g++ -o backup `xml2-config --cflags` `xml2-config --libs` $(SRC) -llogger -Wall
	strip -x -s backup

clean:
	rm -rf backup
