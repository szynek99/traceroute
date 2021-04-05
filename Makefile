CC = g++
CFLAGS  = -std=c++11 -std=gnu++11 -std=c++14 -std=gnu++14 -std=gnu++17 -Wall -Wextra

all: traceroute

traceroute:  traceroute.o utility.o 
	$(CC) $(CFLAGS) -o traceroute traceroute.o utility.o
	$(RM) *.o *~


traceroute.o:  traceroute.cpp utility.h 
	$(CC) $(CFLAGS) -c traceroute.cpp 


utility.o:  utility.cpp utility.h 
	$(CC) $(CFLAGS) -c utility.cpp

clean: 
	$(RM) traceroute *.o *~