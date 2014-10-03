#include "stdafx.h"
#include "ThreadPool.h"



ThreadPool::ThreadPool(int threadsCount = 3)
{
	if(threadsCount > 0)
	{
		this->threadsCount = threadsCount;
		threads = (HANDLE *)calloc(threadsCount, sizeof(HANDLE));
		for(int i = 0; i < threadsCount; i++)
		{
			int* number  = (int*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                sizeof(int));
			*number = i;
			threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)startProc,(LPVOID)number, 0, NULL);
		}
	}
}

static DWORD WINAPI startProc(LPVOID lpParam)
{
	return 1;
}

void ThreadPool::AddFunction(FUNC func)
{
}

ThreadPool::~ThreadPool(void)
{
}

