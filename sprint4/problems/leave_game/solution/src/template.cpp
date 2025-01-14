// #include "template.h"

// /**
//  * Static methods should be defined outside the class.
//  */

// Singleton* Singleton::pinstance_{nullptr};
// std::mutex Singleton::mutex_;


// /**
//  * The first time we call GetInstance we will lock the storage location
//  *      and then we make sure again that the variable is null and then we
//  *      set the value. RU:
//  */
// Singleton* Singleton::GetInstance()
// {
//     std::lock_guard<std::mutex> lock(mutex_);
//     if (pinstance_ == nullptr)
//     {
//         pinstance_ = new Singleton();
//     }
//     return pinstance_;
// }