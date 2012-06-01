#pragma once

#include "..\FastoreCore\CoreConfig.h"

const int DefaultPort = 8064;
const int DefaultSocketThreadCount = 8;
const int DefaultSocketPendingCount = 64;

struct ServiceConfig
{
	int port;
	int socketThreadCount;
	int socketPendingCount;

	CoreConfig coreConfig;

	ServiceConfig() : port(DefaultPort), socketThreadCount(DefaultSocketThreadCount), socketPendingCount(DefaultSocketPendingCount)
	{ }
};