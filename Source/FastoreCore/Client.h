#pragma once
#include "Database.h"
#include "Host.h"

using namespace fs;

class Client
{
	public:
		Database Connect(Host host);
};