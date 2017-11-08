OBJS=main.o code.o parse.o scanner.o  syntable.o TM.o

all: mini-c

mini-c: $(OBJS)
	$(CC) -o $@ $^
	
%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^
clean:
	rm *.o mini-c -f

.PHONY: all clean

