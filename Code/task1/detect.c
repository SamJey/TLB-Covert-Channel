#include<stdio.h>
#include<stdlib.h>
static __inline__ unsigned long long rdtsc(void)
{
	unsigned hi, lo;
	__asm__ __volatile__ ("xorl %%eax,%%eax\n cpuid \n" ::: "%eax", "%ebx", "%ecx", "%edx"); // flush pipeline
	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	__asm__ __volatile__ ("xorl %%eax,%%eax\n cpuid \n" ::: "%eax", "%ebx", "%ecx", "%edx"); // flush pipeline
	return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}
#define ull unsigned long long
#define iter 2000000
#define MAX 4096*4096*32
#define PAGE 4096
ull tic,toc;
ull sum,cnt;
char* a;;

//Function to find average access time looping over S spots, at a stride of H bytes away from each other
//H is stride length in bytes
//S is the number of spots being accessed
ull AMAT(int H,int S)
{
	sum=0;
	char temp;
	for(int i=0,j=0;i<iter;i++,j+=H)
	{
		if(i%S==0)	j =0;
		tic=rdtsc();
		temp=a[j];
		toc=rdtsc();
		sum+=toc-tic;
	}
	return sum/iter;
}

int main()
{
	a=(char*) malloc(MAX);
	FILE* fp=fopen("out.txt","w+");
	int H,S;
	fprintf(fp, "STRIDE/SPOTS " );
	for(int i=0;i<32;i++)
	{
		fprintf(fp, "%3d ",i+1 );
	}
	fprintf(fp, "\n" );
	//Varying H,S and storing the results in a file
	for(H=16;H<MAX;H*=2)
	{
		fprintf(fp, "%11d : ", H);
		printf("Stride Length : %d\nIterating over number of spots :\n",H );
		for(S=1;H*S<MAX && S<=32;S++)
		{
			printf("%d \n", S);
			ull amat=AMAT(H,S);
			fprintf(fp,"%llu ",amat);
		}
		printf("\n");
		fprintf(fp,"\n");
	}
	fclose(fp);
}