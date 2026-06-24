#include <memory>
#include <stdlib.h>
#include <iostream>

class B
{
public:
    void funB () const
    {

    }
    std::string str;
};
class A
{
public:
    A () :b (new B) { }
    void fun () const
    {

    }
    const B* operator->() const
    {
        return b;
    }
private:
    B* b;
};



class MemMgr {
public:
    MemMgr () :a(new A){}
    MemMgr (const char* nmessage)
        :message (nmessage)
    {
        std::cout << message << std::endl;
    }
    void MemAlloc ();

    const A* operator->() const
    {
        return a;
    }

private:
    const char* message;
    A* a;
};



template <typename T>
class MyAllocator {
public:
    T* allocate (std::size_t n)
    {
        return static_cast< T* >(malloc (sizeof (T) * n));
    }
};