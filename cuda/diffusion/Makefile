CC = nvcc
# Added options to avoid link error when the program uses
# large global arrays. (May 12, 2025, Endo)
CFLAGS = -O3 -g -Xcompiler -mcmodel=medium
#CFLAGS = -O3 -g
LDFLAGS = -Xcompiler -mcmodel=medium
#LDFLAGS =
LIBS = 

all: diffusion

mm: diffusion.o
	$(CC) $^ $(LIBS) -o $@ $(LDFLAGS)

%.o : %.cu
	$(CC) $(CFLAGS) -c $< -o $*.o

clean:
	rm -f *.o
	rm -f *~
	rm -f diffusion
