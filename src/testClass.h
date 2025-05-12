#pragma once
#include <cstdint>

class TestClass{
    public:
        TestClass(uint64_t val2){
            testval2 = val2;
            testval = 1234;
        };
        ~TestClass(){};
    private:
        uint64_t testval;
        uint64_t testval2;


};