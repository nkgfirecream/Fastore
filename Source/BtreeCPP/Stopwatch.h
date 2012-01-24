#pragma once
#include <windows.h>
#include <time.h>

class Stopwatch
{
public:
	Stopwatch();

	double GetFrequency(void);

	void StartTimer(void) ;

	double StopTimer(void);

private:
	//LARGE_INTEGER start;

	//LARGE_INTEGER stop;

	clock_t start;
	clock_t end;
	double frequency;
};
