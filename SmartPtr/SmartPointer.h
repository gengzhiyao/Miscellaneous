#include <iostream>
#include <memory>

class B;
class A
{
public:
    A () { std::cout << "A::ctor" << std::endl; };
    A (std::string str) { std::cout << str << std::endl; };
    ~A () { std::cout << "A::dtor" << std::endl; };
    A (const A &other)
    {
        std::cout << "A::copyCtor" << std::endl;
    }
    A (A &&other)
    {
        std::cout << "A::moveCtor" << std::endl;
    }

public:
    std::shared_ptr<B> m_b; 
    //std::weak_ptr<B> m_b;
};

class B
{
public:
    //std::shared_ptr<A> m_a;
    std::weak_ptr<A> m_a;
    B () { std::cout << "B::ctor" << std::endl; };
    ~B () { std::cout << "B::dtor" << std::endl; };

};

class C
{
public:
};
