name=server
CC=g++
LIBS = -levent -levent_pthreads -lxerces-c -lpthread -lglog

# --- targets
all:   
	g++ $(name).cpp -o $(name) $(LIBS)
# --- remove binary and executable files
clean:
	rm -f $(name).o $(name)
#
