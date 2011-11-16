CC=g++
LIBS= -lxerces-c
OBJ = SimpleSockClient.o SimpleSockClientTest.o
EXE=client

%.o: %.c
	$(CC) -c -o $@ $^ $(LIBS)

all: $(OBJ)
	$(CC) -o $(EXE) $^ $(LIBS)

clean:
	rm $(OBJ) $(EXE)
