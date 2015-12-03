#include <iostream>
#include "MemAllocator.h"


namespace Allocator
{
	static MemAllocator a;

	void* MyMalloc(uint32_t n)
	{
		a.Initialize();
		return a.MyMalloc(n);
	}

	void MyFree(void* p)
	{
			a.MyFree(p);
	}
}

int main()
{
    return 0;
}
