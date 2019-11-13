#include<stdio.h>
#include<signal.h>
#include<unistd.h>
#include<stdlib.h>
#include <sys/stat.h>
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
#define iter 20000000
int H=65536,S=4,r=0,init=1;
char* a;

//Interpret signals from driver
void sig_handler(int signo)
{
	if (signo == SIGUSR1)
	{

	}
	if(signo==SIGUSR2)
	{
		init=0;
	}
}

//Find average access time for S spots at a stride length of H, at a given offset(in terms of number of sets)
void AMAT(int H,int S,int offset)
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
	printf("%d \t\t%llu\n",r,sum/iter);
	return;
}


int main()
{
	
	int ppid=getppid();
	signal(SIGUSR1, sig_handler);
	signal(SIGUSR2, sig_handler);
	a=(char*) malloc(MAX);
	while(init)
	{
		// Goes to sleep as soon as it enters the loop. It is waken up by the driver function using signals.
		pause();
		if(!init)	break;
		AMAT(H,S,0);
		r++;	
		r=r%16;
		
	}
	pause();
	r=0;
	while(1)
	{
		pause();
		AMAT(H,S,0);
		kill(ppid,SIGUSR1);
	}
}