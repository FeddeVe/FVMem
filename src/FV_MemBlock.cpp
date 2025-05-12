#include "FV_Mem.h"

FV::MemBlock::MemBlock()
    : m_size{0}, m_buffer{nullptr}, m_lastInsertHDR{nullptr}
{
}

FV::MemBlock::~MemBlock()
{
    std::free(m_buffer);
}

bool FV::MemBlock::init(uint16_t index, uint64_t blockSize)
{
    m_index = index;
    m_size = blockSize;
    m_buffer = std::malloc(m_size);
    /*
    while (m_buffer == nullptr)
    {
        m_size = m_size / 2;
        if (m_size < 1024 * 1024)
        {
            m_size = 0;
            return false;
        }
        m_buffer = std::malloc(m_size);
    }
        */
       if(m_buffer==nullptr){
        return false;
       }
    m_start = reinterpret_cast<uintptr_t>(&m_buffer);
    m_maxFreeSize = m_size;
    FV::AllocHeader *hdr = new (m_buffer) AllocHeader(m_maxFreeSize);
    m_lastInsertHDR = hdr;
    return true;
}

void FV::MemBlock::release()
{
    std::cout << "---- Memmory Releasing ----" << std::endl;
    displayStatus();
    std::free(m_buffer);
    m_buffer = nullptr;
}

bool FV::MemBlock::isFree()
{
    if (m_buffer == nullptr)
    {
        return false;
    }
    FV::AllocHeader *hdr = static_cast<FV::AllocHeader *>(m_buffer);
    if (hdr->getSize() == m_size)
    {
        return true;
    }
    return false;
}

void *FV::MemBlock::place(uint64_t size)
{

    // if maintance is running, we will not wait, but go to the next block
    std::unique_lock<std::mutex> lock(m_maintanceMutex, std::try_to_lock);
    if (!lock.owns_lock())
    {
        return nullptr;
    }
    //  displayStatus();
    void *retPTR = nullptr;

    if (size <= (m_maxFreeSize - sizeof(FV::AllocHeader)))
    {
        uint64_t index = m_start;
        void *tmpPTR = m_lastInsertHDR;
        if(tmpPTR == nullptr){
            tmpPTR = m_buffer;
        }
        void *end = static_cast<char *>(m_buffer) + m_size;
        while (tmpPTR < end)
        {
            FV::AllocHeader *hdr = static_cast<FV::AllocHeader *>(tmpPTR);
            if ((hdr->getUseCount() == 0) && (hdr->getSize() >= size))
            {
                retPTR = static_cast<char *>(tmpPTR) + sizeof(FV::AllocHeader);
                // creating a new HDR for the next block
                void *nextHdrPtr = static_cast<char *>(tmpPTR) + size; 

                uint64_t sizeAvail = hdr->getSize() - size;
                if (sizeAvail < sizeof(FV::AllocHeader))
                {
                    m_lastInsertHDR = nullptr;
                    size = hdr->getSize();
                    int bp = 0;
                    bp++;
                }
                else
                {
                    FV::AllocHeader *nextHDR = new (nextHdrPtr) FV::AllocHeader(hdr->getSize() - size);
                    if (hdr->getSize() == m_maxFreeSize)
                    {
                        m_maxFreeSize = nextHDR->getSize();
                    }
                    m_lastInsertHDR = nextHDR;
                }

                hdr->incUseCount();
                hdr->setSize(size);

                displayStatus();
                return retPTR;
                // return tmp_hdr + sizeofAllocHeader;
                int bp = 0;
                bp++;
                break;
            }
            tmpPTR = static_cast<char *>(tmpPTR) + hdr->getSize();
        }
    }
    return nullptr;
}

bool FV::MemBlock::isIn(void *ptr)
{
    void *endPTR = static_cast<char *>(m_buffer) + m_size;
    if ((ptr > m_buffer) && (ptr < endPTR))
    {
        return true;
    }
    return false;
}

void FV::MemBlock::maintance()
{

    std::lock_guard<std::mutex> lock(m_maintanceMutex);
    // Combining free blocks with usecount 0;
    // first get a block with usecount 0;
    // then get the next block, if usecount == 0, add the size to the freesize else break the inner loop

    m_lastInsertHDR = nullptr;
    uint64_t maxFreeSize = 0;
    void *tmpPTR = m_buffer;
    void *endPTR = static_cast<char *>(m_buffer) + m_size;
    while (tmpPTR < endPTR)
    {
        FV::AllocHeader *hdr = static_cast<FV::AllocHeader *>(tmpPTR);

        if (hdr->getUseCount() == 0)
        {
            if(!m_lastInsertHDR){
                m_lastInsertHDR = hdr;
            }
            uint64_t freeSize = hdr->getSize();
            tmpPTR = static_cast<char *>(tmpPTR) + hdr->getSize();
            while (tmpPTR < endPTR)
            {
                FV::AllocHeader *sHdr = static_cast<FV::AllocHeader *>(tmpPTR);
                if (sHdr->getUseCount() == 0)
                {
                    freeSize += sHdr->getSize();
                }
                else
                {
                    break;
                }
                if (sHdr->getSize() == 0)
                {
                    return;
                }
                tmpPTR = static_cast<char *>(tmpPTR) + sHdr->getSize();
            }
            hdr->setSize(freeSize);
            if (freeSize > maxFreeSize)
            {
                if (freeSize == 0)
                {
                    break;
                }
                maxFreeSize = freeSize;
            }
        }
        else
        {
            tmpPTR = static_cast<char *>(tmpPTR) + hdr->getSize();
        }
    }
    m_maxFreeSize = maxFreeSize;
    displayStatus();
}

void FV::MemBlock::displayStatus()
{
    /*
        std::cout << "---- MEMBLOCK STATUS REPORT : " << m_index << std::endl;
    std::cout << "---- Size: "<< m_size << std::endl;
    void *tmpPTR = m_buffer;
    void *endPTR = static_cast<char *>(m_buffer) + m_size;
    while (tmpPTR < endPTR)
    {
        FV::AllocHeader *hdr = static_cast<FV::AllocHeader *>(tmpPTR);
        std::cout << "- HDR :" << tmpPTR << " - UseCount: " << hdr->getUseCount() << " - Size: " << hdr->getSize() << std::endl;
        tmpPTR = static_cast<char *>(tmpPTR) + hdr->getSize();
    }
    std::cout << "---- END OF MEMBLOCK STATUS REPORT : " << std::endl;
    	*/
}
