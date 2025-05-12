#include "FV_Mem.h"

#include <chrono>

FV::MemPool::MemPool()
{
   
}

FV::MemPool::~MemPool(){}

void FV::MemPool::init(uint32_t maxGB){
 uint64_t vectorSize = maxGB * 1024;
    vectorSize *= 1024;
    vectorSize *= 1024;
    vectorSize = vectorSize / FV::C_DefaultBlockSize;
   // m_memBlocks.resize(vectorSize);
    maintance();
}

void FV::MemPool::maintance(){
    
    auto t1 = std::chrono::high_resolution_clock::now();

    //step 1;   always have 1 free pool available
    //          free pools if unused and more than 1 available
    bool freePoolFound = false;
    for(size_t i = 0; i < m_memBlocks.size(); i++){
        m_memBlocks[i].maintance();
        if(m_memBlocks[i].isFree()){
            if(!freePoolFound){
                freePoolFound = true;
                continue;
            }
            m_memBlocks[i].release();
        }
    }

    if(!freePoolFound){
        for(size_t i = 0; i < m_memBlocks.size(); i++){
            if(!m_memBlocks[i].inUse()){
                m_memBlocks[i].init(i);
                freePoolFound = true;
                break;
            }
        }
    }

    
    //step 2
    //could not create another freePool within budget
    //we should defragment the pools and move something around to fill the gaps
    if(!freePoolFound){
        int bp = 0;
        bp++;
    }

}

void FV::MemPool::maintance(void *ptr)
{
    for(size_t i = 0; i < m_memBlocks.size(); i++){
        if(m_memBlocks[i].isIn(ptr)){
            m_memBlocks[i].maintance();
        }
    }
    
}



void *FV::MemPool::place(uint64_t size){
    for(size_t i = 0; i < m_memBlocks.size(); i++){
        void *ptr = m_memBlocks[i].place(size);
        if(ptr != nullptr){
            return ptr;
        }         
    }

    //create a bigger block to fit the size
    if(size > C_DefaultBlockSize){
        uint64_t blockSize = C_DefaultBlockSize;
        while(blockSize < size){
            blockSize *=2;
        }
        for(size_t i = 0; i < m_memBlocks.size(); i++){
            if(!m_memBlocks[i].inUse()){
                if(!m_memBlocks[i].init(i, blockSize)){
                    if(!m_memBlocks[i].init(i,size)){
                        assert("Could not fit inside memPool");
                    }
                }
                return m_memBlocks[i].place(size);
            }
        }
        int bp = 0;
        bp++;
    }

    // no pointer found check if there is room for expanding the memblocks
    for(size_t i = 0; i < m_memBlocks.size(); i++){
        if(!m_memBlocks[i].inUse()){
            m_memBlocks[i].init(i);
           void *ptr = m_memBlocks[i].place(size);
        if(ptr != nullptr){
            return ptr;
        }  
        }
    }

    return nullptr;
}