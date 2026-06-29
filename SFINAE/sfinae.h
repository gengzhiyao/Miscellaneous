
#ifndef MODERNCPP_SFINAE_H
#define MODERNCPP_SFINAE_H

// ~~~~~~~~~~~~~~~~~~~~~~~~~SFINAE~~~~~~~~~~~~~~~~~~~~~~~~~~
// 编译器在模板实例化时，如果遇到匹配失败-> 无法按照此方法实例化
// 不会马上报错，而是会继续匹配其他模板实现，努力匹配到一个能够实例化的实现

#include <iostream>
#include <type_traits>

/* ---------------------------------------------------------------------------------------------- */
/*                                      SFINAE关键点之enable_if实现                                 */
/* ---------------------------------------------------------------------------------------------- */
///NOTE - 自定义enable_if，模拟底层实现 引入命名空间，防止污染std
namespace Analysis
{
    template<bool Condition , typename T>
    struct enable_if
    {
        //Empty
    };

    // @brief 当Condition为真时，走的是这条路线，将类型T使用type透传下来
    template<typename T>
    struct enable_if<true,T>
    {
        using type = T; //在类（包括类模板）内部，using 别名 = 类型 是 嵌套类型别名声明，作用是：给某个类型起一个 属于当前类的别名，这个别名的归属是类本身，而非类的实例（对象）。
    };

    template<typename T>
    void someFun (typename enable_if<(sizeof (T) <= sizeof (void*)) , T>::type t)
    {

    }

    template<typename T>
    void someFun (typename enable_if<(sizeof (T) > sizeof (void*)) , T>::type& t)
    {

    }
    
    template<typename T>
    struct is_less_than_pointer
    {
        constexpr static bool value = sizeof (T) <= sizeof (void*);
    };

    template<typename T>
    void someFunSimplified (typename enable_if<is_less_than_pointer<T>::value , T>::type t)
    {
        
    }


}

/*------------------------------------------------------------------------------------------------*/

template<typename  T>
// 若condition为true时，type返回的是void类型
// typename std::enable_if<condition,void>::type;
// std::is_integral<T>::value value获取得到是true还是false
typename std::enable_if<std::is_integral<T>::value , void>::type
print_type (T value)
{
    std::cout << "Integral: " << value << std::endl;
}

//只在float*类型中有用
template<typename  T>
typename std::enable_if<std::is_floating_point<T>::value , void>::type
print_type (T* value)
{
    std::cout << "Float Point: " << *value << std::endl;
}

//只在C风格的字符串有效
template<typename  T>
typename std::enable_if<std::is_same<T , const char*>::value || std::is_same<T , char*>::value , void>::type
print_type (T value)
{
    std::cout << "C-style String: " << value << std::endl;
}

//ANCHOR - 不会进来
//判断指针类型，但是不能是char*或者const char*类型
template<typename T>
typename std::enable_if<std::is_pointer<T>::value && !std::is_same<T , const char*>::value && !std::is_same<T , char*>::value , void>::type
print_type (T value)
{
    std::cout << "Other Point: " << *value << std::endl;
}

template<typename T>
class has_foo
{
private:
    typedef char yes[1];
    typedef char no[2];

    template<typename U , void (U::*)() /* const */>
    struct SFINAE
    {

    };

    template<typename U>
    static yes& test (SFINAE<U , &U::foo>*);    //运行期间不会调用，sizeof()返回的是函数返回值的大小

    template<typename U>
    static no& test (...);

public:
    static constexpr bool value = sizeof (test<T> (nullptr)) == sizeof (yes);   //通过SFINAE技术，使编译器优先匹配指针类型的模板

};

template<typename T>
typename std::enable_if<has_foo<T>::value , void>::type
call_foo (T& obj)
{
    obj.foo ();//实现安全检测的调用
    // std::cout << "foo called" << std::endl;
}

class WithFooClass
{
public:
    void foo () /* const */ //NOTE - 非const类型才能处理，因为检测中并没有要求检测是否是const member function
    {
        std::cout << "foo" << std::endl;
    }
};

class WithoutFooClass
{

};

template<typename T,typename Enable=void>
struct TypePrinter
{
    
};

template<typename T>
//typename T::value_type 表示类型T所存储的类型，如vector<int>::value_type为int
struct TypePrinter<T , typename std::enable_if<!std::is_void<typename T::value_type>::value,std::true_type>::type>
{
    static void print ()
    {
        std::cout << "T has a member type value_type" << std::endl;
    }
};

template<typename T>
struct TypePrinter<T , void>
{
    static void print ()
    {
        std::cout << "T dose not have a member type value_type" << std::endl;
    }
};



#endif //MODERNCPP_SFINAE_H