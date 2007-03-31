
include ../make.opt

OBJ = rsthreads.o  rsdir.o

all : $(OBJ) librs dirtest

dirtest: $(OBJ) dirtest.o
	$(CC) $(CFLAGS) -o dirtest $(OBJ) dirtest.o $(RSLIBS)

librs:  $(OBJ)
	$(AR) r $(LIBRS) $(OBJ)
	$(RANLIB) $(LIBRS)

.cc.o:
	$(CC) $(CFLAGS) -c $<

clean:
	-/bin/rm $(OBJ) dirtest.o

clobber: clean
	-/bin/rm dirtest

