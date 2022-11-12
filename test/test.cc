#include <iostream>
#include "../log/block_queue.h"

int main()
{
    block_queue<int> bq(20);
    // prin(20);
    std::cout << bq.max_size() << std::endl;

    return 0;
}