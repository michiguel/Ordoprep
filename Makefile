CC = gcc
CFLAGS = -DNDEBUG -flto -I myopt 
CFLAGSD = -I myopt
WARN = -Wwrite-strings -Wconversion -Wshadow -Wparentheses -Wlogical-op -Wunused -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement -W -Wall -Wextra
OPT = -O3
LIBFLAGS = -lm
EXE = ordoprep

SRC = main.c main2.c myopt/myopt.c proginfo.c pgnget.c mymem.c namehash.c inidone.c plyrs.c bitarray.c strlist.c csv.c
DEPS = bool_t.h  main2.h  proginfo.h  progname.h  version.h pgnget.h mymem.h namehash.h inidone.h plyrs.h bitarray.h strlist.h csv.h
OBJ = main.o main2.o myopt/myopt.o proginfo.o pgnget.o mymem.o namehash.o inidone.o plyrs.o bitarray.o strlist.o csv.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

ordoprep: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(WARN) $(OPT) $(LIBFLAGS)

all:
	$(CC) $(CFLAGS) $(WARN) $(OPT) -o $(EXE) $(SRC) $(LIBFLAGS)

debug:
	$(CC) $(CFLAGSD) $(WARN) $(OPT) -o $(EXE) $(SRC) $(LIBFLAGS)

install:
	cp $(EXE) /usr/local/bin/$(EXE)

clean:
	rm -f *.o *~ myopt/*.o ordo-v*.tar.gz ordo-v*-win.zip *.out









