#include"server/server.h"

int main() {
    
    char * port = "9006";
    char* publisherhost = "127.0.0.2";
    char* brokerhost = "127.0.0.1";
    
    Server m_server(port,publisherhost,brokerhost);

    if(!m_server.Listen()) {
        printf("in maincpp listen error!!\n");
    }
    m_server.eventLoop();

    return 0;

}