#include<unistd.h>
#include<seccomp.h>
#include<linux/seccomp.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/wait.h>
#include<string.h>

#include<bits/stdc++.h>




#ifdef o
int main(){
    // char c[128] = "g++ -o";
    // char *x = " work";
    // int len_xx = strlen(x);

    // char *t = ".cpp";
    // int len_t = strlen(t);
    // printf("%d  %d\n",len_t,len_xx);
    // strncpy(c + 6,x,len_xx);
    // strncpy(c+6+len_xx,x,len_xx);
    // strncpy(c+6+len_xx+len_xx,t,len_t);
    // system(c);

    char savefile_name[64] = "hgakdgagdha";

    char c[128];
    char* end = ".cpp";
    int  i = 0;int j = 0;
    for(;i < strlen(savefile_name);i++) {
        c[i] = savefile_name[i];
    }
    for(;j < strlen(end);j++) {
        c[i+j] = end[j];
    }
    c[i+j] = '\0';
    printf("%s\n",c);
    // for(int  i= 0;i < 20;i++) {
    //     printf("%c",c[i]);
    // }
    // printf("%s\n",c);
    // scmp_filter_ctx ctx;
    // ctx = seccomp_init(SCMP_ACT_ALLOW);
    // seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(write),1,SCMP_A2(SCMP_CMP_EQ,0x10));//第2(从0)个参数等于0x10
    // seccomp_load(ctx);
    // write(1,"i will give you a shell\n",24);//不被拦截
    // write(1,"1234567812345678",0x10);//被拦截
    return 0;
}
#endif
#include<vector>
#include<iostream>

using namespace std;

class Qsort {
public:
    void qsort(vector<int>&nums) {
        check(nums,0,nums.size() - 1);
        return ;
    }
    void check(vector<int>& nums,int l ,int r) {
        if(l >= r)return ;
        int target,ll = l,rr = r,mid = l + (r - l) / 2;

        if(nums[l] < nums[mid]) swap(nums[l],nums[mid]);
        //nums[l] >= nums[mid]
        if(nums[r] <= nums[mid]) swap(nums[l],nums[mid]);
        else if(nums[r] < nums[l] && nums[r] >= nums[l])swap(nums[l],nums[r]);

        target  = nums[l];
        while( l < r) {
            while(l < r&&nums[r] >= target)r--;
            if( l < r) nums[l] = nums[r];
            while(l < r&& nums[l] < target)l++;
            if(l < r)nums[r] = nums[l];
        }

        nums[l] = target;

        check(nums,ll,l - 1);
        check(nums,l + 1,rr);
    }
};
#include<stdio.h>
#include<string.h>
class mergeSort {
public:
    void sort(vector<int>&nums,int l,int r,vector<int>&temp) {
        if(l >= r) {
            return;
        }
        int mid = l + (r  - l )/2;
        sort(nums,l,mid,temp);
        sort(nums,mid+1,r,temp);
        check(nums,l,mid,r,temp);
    }
    void check(vector<int>& nums,int l,int mid,int r,vector<int> &temp) {
        for(int  i = l;i <=r;i++)temp[i] = nums[i];
        int cur = l;
        int k = mid + 1;
        while(l <= mid&&k<=r) {
            if(temp[l] <= temp[k]) {
                nums[cur] = temp[l];
                l++;
            }else {
                nums[cur] = temp[k];
                k++;
            }
            cur++;
        }
        while(l<= mid) {
            nums[cur++] = temp[l++];
        }
        while(k<=r) {
            nums[cur++] = temp[k++];
        }
    }
};
#include<unistd.h>
#include<sys/types.h>
#include<fcntl.h>
#include<stdlib.h>
#include<stdio.h>

int main() {
    // int c = open("./text.txt",O_RDONLY);
    // if( c < 0)printf("open fd error!!\n");
    // dup2(c,0);
    int n;
    cin>>n;
    mergeSort item;
    while(n>0) {
        n--;
        int m;
        cin>>m;
        vector<int> temp(m);
        for(int i = 0;i < m;i++) {
            cin>>temp[i];
        }
        vector<int> c = temp;
        item.sort(temp,0,temp.size() - 1,c);
        for(int  i = 0;i < m;i++) {
            printf("%d ",temp[i]);
        }
        printf("\n");
    }
    return 0;
}