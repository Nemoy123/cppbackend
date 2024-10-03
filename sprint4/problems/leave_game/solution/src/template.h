#pragma once
#include <mutex>
#include <string>


template <typename T>
class Singleton
{   
public:
    static T& getInstance() {
        static T instance;
        return instance;
    }

protected:
    Singleton() = default;
    ~Singleton() = default;
    Singleton(Singleton &other) = delete;
    void operator=(const Singleton &) = delete;

    
};

