#include <hash_map>
#include <hash_set>
#include "Stopwatch.h"
#include <iostream>
using namespace std;

class STDHashTest
{
	public:
	void RunTest();
};

void STDHashTest::RunTest()
{
	long numrows = 1000000;
	Stopwatch watch;
	stdext::hash_map<long,long> hash;

	cout << "Testing STD Hash Map"<< "\n\r";
	watch.GetFrequency();
	typedef stdext::pair<long,long> LongPair;
	for(long i = 0; i < numrows; i++)
	{
		watch.StartTimer();
		hash.insert(LongPair(i,i));
		watch.StopTimer();
	}

	double secs = watch.TotalTime();

	cout << "Total time: " << secs<< "\n\r";
	cout << "Inserts per second: " << numrows/secs<< "\n\r";
	
	stdext::hash_set<long> hashset;

	cout << "Testing STD Hash Set"<< "\n\r";
	watch.Reset();

	for(long i = 0; i < numrows; i++)
	{
		watch.StartTimer();
		hashset.insert(i);
		watch.StopTimer();
	}

	secs = watch.TotalTime();

	cout << "Total time: " << secs<< "\n\r";
	cout << "Inserts per second: " << numrows/secs<< "\n\r";
}