#include<bits/stdc++.h>

using namespace std;


void *basic_memset(void *s,int c,size_t n){
	size_t cnt=0;
	unsigned char *tmp=(unsigned char*) s;
	while(cnt<n) *tmp++=(unsigned char) c,cnt++;
	return s;
}

int target[100000000];

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
