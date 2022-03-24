#include"msg.h"
#include<iostream>
#include<time.h>
using namespace std;


int main() {
    Broker *c = new Broker();
    c->init();
    char cc;
    while(cin>>cc) {
        if(cc=='q'||cc=='Q')break;
        sleep(10);
    }
    delete c;
}