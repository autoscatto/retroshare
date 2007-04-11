

Building should be simple ,You will need qt-4.2
or greater .


Installation
-------------

We have now proper project file in svn, so you can just run

1. qmake

2. edit Makefile.Release the line:
LIBS        =        -L"c:\Qt\4.2.0\lib" -lmingw32 -lqtmain -lQtXml4 -lQtGui4 -lQtNetwork4 -lQtCore4

and add the compiled Libs for RetroShare core in a folder and set the Libs path,
example:
-L"D:/Development/retro/libs" -lretroshare -lssl -lcrypto -lpthreadGC2d -lKadC -lz -lws2_32 -luuid -lole32 -liphlpapi

then will be look this one the LIBS in Makefile.Release :
LIBS        =        -L"c:\Qt\4.2.0\lib" -lmingw32 -lqtmain -lQtXml4 -lQtGui4 -lQtNetwork4 -lQtCore4 -L"D:/Development/retro/libs" -lretroshare -lssl -lcrypto -lpthreadGC2d -lKadC -lz -lws2_32 -luuid -lole32 -liphlpapi

3. make

If you have problems, try first run
 With Linux
./autogen.sh
 With Windows
qmake -project




Windows put compiled binary to directory "release", you can either use explorer or console to launch it.
If RetroShare.exe says missing dll's, you need go to qt's bin dir(usually C:\qt\4.2.1\bin) and copy all dll's to Windows\System32 dir.

-----------------------------------------------------------------------------------------

Generate a new or update a language file 

lupdate retroshare.pro

to build:

lrelease retroshare.pro

with Linguist can modify the .ts files to can translate.
