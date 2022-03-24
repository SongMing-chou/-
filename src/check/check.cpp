#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>

/**
 * @brief 主要是用来检查文件的正确性
 * 
 * @return int 
 */
int main() {

    char * end = "_out.txt";

    int n = 1000;
    char buf[600];//存进标准内容

    int openstd = open("stander.txt",O_RDONLY);
    if(openstd < 0) {
        printf("open std error!!!\n");
    }
    read(openstd,buf,sizeof(buf));

    char check[600];

    int err = 0;
    int right = 0;
    for(int  i = 6 ;i < n + 6;i++) 
    {
        memset(check,0,sizeof(check));
        char s[60] = "/home/user/MYPROJECT/src/test/";
        int len = strlen(s);
        char t[10];
        sprintf(t,"%d",i);
        strncpy(s+len,t,strlen(t));
        strncpy(s+len+strlen(t),end,strlen(end));
        int fd = open(s,0666);
        int k =read(fd,check,sizeof(check));
        bool ok = true;
        for(int  j = 0;j < k;j++) {
            if(buf[j] != check[j]) {
                printf("fd == %d error!!!\n",i);
                close(fd);
                err++;
                ok = false;
                break;
            }
        }
        if(ok) right++;
    close(fd);
    }
    printf("error  = %d  right == %d\n",err,right);
    return 0;
}