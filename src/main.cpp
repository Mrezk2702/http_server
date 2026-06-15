#include "server/Socket.h"
#include <iostream>
using namespace std;




int main()
{
    try
    {
        Socket server_socket(AF_INET, SOCK_STREAM);
        int opt = 1;
        server_socket.set_option_int(SOL_SOCKET, SO_REUSEADDR, opt);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
return 0;
}