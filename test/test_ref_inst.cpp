#include <iostream>
#include <memory>

class A {
    public:
        int prop;
        A() {prop = 1;}
};

class B {
    public:
        A& mem;
        B(A& in_mem): mem(in_mem) {}
};

int main() {
    A a;
    auto* mem_a = &a;
    B b(a);
    
    std::cout << "a's address:\t\t" << &a << "\n";
    std::cout << "copied a's address:\t" << mem_a << "\n";
    std::cout << "b's address:\t\t" << &b << "\n";
    std::cout << "a's address from b:\t" << &(b.mem) << "\n";
    std::cout << "a's property:\t\t" << a.prop << "\n";
    std::cout << "modifying b's a's property...\n";
    b.mem.prop = 2;
    std::cout << "a's new property:\t" << a.prop << "\n";

    return 0;
}