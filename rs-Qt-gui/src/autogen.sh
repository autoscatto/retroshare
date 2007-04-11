echo "Creating project file"
qmake -project -o retroshare.pro
echo "Running script to fix project file"
./script/script.pl retroshare.pro
echo "Making Makefiles"
qmake retroshare.pro
echo "You can now build retroshare by running make"


