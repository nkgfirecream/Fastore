#include <EASTL\hash_map.h>
#include <EASTL\hash_set.h>
#include "Stopwatch.h"
#include <iostream>
using namespace eastl;

class EAHashTest
{
	public:
	void RunTest();
};

void EAHashTest::RunTest()
{
	long numrows = 1000000;
	Stopwatch watch;
	eastl::hash_map<long,long> hash;

	cout << "Testing EA Hash Map" << "\n\r";
	watch.GetFrequency();
	typedef eastl::pair<long,long> LongPair;
	for(long i = 0; i < numrows; i++)
	{
		watch.StartTimer();
		hash.insert(LongPair(i,i));
		watch.StopTimer();
	}

	double secs = watch.TotalTime();

	cout << "Total time: " << secs << "\n\r";
	cout << "Inserts per second: " << numrows/secs << "\n\r";
	
	eastl::hash_set<long> hashset;

	cout << "Testing EA Hash Set" << "\n\r";
	watch.Reset();

	for(long i = 0; i < numrows; i++)
	{
		watch.StartTimer();
		hashset.insert(i);
		watch.StopTimer();
	}

	secs = watch.TotalTime();

	cout << "Total time: " << secs << "\n\r";
	cout << "Inserts per second: " << numrows/secs << "\n\r";
}