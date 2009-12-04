#include <iostream>
#include "mydt.hpp"

class A {
public:
    A(int _x, int _y): x(_x), y(_y) {};
    int x;
    int y;
};

class Functor {
public:
    void operator()(A * a1, A * a2) {
        std::cout << "Do " << a1->x << ", " << a1->y << " TO " << a2->x << ", " << a2->y << std::endl;
    }
};

int main() {
    MyDT<A, Functor> dt;
    Functor f;
    A a1(1, 2), a2(1, 2), a3(1, 2), a4(1, 2);
    dt.begin(f);
    dt.next(a1.x, a1.y, &a1);
    dt.next(a2.x, a2.y, &a2);
    dt.next(a3.x, a3.y, &a3);
    dt.next(a4.x, a4.y, &a4);
    dt.end();
 
    return 0;
}