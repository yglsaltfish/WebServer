#include <iostream>
#include "../log/block_queue.h"
#include "../log/log.h"


void TestLog()
{
    Log::get_instance()->init("log", 0);
    Log::get_instance()->write_log(0, "321", "321");

}

int main()
{
    /* test Log */
    TestLog();
    


    return 0;
}