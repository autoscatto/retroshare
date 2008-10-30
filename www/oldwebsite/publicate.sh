# CON MOUNT 
# fstab:	sshfs#user@shell.sourceforge.net:/  /mnt/sourceforge/   fuse    uid=1000,gid=100,rw,user,noauto,umask=0,allow_other   0 0
mount /mnt/sourceforge/
cd  ~/svn/retroshare/www/
cp -uav * /mnt/sourceforge/home/groups/r/re/retroshare/htdocs/


# CON SSH
cd  ~/svn/retroshare/www/
ssh      user@shell.sourceforge.net "rm -rf /home/groups/r/re/retroshare/htdocs/*"
scp -r * user@shell.sourceforge.net:/home/groups/r/re/retroshare/htdocs/


