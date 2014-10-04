#pragma once
#include <queue>
#include <time.h>
#include <iostream>
#include "windows.h"


typedef DWORD (WINAPI *FUNC)(LPVOID lpParam);



class ThreadPool
{
public:
	ThreadPool(int, std::ostream*);
	~ThreadPool(void);
	void AddFunction(FUNC func, int argument);
	DWORD WINAPI ThreadProc(LPVOID lpParam);
private:
	int threadsCount;
	std::queue<FUNC> funcQueue;
	HANDLE * threads;
	
	void DeleteUnusedThreads(int threadNumber);
	HANDLE hSemaphore;
	time_t functionAddTime;
	CRITICAL_SECTION criticalSection;
	bool* busyThreads;
	int busyThreadsCount;
	std::ostream* logStream;
};

struct ThreadArgument
{
	int ThreadNumber;
	FUNC WorkFunc;
	int WorkFuncArgument;
	ThreadPool* pool;
};


