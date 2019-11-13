#include<stdio.h>
#include<signal.h>
#include <sys/mman.h>
#include<unistd.h>
#include<stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
static __inline__ unsigned long long rdtsc(void)
{
	unsigned hi, lo;
	__asm__ __volatile__ ("xorl %%eax,%%eax\n cpuid \n" ::: "%eax", "%ebx", "%ecx", "%edx"); // flush pipeline
	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	__asm__ __volatile__ ("xorl %%eax,%%eax\n cpuid \n" ::: "%eax", "%ebx", "%ecx", "%edx"); // flush pipeline
	return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}
#define ull unsigned long long
#define MAX 4096*4096*32
int H= 65536,S=4,iter=20000000;
ull tic,toc,sum;;
char temp;
int i,j,r;
ull tstamp=(1ll<<32);
char* a;
double arr[100];
double max;
int ptr=0;
int init=0;
// Receiver

// Returns the average memory access time for stride length H, and number of spots S.
ull AMAT(int H,int S,int offset)
{
	ull tic,toc,sum;
	sum=0;
	char temp;
	for(int i=0,j=offset;i<iter;i++,j+=H)
	{
		if(i%S==0)	j=offset;
		tic=rdtsc();
		temp=a[j];
		toc=rdtsc();
		sum+=toc-tic;
	}
	if(init)
		printf("%d\t\t%llu\t\t",r,sum/iter);
	arr[ptr++]=((double)sum)/iter;
	if(max<arr[ptr-1])
		max=arr[ptr-1];
	return sum/iter;
}


int main()
{
	r=0;
	a=(char*)malloc(MAX);
	while(rdtsc()&tstamp) usleep(10000);
	int t=0;
	// Accessing the same set over 32 iterations while the other process accesses different set in each iteration
	for(t=0;t<32;)
	{
		if(rdtsc()&tstamp)
		{
			while(rdtsc()&tstamp)
			{
				AMAT(H,S,0);
			}
			r++;
			t++;
			r%=16;
		}
		else
		{
			usleep(100000);
		}
	}
	init=1;
	int idx=0;
	int res[1000];
	ull ans;
	r=0;
	t=0;
	printf("Bit Number\tClock Cycles\tReceived Bit\n");
	while(1)
	{
		if(rdtsc()&tstamp)
		{
			while(rdtsc()&tstamp)
			{
				ans=AMAT(H,S,0);
			}
			// If the AMAT is fairly low, then the other process has finished and therefore indicates end of file
			if(ans<=185)
				break;
			// If the AMAT is high, then the other process is accessing the conflicting set. Therefore the process records the bit received as a 1.
			if(ans>=194)
				res[t]=1;
			// Similarly, here, the other process is accessing some nonconflicting set. The process records the bit recieved as a 0.
			else
				res[t]=0;
			printf("%d\n", res[t]);
			r++;
			t++;
			r%=16;
		}
		else
		{
			usleep(100000);
		}
	}
	printf("EOF\nReceived\n");
	for(i=0;i<t;i++)
	{
		printf("%d", res[i]);
	}
	printf("\n");
}