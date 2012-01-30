#include "Stopwatch.h"
#include <time.h>

Stopwatch::Stopwatch()
{
	frequency = GetFrequency();
}

double Stopwatch::GetFrequency()
{
	//LARGE_INTEGER proc_freq;

	//if (!::QueryPerformanceFrequency(&proc_freq)) 
		//throw TEXT("QueryPerformanceFrequency() failed");

	//return proc_freq.QuadPart;

	return (double)CLOCKS_PER_SEC;
}

void Stopwatch::StartTimer()
{
	//DWORD_PTR oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0);

	//::QueryPerformanceCounter(&start);

	//::SetThreadAffinityMask(::GetCurrentThread(), oldmask);
	start = clock();
}

void Stopwatch::StopTimer()
{
	//DWORD_PTR oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0);

	//::QueryPerformanceCounter(&stop);

	//::SetThreadAffinityMask(::GetCurrentThread(), oldmask);

	//return ((stop.QuadPart - start.QuadPart) / frequency);
	end = clock();
	total += end - start;
	
}

void Stopwatch::Reset()
{
	total = 0;
}

double Stopwatch::TotalTime()
{
	return total / frequency;
}
