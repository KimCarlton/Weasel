#include <iostream>
#include "net/Server.h"



int main()
{
    Server server(6518);
    server.run();

    std::cout << "test" << std::endl;
    return 0;
}