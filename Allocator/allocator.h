#include <memory>
#include <stdlib.h>
#include <iostream>
#include <memory>
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
// 为什么需要内存池，或者说，为什么工业中，使用内存池的场景比较多
// 1. 效率问题，不用频繁的申请和释放内存
// 2. 对象连续存储，避免小内存块
// 3. operator new[] 会在顶部或者底部生成 cookie 记录所分配的内存
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

namespace IMPLEMENTATION
{
class allocator
{
    // usage: 
    // DECLARE static allocator alloc; IN YOUR CLASS
    // USE alloc.allocate(size) IN YOUR CUSTOM OPERATOR NEW(STD::SIZE_T size)
private:
    struct Object
    {
        Object* next;
    };

public:
    void* allocate( std::size_t );
    void  deallocate( void*, std::size_t );

private:
    Object* freeStore = nullptr;    // head
    static const int CHUNK = 5;
};
}    // namespace IMPLEMENTATION