
RS_TOP_DIR = ..
include ../make.opt

OBJ = upnphandler.o upnputil.o

UPNPC_DIR = /home/rmf24/prog/src/miniupnp/miniupnpc-20070515
CFLAGS += -I$(UPNPC_DIR)

LIBS += -lminiupnpc
RSLIBS += -lminiupnpc

all : $(OBJ) librs upnptest

upnptest: $(OBJ) upnptest.o
	$(CC) $(CFLAGS) -o upnptest $(OBJ) upnptest.o $(RSLIBS)

librs:  $(OBJ)
	$(AR) r $(LIBRS) $(OBJ)
	$(RANLIB) $(LIBRS)

.cc.o:
	$(CC) $(CFLAGS) -c $<

clean:
	-/bin/rm $(OBJ) upnptest.o

clobber: clean
	-/bin/rm upnptest

