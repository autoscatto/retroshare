
include ../make.opt

OBJ = filedex.o filelook.o

all : $(OBJ) librs ftest looktest

ftest: $(OBJ) ftest.o
	$(CC) $(CFLAGS) -o ftest $(OBJ) ftest.o $(RSLIBS)

looktest: $(OBJ) looktest.o
	$(CC) $(CFLAGS) -o looktest $(OBJ) looktest.o $(RSLIBS)

librs:  $(OBJ)
	$(AR) r $(LIBRS) $(OBJ)
	$(RANLIB) $(LIBRS)

.cc.o:
	$(CC) $(CFLAGS) -c $<

clean:
	-/bin/rm $(OBJ) ftest.o looktest.o

clobber: clean
	-/bin/rm ftest looktest

