#pragma once
#include <queue>
#include <time.h>
#include "windows.h"


typedef DWORD (WINAPI *FUNC)(LPVOID lpParam);


class ThreadPool
{
public:
	ThreadPool(int);
	~ThreadPool(void);
	void AddFunction(FUNC func, int argument);
private:
	int threadsCount;
	std::queue<ThreadArgument*> funcQueue;
	HANDLE * threads;
	DWORD WINAPI ThreadProc(LPVOID lpParam);
	HANDLE hSemaphore;
	time_t functionAddTime;
	CRITICAL_SECTION criticalSection;
	bool* busyThreads;
	int busyThreadsCount;
};

struct ThreadArgument
{
	int ThreadNumber;
	FUNC WorkFunc;
	int WorkFuncArgument;
};

