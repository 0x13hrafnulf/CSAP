CFLAGS=-g

EXE=midterm3_server midterm3_client
all:$(EXE)
clean:
	rm *.o $(EXE)

midterm3_server.o: midterm3_server.c midterm3.h
midterm3_client.o: midterm3_client.c midterm3.h

midterm3_server: midterm3_server.o midterm3_lib.o midterm3.h
	cc $(CFLAGS) midterm3_server.o midterm3_lib.o -o midterm3_server

midterm3_client: midterm3_client.o midterm3_lib.o midterm3.h
	cc $(CFLAGS) midterm3_client.o midterm3_lib.o -o midterm3_client

