CC = gcc
CFLAGS = -O -g -fopenmp
LDFLAGS = -fopenmp
LIBS = 

APP = qsort
OBJS = $(APP).o

all: $(APP)

$(APP): $(OBJS)
	$(CC) $^ $(LIBS) -o $@ $(LDFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $*.o

clean:
	rm -f *.o
	rm -f *~
	rm -f $(APP)
