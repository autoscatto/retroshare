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
	if($_ !~ /#/ && !$done)
	{
		$def =
			"OBJECTS_DIR = temp/obj\n" .
			"RCC_DIR = temp/qrc\n" .
			"UI_DIR  = temp/ui\n" .
			"MOC_DIR = temp/moc\n" .
			"QT     += network xml\n";

		push(@output, $def);
		$done = 1;
	}
	if($_ =~ /util/ && $begin)
	{
		$begin = 0;
		next;
	}
	push(@output, $_);
	$begin = 1 if($_ =~ /INCLUDEPATH/);
}
open(PRO, ">$ARGV[0]");
print PRO @output;
close(PRO);
