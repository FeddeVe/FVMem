#include <iostream>
#include <memory>
#include <vector>
#include <chrono>
#include "src/FV_PTR.h"
#include "src/testClass.h"
#include "src/FV_Mem.h"

double randomFloat()
{
    return (double)(rand()) / (float)(RAND_MAX);
}

int main()
{
    std::cout << "HELLO FV MEMORY SHIZZLE" << std::endl;
    srand(time(0));

    size_t i = sizeof(FV::AllocHeader);
    // FV::MemPool memPool(16);
    memPool.init(16);

    uint64_t vectorSize = 4096 * 4096;
    std::vector<uint64_t> randomPoints;
    for (uint64_t i = 0; i < vectorSize / 4; i++)
    {
        double rdm = randomFloat();
        randomPoints.push_back(rdm * (vectorSize - 1));
    }

    
    memPool.maintance();

    std::vector<TestClass *> rawPTRS;
    rawPTRS.resize(vectorSize);

    auto t0 = std::chrono::high_resolution_clock::now();
    {
        std::vector<uint64_t>c_random = randomPoints;
        auto t1 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < rawPTRS.size(); i++)
        {
            rawPTRS[i] = new TestClass(1);
            if(rawPTRS[i] == nullptr){
                int bp = 0;
                bp++;
            }
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        memPool.maintance();
        auto t3 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < randomPoints.size(); i++)
        {
            if(rawPTRS[randomPoints[i]]){           
                    delete rawPTRS[randomPoints[i]];
                    rawPTRS[randomPoints[i]] = nullptr;
            }
                 
        }
        auto t4 = std::chrono::high_resolution_clock::now();
        memPool.maintance();
        auto t5 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < rawPTRS.size(); i++)
        {
            if (!rawPTRS[i])
            {
                rawPTRS[i] = new TestClass(2);
            }
        }
        auto t6 = std::chrono::high_resolution_clock::now();
        memPool.maintance();
        auto t7 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < rawPTRS.size(); i++)
        {
            delete rawPTRS[i];
        }
        rawPTRS.clear();
        auto t8 = std::chrono::high_resolution_clock::now();
        std::cout << "=== RawPointers Result: " << std::endl;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
        std::cout << " - Filling Table: " << ms.count() << std::endl;
        ms = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2);
        std::cout << " - MemPool maintance: " << ms.count() << std::endl;
        ms = std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3);
        std::cout << " - Deleting Random Points: " << ms.count() << std::endl;
        ms = std::chrono::duration_cast<std::chrono::milliseconds>(t5 - t4);
        std::cout << " - MemPool maintance: " << ms.count() << std::endl;
        ms = std::chrono::duration_cast<std::chrono::milliseconds>(t6 - t5);
        std::cout << " - Filling random points: " << ms.count() << std::endl;
        ms = std::chrono::duration_cast<std::chrono::milliseconds>(t7 - t6);
        std::cout << " - MemPool maintance: " << ms.count() << std::endl;
        ms = std::chrono::duration_cast<std::chrono::milliseconds>(t8 - t7);
        std::cout << " - Delete All: " << ms.count() << std::endl;
    }
    auto t9 = std::chrono::high_resolution_clock::now();
    memPool.maintance();
    auto t10 = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t9 - t0);
    std::cout << " - Total Raw Pointer Time: " << ms.count() << std::endl;
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(t10-t9);
    std::cout << " - Mempool Maintance:" << ms.count() <<std::endl;

    TestClass *testPTR = new TestClass(123);
    memPool.place(1000);
    memPool.place(1000);
    delete testPTR;

    std::shared_ptr<TestClass> shared_ptr_test = std::make_shared<TestClass>(85);
    FV::PTR<TestClass> makeTest = FV::make_Ptr<TestClass>(01);
    FV::PTR<TestClass> test(new TestClass(23));

    return 0;
}