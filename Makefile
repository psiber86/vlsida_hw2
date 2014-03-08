CC = g++
CFLAGS = -O0 -g -Wall

all: 
	$(CC) $(CFLAGS) Main_placeroute.cpp Cell.cpp Placer.cpp -o placeAndRoute

clean:
	rm -rf *.o placeAndRoute
