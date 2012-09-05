#pragma once
#include "Module.h"
#include "CModule.h"

void intializeFastoreModule(sqlite3* db, int argc, void* argv)
{
	//Convert from c to cpp...
	std::vector<module::Address> mas;
	module::Address address;

	string text((char*)argv);
	istringstream reader(text, istringstream::in);
	std::getline(reader, address.Name, ';');

	string numtext;
	std::getline(reader, numtext);
	
	address.Port = atoi(numtext.c_str());

	mas.push_back(address);
	intializeFastoreModule(db, mas);
}