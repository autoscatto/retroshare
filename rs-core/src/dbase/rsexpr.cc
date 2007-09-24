
#include "dbase/findex.h"
#include "rsiface/rsexpr.h"


/******************************************************************************************
Boolean Search Expression
classes:

	Expression: 		The base class of all expression typest
	CompoundExpression: The expression which uses a logical operator to combine
							the results of two expressions
	StringExpression: 	An expression which uses some sort of string comparison.
	RelExpression: 		A Relational Expression where > < >= <= == make sense. 
							e.g. size date etc

All of the above are abstract classes
******************************************************************************************/

bool NameExpression::eval(FileEntry *file) 
{
	return evalStr(file->name);
}

/******************************************************************************************
Some implementations of Relational Expressions.

******************************************************************************************/

bool DateExpression::eval(FileEntry *file)
{
		return evalRel(file->modtime);	
}

bool SizeExpression::eval(FileEntry *file)
{
	return evalRel(file->size);	
}

bool PopExpression::eval(FileEntry *file)
{
	return evalRel(file->pop);
}

bool StringExpression::evalStr(std::string &str){
	std::list<std::string>::const_iterator iter;
	switch (Op) {
		case ContainsAllStrings:
			for (iter = terms.begin(); iter != terms.end(); iter++) {	
				if (str.find(*iter)== std::string::npos) {
					return false;	
				} 
			}
			return true;
		break;
		case ContainsAnyStrings:
			for (iter = terms.begin(); iter != terms.end(); iter++) {
				if (str.find(*iter)!= std::string::npos) {
					return true;
				}
			}
		break;
		case EqualsString:
			iter = find(terms.begin(),terms.end(),str);
			if (iter != terms.end()) {
				return true;	
			}
		break;
		default:
			return false;
	}
	return false;
}

bool PathExpression::eval(FileEntry *file){
	std::string path;
	/*Construct the path of this file*/
	DirEntry * curr = file->parent;
	while ( curr != NULL ){
		path = curr->name+"/"+ path;
		curr = curr->parent;
	}
	return evalStr(path);
}

bool ExtExpression::eval(FileEntry *file){
	std::string ext;
	/*Get the part of the string after the last instance of . in the filename */
	unsigned int index = file->name.find_last_of('.');
	if (index != std::string::npos) {
		ext = file->name.substr(index+1);
		if (ext != "" ){
			return evalStr(ext);	
		}
	}
	return false;
}
