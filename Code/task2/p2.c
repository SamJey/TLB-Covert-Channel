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
#define iter 20000000
int H=65536,S=4,r=0,init=1,offset=0,conf;
char* a;

//p2 loops through the 16 different sets

//Interpret signals from driver
void sig_handler(int signo)
{
	if (signo == SIGUSR1)
	{
		conf=1;
	}
	if(signo==SIGUSR2)
	{
		init=0;
		conf=0;
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
	return;
}
int main()
{

	int ppid=getppid();
	// setting up shared memory
	char* name = "data";
	int size = 128;
	int pid = getpid();
	int fd = shm_open(name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	ftruncate(fd, size);
	char* shmem = mmap(NULL, 128, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	signal(SIGUSR1, sig_handler);
	signal(SIGUSR2,sig_handler);
	a=(char*) malloc(MAX);
	while(init)
	{
		// Goes to sleep as soon as it enters the loop. It is waken up by the driver function using signals.
		pause();
		if(!init)	break;
		AMAT(H,S,offset);
		offset+=4096;
		offset%=H;
		r++;	
		r=r%16;
		// Signals the driver function
		kill(ppid,SIGUSR1);
	}
	pause();
	// The conflicting set number is read from shared memory/
	int off=atoi(shmem);
	offset=off*4096;
	int noconflict=(offset+4096)%H;	
	r=off;
	while(1)
	{
		pause();
		// if conf is set, then the conflicting set is accessed
		// otherwise any other non conflicting set is accessef
		if(conf)
			AMAT(H,S,offset);
		else
			AMAT(H,S,noconflict);
	}
}