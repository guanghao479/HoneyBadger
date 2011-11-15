CC=g++
LIBS= -lxerces-c
OBJ = SgSimpleSockClient.o SgSimpleSockClientTest.o
EXE=client

%.o: %.c
	$(CC) -c -o $@ $^ $(LIBS)

all: $(OBJ)
	$(CC) -o $(EXE) $^ $(LIBS)

clean:
	rm $(OBJ) $(EXE)
