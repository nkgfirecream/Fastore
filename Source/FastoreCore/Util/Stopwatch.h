#pragma once
#if defined(_WIN32)
#include <windows.h>
#endif

#include <time.h>

class Stopwatch
{
public:
	Stopwatch();

	double GetFrequency(void);

	void StartTimer(void) ;

	void StopTimer(void);

	double TotalTime(void);

	void Reset(void);

private:
	//LARGE_INTEGER start;

	//LARGE_INTEGER stop;
	clock_t total;
	clock_t start;
	clock_t end;
	double frequency;
};
