#ifndef MEM_ALLOCATOR_H
#define MEM_ALLOCATOR_H


#include <cstdint>
#include <vector>


#define FREE_LIST_SIZE 65

struct Header
{
    uint32_t size;
    Header* next, *prev; //previous and next block of the same size group
    bool isFree;

    Header(uint32_t s, bool f, Header* n, Header* p) :
    	size(s), next(n), prev(p), isFree(f) {}
};

class MemAllocator
{
public:

    MemAllocator();

    MemAllocator(const MemAllocator& other) = delete;
    MemAllocator& operator =(const MemAllocator& other) = delete;

	~MemAllocator();

	void Initialize();

	void* MyMalloc(uint32_t numBytes);
	void MyFree(void* block);

private:

	Header* FindSuitableFreeBlock(uint32_t numBytes);
	void* AllocateNewBlock(uint32_t numBytes);
	void TakeFreeBlock(Header* block);
	void SplitBlock(Header* block, uint32_t sizeToGet);

	uint32_t memSize;
	void* memory;
	void* lastUsedBlock;
	void* freeList[FREE_LIST_SIZE];

	bool isInitialized;
};

#endif // MEM_ALLOCATOR_H
