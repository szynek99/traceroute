CC = g++
CFLAGS  = -std=gnu++17 -Wall -Wextra

all: traceroute

traceroute:  traceroute.o utility.o 
	$(CC) $(CFLAGS) -o traceroute traceroute.o utility.o


traceroute.o:  traceroute.cpp utility.h 
	$(CC) $(CFLAGS) -c traceroute.cpp 


utility.o:  utility.cpp utility.h 
	$(CC) $(CFLAGS) -c utility.cpp

distclean: 
	$(RM) traceroute *.o *~

clean: 
	$(RM) *.o *~