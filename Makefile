# build helloworld executable when user executes "make"
all: dot1x
		
dot1x: main.o
	$(CC) $(LDFLAGS) main.o -o dot1x
        
rc4.o: rc4.c
	$(CC) $(CFLAGS) -c rc4.c

main.o: main.c
	$(CC) $(CFLAGS) -c main.c 
        
# remove object files and executable when user executes "make clean"
clean:
	rm *.o *.gch dot1x