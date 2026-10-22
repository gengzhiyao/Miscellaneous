#include <memory>
#include <stdlib.h>
#include <iostream>

class B
{
public:
    void        funB( ) const {}
    std::string str;
};
class A
{
public:
    A( )
        : b( new B )
    {
    }
    void     fun( ) const {}
    const B* operator->( ) const { return b; }

private:
    B* b;
};

class MemMgr
{
public:
    MemMgr( )
        : a( new A )
    {
    }
    MemMgr( const char* nmessage )
        : message( nmessage )
    {
        std::cout << message << std::endl;
    }
    void MemAlloc( );

    const A* operator->( ) const { return a; }

private:
    const char* message;
    A*          a;
};

template <typename T>
class MyAllocator
{
public:
    T* allocate( std::size_t n ) { return static_cast<T*>( malloc( sizeof( T ) * n ) ); }
};


// --- per-class allocator ---
struct Object
{
    int  age;
    char type;
};

// 内存池中以对象管理资源
class MemPool
{
public:
    static void* operator new( std::size_t size );
    static void  operator delete( void* obj, std::size_t size );

private:
    union   
    {
        Object*  obj;   // embeded pointer
        MemPool* next;
    };

private:
    static const int BLOCK_SIZE;
    static MemPool*  m_pNextHead;   // 始终指向当前要返回的对象的下一个位置
};
