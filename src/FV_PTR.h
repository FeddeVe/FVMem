#pragma once

#include <cstdint>
#include <mutex>
#include <iostream>
#include <memory>
#include <utility>
#include "FV_Mem.h"

namespace FV
{

    template <typename T>
    class PTR
    {
    public:
        PTR(T *ptr = nullptr)
        {
            std::cout << "CONSTRUCTOR" << std::endl;
            m_ptr = ptr;            
            void *headerPTR = ptr;
            headerPTR = static_cast<char*>(headerPTR) - sizeof(FV::AllocHeader);
            m_allocHeader = static_cast<FV::AllocHeader*>(headerPTR); 
        }
        // Copy constructor
        PTR(PTR<T> &sp)
        {
            // initializing shared pointer from other Shared_ptr object
            m_ptr = sp.m_ptr;
            // initializing reference counter from other shared pointer
            m_allocHeader = sp.m_allocHeader;
            m_allocHeader->incUseCount();
        }
        // reference count getter
        unsigned int use_count()
        {
           return m_allocHeader->getUseCount();
        }
        // shared pointer getter
        T *get()
        {
            return m_ptr;
        }
        // Overload * operator
        T &operator*()
        {
            return *m_ptr;
        }
        // Overload -> operator
        T *operator->()
        {
            return m_ptr;
        }
        // overloading the = operator
        void operator=(PTR sp)
        {
            // if assigned pointer points to the some other location
            if (m_ptr != sp.m_ptr)
            {
                // if shared pointer already points to some location
                if (m_ptr && m_allocHeader)
                {
                    // decrease the reference counter for the previously pointed location
                    m_allocHeader->decUseCount(); 
                    if(m_allocHeader->getUseCount() == 0){
                        delete m_ptr;
                    }
                }
                // reference new memory location
                m_ptr = sp.m_ptr;
                m_allocHeader = sp.m_allocHeader;
                // increase new memory location reference counter.
                if (m_ptr)
                {
                    m_allocHeader->incUseCount();
                }
            }
        }
        // Destructor
        ~PTR()
        {
            m_allocHeader->decUseCount();
            if(m_allocHeader->getUseCount() == 0){
                delete m_ptr;
            }        
        }

    private:
        // Reference counter
        FV::AllocHeader *m_allocHeader;
        // Shared pointer
        T *m_ptr;
    };

    template <class _Tp, typename... _Args>
    inline FV::PTR<_Tp> make_Ptr(_Args &&...__args)
    {      
      //  _Tp *ptr = new _Tp(__args...);
      //  FV::PTR fvPTR = new _Tp(__args);
      //  return FV::PTR fvptr<_Tp>(ptr);
      return FV::PTR<_Tp> ( new _Tp(__args...));
     // return ret;
    }
}