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
int H = 65536;
int S=4;
int t,i,j,k;
#define iter 20000000
ull tstamp=(1ll<<32);
char* a;
ull res[1000];
// Transmitter
int init=1;

// Returns the average memory access time for stride length H, and number of spots S.
void AMAT(int H,int S,int offset)
{
	ull tic,toc,sum;
	sum=0;
	char temp;
	for(i=0,j=offset;i<iter;i++,j+=H)
	{
		if(i%S==0)	j=offset;
		tic=rdtsc();
		temp=a[j];
		toc=rdtsc();
		sum+=toc-tic;
	}
	res[t]=sum/iter;
	// Concurrency issues. Please restart.
	if(res[t]<=185)
	{
		printf("Error. Start again!\n");
		exit(0);
	}
	if(init)
		printf("%d\t\t%llu\n",t%16,res[t]);
	return;
}
int main(int argv,char* argc[])
{
	printf("Set number\tClock Cycles\n");
	if(argv!=2)
	{
		printf("Enter the data to be transmitted\n");
		return 0;
	}
	int n=strlen(argc[1]);
	a=(char*)malloc(MAX);
	int offset=0;
	while(rdtsc()&tstamp) usleep(10000);
	// Calibration phase. This process accesses different sets each time and the conflicting set is obtained using the AMAT data using a trivial heuristic.
	for(t=0;t<32;)
	{
		if(rdtsc()&tstamp)
		{
			while(rdtsc()&tstamp)
			{
				AMAT(H,S,offset);
			}
			offset+=4096;
			offset%=H;
			t++;
		}
		else
		{
			usleep(100000);
		}
	}
	init=0;
	
	// Finding the conflicting set.
	int cnt[16];
	for(i=0;i<16;i++)	cnt[i]=0;
	for(i=0;i<1;i++)
	{
		int mx=0;
		for(j=0,k=16*i;j<16;j++,k++)
		{
			if(mx<res[k])
				mx=res[k];
		}
		for(j=0,k=16*i;j<16;j++,k++)
		{
			if(res[k]==mx)
				cnt[j]++;
		}
	}
	int mx=0,idx=-1;
	for(i=0;i<16;i++)
	{
		if(cnt[i]>mx)
		{
			mx=cnt[i];
			idx=i;
		}
	}
	printf("The matching set corresponds to %d\n",idx);

	// p2 sends data using the following mechanism: If the value to be sent is a 0, then p2 accesses nonconflicting set. Otherwise p2 accesses the conflicting set
	printf("Sending %s\n",argc[1]);
	int a=idx*4096;
	int b=((idx)?0:1)*4096;	// nonconflicting offset
	for(k=0;k<n;)
	{
		if(rdtsc()&tstamp)
		{
			while(rdtsc()&tstamp)
			{
				if(argc[1][k]=='0')
					AMAT(H,S,b);
				else
					AMAT(H,S,a);
			}
			k++;
		}
		else
		{
			usleep(100000);
		}
	}
	printf("Transmitted\n");
}