#include <iostream>
#include <memory>
#include <functional>
#include <vector>
#include <string>
#include <stdexcept>
#include "SmartPointer.h"

void myCustomDel (A *a);
void DecltypeCustom (A *a);

int main ()
{
    // int* i = new int (10);
    // std::shared_ptr<int> pi = std::make_shared<int> (11);
    // std::shared_ptr<A> a1 = std::make_shared<A> ("std::string");
    // std::shared_ptr<A> a2 (new A ("arguments"));
    // a2 = a1;
    // A* a3 = a2.get ();
    //!delete a3; 不要手动删除指针指针返回的裸指针，否则会报异常
    // std::weak_ptr<A> weak (a2);

    //在循环引用中，每个对象的引用计数依赖于另一个对象的释放，而另一个对象的释放又依赖于当前对象的释放，形成一个无法打破的闭环。
    // std::shared_ptr<A> loopRefa = std::make_shared<A> ();
    // std::shared_ptr<B> loopRefb = std::make_shared<B> ();
    // loopRefa->m_b = loopRefb;
    // loopRefb->m_a = loopRefa;   //B::std::weak_ptr<A> m_a;不增加引用计数
    //离开作用域之后，由于loopRefa的引用计数置零，A的对象被销毁，进一步使得shared_ptr::m_b指针成员被销毁
    //loopRefb的引用计数减至1，但是到达作用域又得减一次，基本可以看作是在极短的时间内减两次，从而引用计数归零，内存释放。

    std::shared_ptr<A> moveA1 = std::make_shared<A> ();
    std::shared_ptr<A> moveA2 = std::make_shared<A> ();
    std::shared_ptr<A> moveA3 = std::make_shared<A> ("moveA3");
    moveA1 = moveA2;
    moveA1 = moveA3;
    moveA1 = std::move (moveA2);    //执行这行触发的析构函数是由于moveA1接收一个对象，释放原本对象产生的

    //std::shared_ptr<A> invalidA = new A ("..");
    std::shared_ptr<A> validA (new A ("validA"));
    //std::move (moveA2);
    std::cout << validA << std::endl;

    std::auto_ptr<C> autoPtrC1 (new C); //auto_ptr在VS2017中被移除，但在GCC中，目前还仍被保留
    std::auto_ptr<C> autoPtrC2 (autoPtrC1);
    if (autoPtrC1.get () == NULL)
    {
        //对于UTF8编码的文件，若终端中显示的字体是乱码，那么在windows的时间设置中，将编码方式转换为Unicode编码
        std::cout << std::string ("autoptr是空指针。") << std::endl;
    }
    C *c = autoPtrC2.release ();
    if (autoPtrC2.get()==nullptr)
    {
        std::cout << "autoPtrC2为空" << std::endl;
    }

    std::unique_ptr<A> uniqueA1 (new A ("use unique pointer"));

    std::unique_ptr<A , decltype(myCustomDel) *> (new A ("custom deleter object") , DecltypeCustom);
    decltype((myCustomDel)) fun = DecltypeCustom;//函数引用，一旦绑定就不可更改

    return 0;
}

void myCustomDel (A* a)
{
    std::cout << "执行自定义删除器" << std::endl;
    delete a;
}

void DecltypeCustom (A *a)
{
    std::cout << "执行自定义删除器:DecltypeCustom" << std::endl;
    delete a;
}


/* ---------------------------------------------------------------------------------------------- */
/*                                           C++ Primer                                           */
/* ---------------------------------------------------------------------------------------------- */

/**
 *@brief StrBlobPtr和StrBlob之间的关系并非观察者模式，更可以看作是迭代器模式和代理模式。
 * StrBlobPtr 类似于一个智能指针，它指向 StrBlob 中的元素。
 * 更具体地说，StrBlobPtr 内部使用一个弱指针（std::weak_ptr）来指向 StrBlob 所管理的底层 vector（即 StrBlob 的 data 成员）。
 * 这样，StrBlobPtr 不会影响 StrBlob 的引用计数，从而可以避免循环引用。
 * StrBlobPtr 设计用来遍历 StrBlob 中的元素，它类似于一个迭代器。
 * StrBlob 提供了 begin() 和 end() 成员函数，分别返回指向第一个元素和最后一个元素之后位置的 StrBlobPtr 对象。
 * 因此，我们可以使用 StrBlobPtr 来遍历 StrBlob 中的所有元素。
 */

// 提前声明，因为StrBlobPtr需要访问StrBlob的私有成员
class StrBlobPtr;

// StrBlob类：管理string的动态数组
class StrBlob
{
    friend class StrBlobPtr;  // 允许StrBlobPtr访问其私有成员
public:
    using size_type = std::vector<std::string>::size_type;

