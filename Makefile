CC = g++
CFLAGS = -O0 -g -Wall -Wextra -pedantic -std=c++0x

default: channel

channel:
	$(CC) $(CFLAGS) -DCHANNEL_ROUTING Main_placeroute.cpp Cell.cpp Placer.cpp channel_routing.cpp -o channel_apr

clean:
	rm -f *_apr *~
