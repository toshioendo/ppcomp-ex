CC = nvc
#CFLAGS = -O3 -acc
CFLAGS = -O3 -acc -Minfo=accel
LDFLAGS = -acc
LIBS = 

APP = bsort
OBJS = $(APP).o

all: $(APP)

$(APP): $(OBJS)
	$(CC) $^ $(LIBS) -o $@ $(LDFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $*.o

clean:
	rm -f *.o
	rm -f $(APP)
	rm -f *~
