#pragma once
#include <queue>
#include <time.h>
#include <iostream>
#include "windows.h"


typedef int (*FUNC)(int param);

struct FuncForThread
{
	FUNC WorkFunc;
	int WorkFuncArgument;
};

class ThreadPool
{
public:
	ThreadPool(int, std::ostream*);
	~ThreadPool(void);
	void AddFunction(FUNC func, int argument);
	DWORD WINAPI ThreadProc(LPVOID lpParam);
	
private:
	int threadsCount;
	std::queue<FuncForThread*> funcQueue;
	HANDLE * threads;
	void DoFunction(int threadNumber);
	void DeleteUnusedThreads(int threadNumber);
	HANDLE hSemaphore;
	time_t functionAddTime;
	CRITICAL_SECTION criticalSection;
	bool* busyThreads;
	int busyThreadsCount;
	std::ostream* logStream;
};

struct StartThreadArgument
{
	int ThreadNumber;
	ThreadPool* pool;
};



