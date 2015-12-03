#include "MemAllocator.h"

#include <cstdlib>


#define ALIGNMENT (1 << 3)

MemAllocator::MemAllocator() : memSize(0), memory(NULL), lastUsedBlock(NULL), isInitialized(false)
{
	for(int i = 0; i < FREE_LIST_SIZE; ++i) this->freeList[i] = NULL;
}


MemAllocator::~MemAllocator()
{
	free(this->memory);
}


void MemAllocator::Initialize()
{
	if(this->isInitialized) return;

	uint32_t sizeToTryToAllocate = (1 << 31);
	do
	{
		this->memory = malloc(sizeToTryToAllocate);
		sizeToTryToAllocate /= 2;
	}while(this->memory == NULL);

	this->memSize = sizeToTryToAllocate * 2;

	this->isInitialized = true;
}


void* MemAllocator::MyMalloc(uint32_t numBytes)
{
	void* dataAddr = NULL;

    if(numBytes < 64)
	{
		for(int i = numBytes; i <= 64; ++i)
		{
			if(this->freeList[i] != NULL)
			{
				dataAddr = this->freeList[i];
				break;
			}
		}

		if(dataAddr != NULL)
		{
            this->TakeFreeBlock((Header*)dataAddr);

            if(((Header*)dataAddr)->size > numBytes)
			{
				this->SplitBlock((Header*)dataAddr, numBytes);
			}

            dataAddr = (char*)dataAddr + sizeof(Header);
		}
	}

	else
	{
        void* freeBlock = this->FindSuitableFreeBlock(numBytes);
		if(freeBlock != NULL)
		{
			this->TakeFreeBlock((Header*)freeBlock);

			if(((Header*)freeBlock)->size > numBytes)
			{
				this->SplitBlock((Header*)freeBlock, numBytes);
			}

			dataAddr = (char*)freeBlock + sizeof(Header);
		}

	}

	if(dataAddr == NULL)
	{
		dataAddr = this->AllocateNewBlock(numBytes);
		if(dataAddr != NULL)
		{
            dataAddr = (char*)dataAddr + sizeof(Header);
		}
	}


	return dataAddr;
}


void MemAllocator::MyFree(void* block)
{
    Header* blockHeader = (Header*)((char*)block - (sizeof(Header)));

	size_t sizeIndex = (blockHeader->size < 64) ? blockHeader->size : 64;
    Header* firstFreeOfSameSizeGroup = (Header*)(this->freeList[sizeIndex]);

    blockHeader->isFree = true;
    blockHeader->prev = NULL;


	if(firstFreeOfSameSizeGroup == NULL)
	{
		this->freeList[sizeIndex] = blockHeader;
		blockHeader->next = NULL;
	}

	else
	{
		firstFreeOfSameSizeGroup->prev = blockHeader;
		blockHeader->next = firstFreeOfSameSizeGroup;
	}

}


Header* MemAllocator::FindSuitableFreeBlock(uint32_t numBytes)
{
	Header* currentBlock = (Header*)this->freeList[64];

	if(currentBlock == NULL) return currentBlock;

	while(currentBlock->size < numBytes && currentBlock->next != NULL)
	{
		currentBlock = currentBlock->next;
	}

	return currentBlock;
}


void* MemAllocator::AllocateNewBlock(uint32_t numBytes)
{
	if(this->lastUsedBlock == NULL)
	{
		this->lastUsedBlock = (char*)(this->memory) + (ALIGNMENT - (sizeof(Header) % ALIGNMENT));
	}

	else
	{
		Header* lastUsedBlockHeader = (Header*)(this->lastUsedBlock);

		//the gap between the last used byte and the first following address, multiple of ALIGNMENT
		size_t gapSize = ALIGNMENT -
				(size_t)(lastUsedBlockHeader + sizeof(Header) + lastUsedBlockHeader->size) % ALIGNMENT;

		this->lastUsedBlock = (sizeof(Header) % ALIGNMENT == gapSize) ?
						((char*)(this->lastUsedBlock) + sizeof(Header) + lastUsedBlockHeader->size) :
						((char*)(this->lastUsedBlock) + sizeof(Header) + lastUsedBlockHeader->size +
						(ALIGNMENT -
							(size_t)((char*)(this->lastUsedBlock) + sizeof(Header) + lastUsedBlockHeader->size
							+ sizeof(Header)) % ALIGNMENT));


	}

	if((char*)(this->lastUsedBlock) + sizeof(Header) + numBytes > ((char*)(this->memory) + this->memSize))
	{
		return NULL;
	}

    Header* newHeader = (Header*)(this->lastUsedBlock);
    newHeader->size = numBytes;
	newHeader->prev = NULL;
	newHeader->next = NULL;
	newHeader->isFree = false;

    return ((char*)(this->lastUsedBlock) + sizeof(Header));
}


void MemAllocator::TakeFreeBlock(Header* block)
{
	block->isFree = false;

	if(block->prev == NULL && block->next == NULL)
	{
		this->freeList[(block->size < 64) ? block->size : 64] = NULL;
		return;
	}

	if(block->prev != NULL)
	{
		block->prev->next = block->next;
	}

	if(block->next != NULL)
	{
		block->next->prev = block->prev;
	}

}


void MemAllocator::SplitBlock(Header* block, uint32_t sizeToGet)
{
	if((sizeof(Header) + block->size + sizeof(Header)) > (block->size + sizeof(Header)))
	{
		return;
	}

	Header* freePart = (Header*)(block + sizeof(Header) + sizeToGet);

	freePart->isFree = true;
	freePart->size = block->size - sizeToGet;

	block->size = sizeToGet;

	this->MyFree((void*)(freePart + (sizeof(Header))));
}
