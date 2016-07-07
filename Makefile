# build helloworld executable when user executes "make"
all: dot1x
		
dot1x: main.o
	$(CC) $(LDFLAGS) main.o -o dot1x

main.o: main.c
	$(CC) $(CFLAGS) -c main.c 
        
# remove object files and executable when user executes "make clean"
clean:
	rm -rf *.o *.gch dot1x