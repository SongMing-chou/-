#include<iostream>
#include<vector>


using namespace std;


int main() {
    printf("hello linux!!\n");
    for(int  i = 0;i < 100;i++) {
        if(i%10 == 0)printf("\n");
        printf("%d ",i);
    }
    printf("\n");
    return 0;
}