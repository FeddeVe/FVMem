#pragma once

#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <array>
#include <cassert>
#include <iostream>

namespace FV
{
    const uint64_t C_DefaultBlockSize = 64 * 1024 * 1024;

    class MemBlockHeader
    {
    };

    class AllocHeader
    {
    public:
        AllocHeader(uint64_t size);
        ~AllocHeader();
        void incUseCount();
        void decUseCount();
        void resetUseCount();
        uint64_t getUseCount();
        uint64_t getSize();
        void setSize(uint64_t size);

    private:
        void save();
        void decompileRawData(uint16_t &useCount, uint64_t &size);
        void compileRawData(const uint16_t &useCount, const uint64_t &size);
        uint64_t m_size;
        uint64_t m_useCount; // doesn't matter in space if we use a uint8 or 16, c++ aligns space ass 3*8 so 24bytes overhead
        std::shared_mutex m_mutex;
    };

    class MemBlock
    {
    public:
        MemBlock();
        ~MemBlock();
        bool init(uint16_t index, uint64_t blockSize = C_DefaultBlockSize);
        void release();
        bool isFree();
        bool inUse() { return m_buffer != nullptr; };
        void *place(uint64_t size);
        bool isIn(void *ptr);
        void maintance();
        void displayStatus();

    private:
        uint64_t m_size;
        uint64_t m_start;
        uint64_t m_maxFreeSize;
        void *m_buffer;
        FV::AllocHeader *m_lastInsertHDR;
        std::mutex m_maintanceMutex;
        uint16_t m_index;
    };

    class MemPool
    {
    public:
        MemPool(const MemPool &) = delete;
        MemPool &operator=(const MemPool &) = delete;

        static MemPool *getInstance()
        {
            if (!instance)
            {
                //creating insance the old fashion way. new just not works at the moment.
                instance = new FV::MemPool();
            }
            return instance;
        }

        static void *operator new(std::size_t size){
            return malloc(size);
        }

        static void operator delete(void *ptr){
            return free(ptr);
        }

        void init(uint32_t maxGB);
        void *place(uint64_t size);

        // call it reguarly to do some work outside the new and delete operators
        // this will speed up the new and delete operators, but the maintance need to be done somewhere
        void maintance();

        // do maintance only for block pointer is in
        void maintance(void *ptr);
        bool isInit() const { return isInitialized; };

    private:
        inline static MemPool *instance{nullptr};
        MemPool();
        ~MemPool();
        std::array<FV::MemBlock, 256> m_memBlocks;
        bool isInitialized;
    };
}

inline void *operator new(size_t size)
{

    if(size == 0){
        size++;
    }
    //  std::cout << "New operator overloading " << std::endl;
    size = size + sizeof(FV::AllocHeader); 
    void *ptr = FV::MemPool::getInstance()->place(size);
    if(ptr != nullptr){
        return ptr;
    }
    throw std::bad_alloc{}; // required by [new.delete.single]/3
}

inline void *operator new[](size_t size){
    if(size == 0){
        size++;
    }
    //  std::cout << "New operator overloading " << std::endl;
    size = size + sizeof(FV::AllocHeader); 
    void *ptr = FV::MemPool::getInstance()->place(size);
    if(ptr != nullptr){
        return ptr;
    }
    throw std::bad_alloc{}; // required by [new.delete.single]/3 
}

inline void operator delete(void *p) noexcept
{
    //  std::cout << "Delete operator overloading " << std::endl;

    void *hdrPTR = static_cast<char *>(p) - sizeof(FV::AllocHeader);
    FV::AllocHeader *hdr = static_cast<FV::AllocHeader *>(hdrPTR);
    hdr->resetUseCount();
    FV::MemPool::getInstance()->maintance(p);
    // free(p);
}

inline void operator delete(void *p, size_t size) noexcept
{
    void *hdrPTR = static_cast<char *>(p) - sizeof(FV::AllocHeader);
    FV::AllocHeader *hdr = static_cast<FV::AllocHeader *>(hdrPTR);
    hdr->resetUseCount();
    FV::MemPool::getInstance()->maintance(p);
}

inline void operator delete[](void* p) noexcept
{
    void *hdrPTR = static_cast<char *>(p) - sizeof(FV::AllocHeader);
    FV::AllocHeader *hdr = static_cast<FV::AllocHeader *>(hdrPTR);
    hdr->resetUseCount();
    FV::MemPool::getInstance()->maintance(p);
}

inline void operator delete[](void* p, std::size_t size) noexcept
{
    void *hdrPTR = static_cast<char *>(p) - sizeof(FV::AllocHeader);
    FV::AllocHeader *hdr = static_cast<FV::AllocHeader *>(hdrPTR);
    hdr->resetUseCount();
    FV::MemPool::getInstance()->maintance(p);
}

 