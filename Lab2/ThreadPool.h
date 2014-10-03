#pragma once
#include <queue>
#include "windows.h"


typedef DWORD (WINAPI *FUNC)(LPVOID lpParam);


class ThreadPool
{
public:
	ThreadPool(int);
	~ThreadPool(void);
	void AddFunction(FUNC func);
private:
	int threadsCount;
	std::queue<FUNC> threadQueue;
	HANDLE * threads;
};

