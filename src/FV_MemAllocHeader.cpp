#include "FV_Mem.h"

FV::AllocHeader::AllocHeader(uint64_t size)
   : m_useCount{0}, m_size{size}
{ 
    m_dbg[0] = 'a';
    m_dbg[1] = 'l';
    m_dbg[2] = 'l';
    m_dbg[3] = 'h';
    m_dbg[4] = 'e';
    m_dbg[5] = 'a';
    m_dbg[6] = 'd';
    m_dbg[7] = '0';
}

FV::AllocHeader::~AllocHeader(){}

void FV::AllocHeader::incUseCount(){
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_useCount >= UINT8_MAX){
        assert("USECOUNT CANNOT BE GREATER THAN");
    }
    m_useCount++;
}

void FV::AllocHeader::decUseCount(){
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_useCount == 0){
        assert("USECOUNT IS ALREADY ZERO, SOMEONE FUCKED UP");
    }
    m_useCount--;
}

void FV::AllocHeader::resetUseCount()
{
     std::lock_guard<std::mutex> lock(m_mutex); 
    m_useCount = 0;
}

uint64_t FV::AllocHeader::getUseCount()
{
     std::lock_guard<std::mutex> lock(m_mutex);
     return m_useCount;
}

void FV::AllocHeader::setSize(uint64_t size){
    std::lock_guard<std::mutex> lock(m_mutex);
    m_size = size;
}

uint64_t FV::AllocHeader::getSize(){
  //  std::lock_guard<std::mutex> lock(m_mutex);
    return m_size;
}
 