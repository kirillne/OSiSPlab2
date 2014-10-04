// Lab2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "ThreadPool.h"
#include <synchapi.h>


DWORD WINAPI Sum(LPVOID lpParam)
{
	Sleep(3000);
	std::cout << "Sum done\n";
	return 0;
}

DWORD WINAPI Error(LPVOID lpParam)
{
	Sleep(2000);
	int zero = 0;
	//int a = 1/zero;
	throw new std::exception("Exception");
	std::cout << "Error done\n";
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int threadsCount;
	std::cin >> threadsCount;
	ThreadPool* pool = new ThreadPool(threadsCount, &std::cout);
	while (true)
	{
		char command;
		std::cin >> command;
		switch (command)
		{
		case 'a':
			pool->AddFunction(Sum,0);
			break;
		case 'e':
			pool->AddFunction(Error,0);
			break;
		case 'c':
			delete pool;
			return 0;
		}
	}

}