    // 构造函数
    StrBlob () : data (std::make_shared<std::vector<std::string>> ()) { }
    StrBlob (std::initializer_list<std::string> il)
        : data (std::make_shared<std::vector<std::string>> (il))
    {
    }

    // 元素数量
    size_type size () const { return data->size (); }
    bool empty () const { return data->empty (); }

    // 添加/删除元素
    void push_back (const std::string &t) { data->push_back (t); }
    void pop_back ()
    {
        check (0 , "pop_back on empty StrBlob");
        data->pop_back ();
    }

    // 访问元素
    std::string &front ()
    {
        check (0 , "front on empty StrBlob");
        return data->front ();
    }
    const std::string &front () const
    {
        check (0 , "front on empty StrBlob");
        return data->front ();
    }
    std::string &back ()
    {
        check (0 , "back on empty StrBlob");
        return data->back ();
    }
    const std::string &back () const
    {
        check (0 , "back on empty StrBlob");
        return data->back ();
    }

    // 返回指向首元素/尾后位置的StrBlobPtr
    StrBlobPtr begin ();
    StrBlobPtr end ();

private:
    // 管理底层vector的智能指针
    std::shared_ptr<std::vector<std::string>> data;

    // 检查下标是否合法，非法则抛出异常
    void check (size_type i , const std::string &msg) const
    {
        if (i >= data->size ())
        {
            throw std::out_of_range (msg);
        }
    }
};


// StrBlobPtr类：StrBlob的智能指针/迭代器
class StrBlobPtr
{
public:
    // 构造函数
    StrBlobPtr () : curr (0) { }
    StrBlobPtr (StrBlob &a , size_t sz = 0)
        : wptr (a.data) , curr (sz)
    {
    }

    // 解引用：返回当前位置的string引用
    std::string &deref () const
    {
        auto p = check (curr , "dereference past end");
        return (*p)[curr];
    }

    // 前缀递增：移动到下一个元素，返回自身引用
    StrBlobPtr &incr ()
    {
        // 先检查当前位置是否合法
        check (curr , "increment past end of StrBlobPtr");
        ++curr;
        return *this;
    }

    // 重载==运算符，用于判断是否到达end
    bool operator==(const StrBlobPtr &other) const
    {
        return wptr.lock () == other.wptr.lock () && curr == other.curr;
    }
    bool operator!=(const StrBlobPtr &other) const
    {
        return !(*this == other);
    }

private:
    // 检查当前位置是否合法，返回指向vector的shared_ptr（若合法）
    std::shared_ptr<std::vector<std::string>>
        check (size_t i , const std::string &msg) const
    {
        auto ret = wptr.lock ();  // 尝试获取shared_ptr（判断vector是否存在）
        if (!ret)
        {
            throw std::runtime_error ("unbound StrBlobPtr");
        }
        if (i >= ret->size ())
        {
            throw std::out_of_range (msg);
        }
        return ret;  // 返回shared_ptr，延长vector的生命周期
    }

    // 指向StrBlob底层vector的弱指针，该弱指针只负责观察StrBlob类中的数据是否有效，但不增加引用计数。
    // 这意味着，作为友元类，不影响StrBlob中数据成员data的生命周期，只是提供一个更安全的方式，去访问其中的数据元素。
    std::weak_ptr<std::vector<std::string>> wptr;
    // 当前元素的下标
    size_t curr;
};


// 实现StrBlob的begin和end函数
StrBlobPtr StrBlob::begin ()
{
    return StrBlobPtr (*this);
}
StrBlobPtr StrBlob::end ()
{
    return StrBlobPtr (*this , data->size ());
}


// 测试主函数
int main2 ()
{
    try
    {
        // 创建StrBlob对象并添加元素
        StrBlob sb{ "Hello", "StrBlob", "and", "StrBlobPtr" };
        std::cout << "StrBlob的大小：" << sb.size () << std::endl;
        std::cout << "StrBlob的首元素：" << sb.front () << std::endl;
        std::cout << "StrBlob的尾元素：" << sb.back () << std::endl;

        // 用StrBlobPtr遍历StrBlob
        std::cout << "\n遍历StrBlob中的元素：" << std::endl;
        StrBlobPtr p = sb.begin ();
        while (p != sb.end ())
        {
            std::cout << p.deref () << " ";
            p.incr ();
        }
        std::cout << std::endl;

        // 测试异常场景（越界访问）
        std::cout << "\n测试越界访问：" << std::endl;
        StrBlobPtr p2 (sb , 10);  // 初始位置超过元素数量
        p2.deref ();  // 会抛出out_of_range异常
    }
    catch (const std::exception &e)
    {
        std::cerr << "异常：" << e.what () << std::endl;
    }

    return 0;
}