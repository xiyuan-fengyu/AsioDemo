//
// Created by xiyuan_fengyu on 2018/1/24.
//

#include <iostream>
#include <functional>

using namespace std;
using namespace std::placeholders;


int main() {

    auto f = bind([](int i, int j){
        cout << (i + j) << endl;
    }, _1, 2);
    f(1);

    return 0;
}
