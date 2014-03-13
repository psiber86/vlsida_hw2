CC = g++
CFLAGS = -O0 -g -Wall -Wextra -pedantic -std=c++0x

all: 
	$(CC) $(CFLAGS) Main_placeroute.cpp Cell.cpp Placer.cpp -o placeAndRoute

clean:
	rm -f *.o placeAndRoute *~
