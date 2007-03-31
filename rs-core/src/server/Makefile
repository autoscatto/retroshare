
include ../make.opt

OBJ = filedexserver.o pqifiler.o

all : $(OBJ) librs

librs:  $(OBJ)
	$(AR) r $(LIBRS) $(OBJ)
	$(RANLIB) $(LIBRS)

.cc.o:
	$(CC) $(CFLAGS) -c $<

clean:
	-/bin/rm $(OBJ) 

clobber: clean
#	-/bin/rm 


