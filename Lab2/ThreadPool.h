#pragma once
#include <queue>
#include <time.h>
#include <iostream>
#include "windows.h"


typedef DWORD (WINAPI *FUNC)(LPVOID lpParam);

struct ThreadArgument
{
	int ThreadNumber;
	FUNC WorkFunc;
	int WorkFuncArgument;
};

class ThreadPool
{
public:
	ThreadPool(int, std::ostream*);
	~ThreadPool(void);
	void AddFunction(FUNC func, int argument);
private:
	int threadsCount;
	std::queue<ThreadArgument*> funcQueue;
	HANDLE * threads;
	DWORD WINAPI ThreadProc(LPVOID lpParam);
	void DeleteUnusedThreads(ThreadArgument* threadArgument);
	HANDLE hSemaphore;
	time_t functionAddTime;
	CRITICAL_SECTION criticalSection;
	bool* busyThreads;
	int busyThreadsCount;
	std::ostream* logStream;
};



