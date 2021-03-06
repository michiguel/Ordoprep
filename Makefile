CC = gcc
CFLAGS = -DNDEBUG -flto -I myopt 
CFLAGP = -DNDEBUG -pg -I myopt 
CFLAGSD = -I myopt
WARN = -Wwrite-strings -Wconversion -Wshadow -Wparentheses -Wlogical-op -Wunused -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement -W -Wall -Wextra
OPT = -O3
LIBFLAGS = -lm
EXE = ordoprep

SRC = main.c main2.c myopt/myopt.c proginfo.c pgnget.c mymem.c namehash.c inidone.c plyrs.c bitarray.c strlist.c csv.c justify.c myhelp.c encount.c groups.c mytimer.c
DEPS = boolean.h main2.h proginfo.h progname.h version.h pgnget.h mymem.h namehash.h inidone.h plyrs.h bitarray.h strlist.h csv.h mytypes.h ordolim.h datatype.h mystr.h justify.h myhelp.h encount.h groups.h mytimer.h
OBJ = main.o main2.o myopt/myopt.o proginfo.o pgnget.o mymem.o namehash.o inidone.o plyrs.o bitarray.o strlist.o csv.o justify.o myhelp.o encount.o groups.o mytimer.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXE): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(WARN) $(OPT) $(LIBFLAGS)

all:
	$(CC) $(CFLAGS) $(WARN) $(OPT) -o $(EXE) $(SRC) $(LIBFLAGS)

profile:
	$(CC) $(CFLAGP) $(WARN) -o $(EXE) $(SRC) $(LIBFLAGS)

debug:
	$(CC) $(CFLAGSD) $(WARN) $(OPT) -o $(EXE) $(SRC) $(LIBFLAGS)

install:
	cp $(EXE) /usr/local/bin/$(EXE)

clean:
	rm -f *.o *~ myopt/*.o ordo-v*.tar.gz ordo-v*-win.zip *.out

$(OBJ): $(DEPS)








