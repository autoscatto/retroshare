#!/usr/bin/perl
################################################
#Script to parse out unwanted INCLUDEPATH lines#
#because they cause the failing of build.      #
################################################

open(PRO, $ARGV[0]);
@pro = <PRO>;
close(PRO);

foreach(@pro)
{
	$begin = 1 if($_ =~ /DEPENDPATH/);
	$begin = 0 if($_ =~ /HEADERS/);
	s/\//\\/ if($begin);
	s/\n/\r\n/;
	push(@output, $_);
}
open(PRO, ">$ARGV[0]");
print PRO @output;
close(PRO);
