CC = "gcc"

dot1x: 
	$(CC) $(LDFLAGS) main.c -o dot1x -DBUILD_DATE="\"`date '+%Y-%m-%d %H:%M:%S'`"\"
clean:
	rm dot1x