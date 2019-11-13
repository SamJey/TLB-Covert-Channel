#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
void sig_handler(int signo)
{
	if (signo == SIGUSR1)
	{

	}
}
/*Change appropriately*/
char* hyperthread1 ="0";
char* hyperthread2 ="4";
/*Insert path here*/
char* path="/home/ram/Workspace/cs3500/project/Code/task2/";
char* temp;


//This program is a driver code to make sure p1 and p2 execute simultaneously, and to synchronize their accesses.
//The driver program is just for convenience during testing. In the actual trial run(task 3), t
//here will be no driver program and no other forms of communication between p1 and p2
int main()
{
	signal(SIGUSR1, sig_handler);
	temp=malloc(1000);
	char* name = "data";
	int size = 128;
	int pid = getpid();
	int fd = shm_open(name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	ftruncate(fd, size);
	char* shmem = mmap(NULL, 128, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	//Running p1
	int p1=fork();
	if(!p1)
	{
		char* argv[5];
		argv[0]="taskset";
		argv[1]="-c";
		argv[2]="0";
		strcpy(temp,path);
		strcat(temp,"p1");
		argv[3]=temp;
		argv[4]=0;
		execvp("taskset",argv);
	}

	//Running p2
	int p2=fork();
	if(!p2)
	{
		char* argv[5];
		argv[0]="taskset";
		argv[1]="-c";
		argv[2]="4";
		strcpy(temp,path);
		strcat(temp,"p2");
		argv[3]=temp;
		argv[4]=0;
		execvp("taskset",argv);
	}
	sleep(2);
	int ch=1;
	//Looping over number of sets(16) and synchronizing p1 and p2 for each set access
	printf("Set number\tClock cycles\n");
	while(ch==1){
		for(int i=0;i<16;i++)
		{
			kill(p1, SIGUSR1);
			kill(p2, SIGUSR1);
			pause();
		}
		printf("Do you want to recalibrate?(0/1) \n");
		scanf("%d",&ch);
		if(!ch)	break;
	}

	kill(p1,SIGUSR2);
	kill(p2,SIGUSR2);

	//User sends the set number with largest AMAT
	//Sending the offest to P2 vie shared memory
	printf("Send offset to P2:\n");
	char* s =malloc(128);
	scanf("%s",s);
	memcpy(shmem,s,128);
	sleep(1);
	kill(p1,SIGUSR1);
	kill(p2,SIGUSR1);
	sleep(1);
	while(1)
	{
		printf("Do you want to run both?(0/1) ");
		scanf("%d",&ch);
		if(ch==-1)
			break;
		//Accessing non-confilicting set
		if(ch==0)
		{
			kill(p1, SIGUSR1);
			kill(p2,SIGUSR2);	
		}
		//Accesssing the same set
		else
		{
			kill(p2, SIGUSR1);
			kill(p1, SIGUSR1);
		}
		pause();
	}
	kill(p1,SIGKILL);
	kill(p2,SIGKILL);
	return 0;
}