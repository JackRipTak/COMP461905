#include<bits/stdc++.h>

using namespace std;

int target[100000000];

void *basic_memset(void *s,int c,size_t n){
	size_t cnt=0;
	unsigned int cc=0;
	for(int i=1;i<=4;i++) cc<<=8,cc+=(unsigned int )c;
	unsigned int *tt=(unsigned int *)s;
	int nn=n/4-1;
	while(cnt<nn) *tt++=cc,cnt++;
	cnt*=4;
	unsigned char *tmp=(unsigned char*) tt;
	while(cnt<n) *tmp++=c,cnt++;
	return s;
}


int main(){
	int other[1000000];
	for(int i=1;i<=10;i++){
		for(int i=1;i<=100000;i++) other[i]=rand();
		double tmp=clock(); 
		memset(target,i,sizeof(target));
		cout<<(clock()-tmp)/(double)CLOCKS_PER_SEC<<endl;
	}
	return 0;
}
