#include <iostream>

#include "threadpool.h"


struct TestTp
{
    TestTp(int a, int b) : a_(a), b_(b) {}
    int operator()() const { return a_ + b_; }

    int a_;
    int b_;
};


int main(int, char**){
    std::cout << "Hello, from threadpool!\n";

    ThreadPool tp(4);

    auto f1 = tp.submit([](int a, int b){ return a + b; }, 1, 2);
    auto f2 = tp.submit([](int a, int b){ return a + b; }, 3, 4);

    std::cout << f1.get() << std::endl;
    std::cout << f2.get() << std::endl;

    TestTp tt(5, 6);
    auto f3 = tp.submit(tt);
    std::cout << f3.get() << std::endl;


    std::vector< std::future<int> > results;

    for(int i = 0; i < 10; ++i) {
        results.emplace_back(
            tp.submit([i] {
                std::cout << "hello " << i;
                std::this_thread::sleep_for(std::chrono::seconds(2));
                std::cout << "world " << i << std::endl;
                return i*i;
            })
        );
    }

    for(auto && result: results)
        std::cout << result.get() << ' ';

    return 0;

}
