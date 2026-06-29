#include "sfinae.h"
#include <memory>
#include <assert.h>
#include <type_traits>
#ifdef _WIN32
#include <windows.h>
#endif
struct WithValueType
{
    using value_type = int; // 嵌套类型别名，属于类而不属于对象
};

// NOTE - 数组形式的模板实参推断，这样是不对的，C++ 中，固定大小数组的引用 必须明确指定数组的大小，除非用模板参数推导大小
//  template<typename T>
//  void ArrFunction (const T (&) [])
//  {
//  }

// 这种写法才是正确的
template <typename T, std::size_t N>
void ArrFunction(const T (&)[N])
{
}

// template <typename T>
// void RemoveFloatPointer(T value,std::enable_if_t<std::is_same_v<T, float *>, T> = 0)
// {
//     std::cout << "这是float的重载形式" << std::endl;
// }

template <typename T>
void RemoveFloatPointer(std::enable_if_t<std::is_same_v<T, float *>, T> = 0)
{
    std::cout << "这是float的重载形式" << std::endl;
}

// template<>
// void RemoveFloatPointer( )
// {
//     std::cout << "这是采用的int形式" << std::endl;
// }

int main()
{
    #ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    #endif
    int a = 5;
    print_type(a);
    std::unique_ptr<float> b = std::make_unique<float>(2.4f);
    float *c = new float(2.4f);
    print_type(c);
    print_type(b.get());

    const char *string = "Hello C-style string.";
    print_type(string);

    // 编译器认为float*类型的模板
    double *d = new double{2.3};
    print_type(d);
    WithFooClass fooClass;
    call_foo(fooClass);

    TypePrinter<WithValueType, std::true_type>::print();

    int arr[] = {2, 2, 2, 2};
    ArrFunction(arr);

    // int (&) arrRef[4] = arr;    //ERROR - 错误写法
    int (&arrRef)[4] = arr;

    // static_assert(std::is_lvalue_reference<arrRef>::value);
    static_assert(std::is_lvalue_reference<decltype(arrRef)>::value);
    static_assert(std::is_lvalue_reference_v<decltype((a))>);

    // const float* const& refc = c;
    float *f = new float;
    float aa = 0;
    RemoveFloatPointer<float*>(f);
    return 0;
}