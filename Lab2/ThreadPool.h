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

enum ThreadState
{
	TS_FREE, TS_BUSY, TS_DELETED
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
	void RecreateThread();
	void NewThread(int i);
	HANDLE hSemaphore;
	time_t functionAddTime;
	CRITICAL_SECTION criticalSection;
	ThreadState* threadsStates;
	int busyThreadsCount;
	int deletedThreadsCount;
	std::ostream* logStream;
};

struct StartThreadArgument
{
	int ThreadNumber;
	ThreadPool* pool;
};



