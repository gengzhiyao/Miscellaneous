#include "allocator.h"

void MemMgr::MemAlloc( )
{
    int* pInt1 = static_cast<int*>( malloc( 10 * sizeof( int ) ) );
    int* pInt2 = static_cast<int*>( calloc( 10, sizeof( int ) ) );
    std::cout << pInt1 << std::endl << pInt2 << std::endl;
    std::cout << *pInt1 << std::endl << *pInt2 << std::endl;
    int* pInt3 = static_cast<int*>( realloc( pInt2, 15 ) );
    std::cout << pInt3 << std::endl;
}

struct Const
{
    Const( char* nstring )
        : CString( nstring )
    {
    }
    char& CanChangeString( ) const { return CString[0]; }

    char* GetString( ) const { return CString; }

private:
    char* CString;
};

const int MemPool::BLOCK_SIZE = 512;
MemPool*  MemPool::m_pNextHead = nullptr;

int main1( )
{
    MemMgr mgr;
    mgr.MemAlloc( );
    mgr.operator->( )->operator->( )->funB( );    // 链式调用??
    // mgr->funB (); //这行代码编译不通过

    MyAllocator<MemMgr> myalloc;
    myalloc.allocate( 2 );

    // char *ch = "Hello"; //C++11中不合法
    const char* hello = "Hello";

    Const constChar( "String" );
    // char ch = constChar.CanChangeString ();
    // ch = 'T';
    // char& pch = constChar.CanChangeString ();
    // pch = 'A';
    // char* pc = &constChar.CanChangeString ();
    // *pc = 'E';
    // std::cout << constChar.GetString () << std::endl;

    char  arrch[] = "World";
    Const conch( arrch );
    char* tmp = &conch.CanChangeString( );
    *tmp = 'M';
    std::cout << conch.GetString( ) << std::endl;
    return 0;
}

int main( ) { return 0; }

void* MemPool::operator new( std::size_t size )
{
    if ( size != sizeof( MemPool ) ) return ::operator new( size );
    static_assert( BLOCK_SIZE >= 2, "BLOCK_SIZE must be >= 2" );
    MemPool* curObj = MemPool::m_pNextHead;

    if ( curObj )
    {
        m_pNextHead = curObj->next;
    }
    else    // 第一次进入或内存池不够时进入
    {
        MemPool* block = reinterpret_cast<MemPool*>( malloc( BLOCK_SIZE * sizeof( MemPool ) ) );  // return block = &block[0]

        for ( size_t i = 1; i < MemPool::BLOCK_SIZE - 1; i++ )
        {
            block[i].next = &block[i + 1];
        }
        block[MemPool::BLOCK_SIZE - 1].next = nullptr;  // at last or for next malloc
        MemPool::m_pNextHead = &block[1];   // pointer to next element for next usage
        curObj = block;
    }
    return curObj;
}

void MemPool::operator delete( void* obj, std::size_t size )
{
    if ( obj == nullptr ) return;
    if ( size != sizeof( MemPool ) ) return ::operator delete( obj, size );
    MemPool* deadObj = static_cast<MemPool*>( obj );
    MemPool* cur = MemPool::m_pNextHead;
    deadObj->next = cur;
    MemPool::m_pNextHead = deadObj;
}

void* IMPLEMENTATION::allocator::allocate( std::size_t objSize )
{
    Object* ptr = nullptr;
    if ( !freeStore )
    {
        std::size_t nSize = CHUNK * objSize;
        ptr = static_cast<Object*>( malloc( nSize ) );
        for ( size_t i = 0; i < CHUNK - 1; i++ )
        {
            ptr->next = ( Object* )( ( char* )ptr + objSize );
            ptr = ptr->next;
        }
        ptr->next = nullptr;    // at last
    }
    ptr = freeStore;    // next malloc
    freeStore = freeStore->next;
    return ptr;
}

void IMPLEMENTATION::allocator::deallocate( void* p, std::size_t )
{
    static_cast<Object*>( p )->next = freeStore;
    freeStore = static_cast<Object*>( p );
}

