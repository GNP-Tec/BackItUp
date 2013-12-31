all:
	g++ -o backup `xml2-config --cflags` `xml2-config --libs` main.cpp config.cpp fiterator.cpp -Wall -g -DDEBUG

release:
	g++ -o backup `xml2-config --cflags` `xml2-config --libs` main.cpp config.cpp fiterator.cpp -Wall

clean:
	rm -rf backup *~
