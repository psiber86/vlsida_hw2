CC = g++
CFLAGS = -Wall -Wextra -pedantic -std=c++0x

default: channel

channel: Main_placeroute.cpp Cell.cpp Placer.cpp channel_routing.cpp Cell.h Placer.h channel_routing.hpp
	$(CC) $(CFLAGS) -O3 -DCHANNEL_ROUTING Main_placeroute.cpp Cell.cpp Placer.cpp channel_routing.cpp -o channel_apr

debug: Main_placeroute.cpp Cell.cpp Placer.cpp channel_routing.cpp Cell.h Placer.h channel_routing.hpp
	g++ -g -O0 $(CFLAGS) -DCHANNEL_ROUTING -DDEBUG Main_placeroute.cpp Cell.cpp Placer.cpp channel_routing.cpp -o channel_apr

clean:
	rm -f *_apr *~
