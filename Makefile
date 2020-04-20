#
#  Makefile for javacquire.cpp program
#
CFLAGS = -lm
SRC = javacquire.cpp
OBJ = javacquire.o process_input.o get_shm.o acquire_data.o

javacquire: $(OBJ)
	g++ $(CFLAGS) -o javacquire $(OBJ)

javacquire.o: javacquire.cpp
	g++ -c javacquire.cpp
	
process_input.o: process_input.cpp
	g++ -c process_input.cpp

get_shm.o: get_shm.cpp
	g++ -c get_shm.cpp

acquire_data.o: acquire_data.cpp
	g++ -c acquire_data.cpp

FORCE:

clean:
	rm javacquire $(OBJ)
