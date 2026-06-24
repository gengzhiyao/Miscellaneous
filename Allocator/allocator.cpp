#include "allocator.h"

void MemMgr::MemAlloc ()
{
    int* pInt1 = static_cast< int* >(malloc (10 * sizeof (int)));
    int* pInt2 = static_cast< int* >(calloc (10 , sizeof (int)));
    std::cout << pInt1 << std::endl << pInt2 << std::endl;
    std::cout << *pInt1 << std::endl << *pInt2 << std::endl;
    int* pInt3 = static_cast< int* >(realloc (pInt2 , 15));
    std::cout << pInt3 << std::endl;
}


struct Const
{
    Const (char* nstring)
        :CString (nstring)
    {
    }
    char& CanChangeString () const
    {
        return CString[0];
    }

    char* GetString () const
    {
        return CString;
    }

private:
    char* CString;
};

int main1 ()
{
    MemMgr mgr;
    mgr.MemAlloc ();
    mgr.operator->()->operator->()->funB ();    //链式调用??
    //mgr->funB (); //这行代码编译不通过

    MyAllocator<MemMgr> myalloc;
    myalloc.allocate (2);

    //char *ch = "Hello"; //C++11中不合法
    const char* hello = "Hello";

    Const constChar("String");
    // char ch = constChar.CanChangeString ();
    // ch = 'T';
    // char& pch = constChar.CanChangeString ();
    // pch = 'A';
    // char* pc = &constChar.CanChangeString ();
    // *pc = 'E';
    // std::cout << constChar.GetString () << std::endl;

    char arrch [] = "World";
    Const conch (arrch);
    char* tmp = &conch.CanChangeString ();
    *tmp = 'M';
    std::cout << conch.GetString () << std::endl;
    return 0;
}

