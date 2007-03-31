
/*
 * "$Id: rsdir.cc,v 1.1 2007-02-19 20:08:30 rmf24 Exp $"
 *
 * RetroShare C++ Interface.
 *
 * Copyright 2004-2007 by Robert Fernie.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License Version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems to "retroshare@lunamutt.com".
 *
 */



#include "util/rsdir.h"
#include <string>
#include <iostream>

std::string 	RsDirUtil::getTopDir(std::string dir)
{
	std::string top;

	/* find the subdir: [/][dir1.../]<top>[/]
	 */
	int i,j;
	int len = dir.length();
	for(j = len - 1; (j > 0) && (dir[j] == '/'); j--);
	for(i = j; (i > 0) && (dir[i] != '/'); i--);
	if (i == j)
		return top; /* empty */
	if (dir[i] == '/')
		i++;

	for(; i <= j; i++)
	{
		top += dir[i];
	}

	std::cerr << "getTopDir(" << dir << ") -> " << top << std::endl;
	return top;
}


std::string 	RsDirUtil::removeTopDir(std::string dir)
{
	std::string rest;

	/* remove the subdir: [/][dir1.../]<top>[/]
	 */
	int i,j;
	int len = dir.length();
	for(j = len - 1; (j > 0) && (dir[j] == '/'); j--);
	for(i = j; (i > 0) && (dir[i] != '/'); i--);
	if (i == j)
		return rest; /* empty */
	if (dir[i] == '/')
		i++;

	/* topdir (as above) */
	/* now back past all //'s and well be at end of restDirs */
	if ((i > 0) && (dir[i-1] == '/'))
	{
		i--;
		for(; (i > 0) && (dir[i] == '/'); i--);
	}

	for(j = 0; j <= i; j++)
	{
		rest += dir[j];
	}

	std::cerr << "removeTopDir(" << dir << ") -> " << rest << std::endl;
	return rest;
}



std::string 	RsDirUtil::getRootDir(std::string dir)
{
	std::string root;

	/* find the subdir: [/]root[/...]
	 */
	int i,j;
	int len = dir.length();
	for(i = 0; (i < len) && (dir[i] == '/'); i++);
	for(j = i; (j < len) && (dir[j] != '/'); j++);
	if (i == j)
		return root; /* empty */
	if (dir[j] == '/')
		j--;

	for(; i <= j; i++)
	{
		root += dir[i];
	}

	std::cerr << "getRootDir(" << dir << ") -> " << root << std::endl;
	return root;
}
std::string 	RsDirUtil::removeRootDir(std::string path, std::string root)
{
	/* too tired */
	std::string notroot;
	std::cerr << "remoteRootDir( TODO! )";

	int i = 0, j = 0;

	/* catch empty data */
	if ((root.length() < 1) || (path.length() < 1))
		return notroot;
		
	if ((path[0] == '/') && (root[0] != '/'))
	{
		i++;
	}

	for(; (i < path.length()) && (j < root.length()) && (path[i] == root[j]); i++, j++);

	/* should have consumed root. */
	if (j == root.length())
	{
		std::cerr << "matched root!" << std::endl;
	}
	else
	{
		std::cerr << "failed i: " << i << ", j: " << j << std::endl;
		std::cerr << "root: " << root << " path: " << path << std::endl;
		return notroot;
	}

	if (path[i] == '/')
	{
		i++;
	}

	for(; i < path.length(); i++)
	{
		notroot += path[i];
	}

	std::cerr << "Found NotRoot: " << notroot << std::endl;

	return notroot;
}

std::string 	RsDirUtil::getRootDir2(std::string)
{
	/* too tired */
	std::string notroot;
	std::cerr << "remoteRootDir( TODO! )";
	return notroot;
}

