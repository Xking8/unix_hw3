#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#define MAXLINE 15000
#define MAXBUFF 15000
int outfd = fileno(stdout);
int stdin_hlr;
int astdin = dup(0);
FILE *fp1; //to cat > file.txt

typedef struct cmd
{
    char* wholeline;
    char* arg[256];
    char filename[256];
    bool arrow;

}cmd_t;
typedef struct npipe_msg
{
	char msgbuff[MAXBUFF];
	bool empty;
}npipe_msg_t;

void shell_service();
int cmd_parser(char*,cmd_t*);
void exec_cmd(cmd_t,int,int,int[][2],int,npipe_msg_t*,int[2],int,int[2],int,bool);
void reaper();

void move_npipemsg(npipe_msg_t*);
int main()
{
    signal(SIGCHLD,(void (*)(int))reaper);
	shell_service();
}

void shell_service() {
	stdin_hlr=dup(0);
	cmd_t cmd[1000];
	bool exclamation=0;//no use
	npipe_msg_t npipe_msg[2];

	for (int s=0;s<2;s++)
	{
		npipe_msg[s].empty=true;
	}
	while(1)
	{
		char line[MAXLINE];
		write(outfd,"shell-prompt$ ",14);
		fgets(line,MAXLINE,stdin);
		line[strlen(line)-1]=0;
		printf("%s.\n",line);
		int num_of_cmd=cmd_parser(line,cmd);
		int pipeA[2][2]={{-1,-1},{-1,-1}};
		int Wnumberpipe[2]={-1,-1};
		int Rnumberpipe[2]={-1,-1};
	
		int readpipe=-1,writepipe=-1,npipe_count=-1,rnpipe=0;
		//int pipeA[2][2];
		//int **pipeA;
		int pipeTurn=0;

		//int pipeN[1000][2];
		//if(pipe(pipeA[0])<0||pipe(pipeA[1])<0)
		//	fprintf(stderr,"pipe create error\n");

		//printf("initial pipe:pipeA [0][0]=%d [0][1]=%d [1][0]=%d [1][1]=%d\n",pipeA[0][0],pipeA[0][1],pipeA[1][0],pipeA[1][1]);
		//pipeA = malloc(2*sizeof(int*));
		//pipeA[0] = malloc(2*sizeof(int));
		//pipeA[1] = malloc(2*sizeof(int));
		
		move_npipemsg(npipe_msg);

		if(pipe(pipeA[0])<0||pipe(pipeA[1])<0)
			fprintf(stderr,"pipe create error\n");
		else
		{
			//printf("create pipes sucess\n");
			//printf("pipeA [0][0]=%d [0][1]=%d [1][0]=%d [1][1]=%d\n",pipeA[0][0],pipeA[0][1],pipeA[1][0],pipeA[1][1]);
		}
		if(pipe(Wnumberpipe)<0)
			fprintf(stderr,"num pipe create error\n");
		else
			//printf("Wnumpipe[0]=%d Wnumpipe[1]=%d\n",Wnumberpipe[0],Wnumberpipe[1]);

		if(npipe_msg[0].empty==false)
		
			if(pipe(Rnumberpipe)<0)
				fprintf(stderr,"num pipe create error\n");
			
			else
			{
				//printf("Rnumpipe[0]=%d Rnumpipe[1]=%d\n",Rnumberpipe[0],Rnumberpipe[1]);
				fcntl(Rnumberpipe[1],F_SETFL,fcntl(Rnumberpipe[1],F_GETFL) | O_NONBLOCK);
			}


		int i;		
		for(i=0;i<num_of_cmd;i++)
		{
			int exe_ind=i;
			dup2(stdin_hlr,0);
			//printf("###############processing %dth cmd\n",i);
			//printf("stdin_hlr:%d\n",stdin_hlr);
			
			readpipe=-1;
			writepipe=-1;
			npipe_count=-1;
			rnpipe=0; //flag of if read from Rnumberpipe, 0 or 1

			if( i==0 && npipe_msg[0].empty==false)
			{	
				//printf("in rnpipe rnumpipe[1]=%d\n",Rnumberpipe[1]);
				//close(Rnumberpipe[0]);
				//npipe_msg[0].msgbuff[strlen(npipe_msg[0].msgbuff-1)]='\0';
				//printf("after assign null\n");
				//printf("%s\n",npipe_msg[0].msgbuff);
				//printf("after print msgbugg, length=%d\n",strlen(npipe_msg[0].msgbuff));
				write(Rnumberpipe[1],npipe_msg[0].msgbuff,strlen(npipe_msg[0].msgbuff)+1);
				//write(1,npipe_msg[0].msgbuff,MAXBUFF);
				//printf("after write\n");
				close(Rnumberpipe[1]);
				Rnumberpipe[1]=-1;
				rnpipe=1;

			}

			if(i>0) //get stdin from pipe
			{	
				readpipe=pipeTurn;
				pipeTurn=(pipeTurn+1)%2;
				if(pipe(pipeA[pipeTurn])<0)
					fprintf(stderr,"pipe create error\n");
				else
					;
					//printf("***create pipe[%d]\n",pipeTurn);

			}	
			if(i<num_of_cmd-1) //pipe stdout to pipe
			{
				if(i+1==num_of_cmd-1 && isdigit(cmd[i+1].arg[0][0]))//pipe to number pipe
				{
					//printf("IN writeto numbered pipe\n");

					//printf("arg[0][0]=%c, arg[0][1]=%c, arg[0][2]=%c\n",cmd[i].arg[0][0],cmd[i].arg[0][1],cmd[i].arg[0][2]);
					//printf("arg[0][0]=%c, arg[0][1]=%c\n",cmd[i+1].arg[0][0],cmd[i+1].arg[0][1]);
					npipe_count= atoi(cmd[i+1].arg[0]);
					//printf("cccc count=%d\n",npipe_count);
					i=i+1;

				}	
				else
				{
					//printf("Not numbered pipe\n");
					writepipe=pipeTurn;
				}
			}
			else //i==num_of_cmd
			{
				//default pipe out to sockfd
			}
			//printf("$$$pipeA [0][0]=%d [0][1]=%d [1][0]=%d [1][1]=%d\n",pipeA[0][0],pipeA[0][1],pipeA[1][0],pipeA[1][1]);
			//printf("$$$Wnumpipe[0]=%d Wnumpipe[1]=%d\n",Wnumberpipe[0],Wnumberpipe[1]);
			exec_cmd(cmd[exe_ind],readpipe,writepipe,pipeA,npipe_count,npipe_msg,Wnumberpipe,rnpipe,Rnumberpipe,fileno(stdout),exclamation);
		}
	

	}//end of while
	

}

void exec_cmd(cmd_t cmd, int readpipe, int writepipe, int pipeA[][2], int npipe_count, npipe_msg_t* npipe_msg,int Wnumberpipe[2],int rnpipe,int Rnumberpipe[2],int sockfd,bool excla)
//void exec_cmd(cmd_t cmd, int readpipe, int writepipe, int** pipeA, int pipeN[][2],int sockfd)
{
	int pid;
	char filename[256];
	pid=fork();
	if(pid==0)
	{
        setpgid(0,getpid());
        tcsetpgrp(astdin,getpid());
        tcsetpgrp(fileno(stdout),getpid());
        printf("???????????????????");
		/*setsid();
		chdir("/"); 
    	umask(0);*/
		int readfd=STDIN_FILENO;
		int writefd=sockfd;
		//printf("-----------strlen=%d\n",strlen(cmd.arg[0]));
		//printf("!!!!!cmd.arg[0][2]=%c!!!!\n",cmd.arg[0][2]);
		//printf("readpipe=%d writepipe=%d\n",readpipe,writepipe);	
		//strcpy(filename,curdir);
		//mypath=strtok(mypath,":");
		//strcat(filename,mypath);
		//strcat(filename,"/");
		strcpy(filename,cmd.arg[0]);
		if(readpipe!=-1)
		{	
			//printf("innnnnnnnnnnnnnnn\n");
			close(pipeA[readpipe][1]);
			//printf("after close\n");
			//char tbuff[256];
			//read(pipeA[readpipe][0],tbuff,256);
			//printf("~~~~~~~~~~~~tbuff=%s\n",tbuff);
			dup2(pipeA[readpipe][0],STDIN_FILENO);
			close(pipeA[readpipe][0]);
			pipeA[readpipe][0]=-1;
			//close(pipeA[0][1]);
			//dup2(pipeA[0][0],STDIN_FILENO);

		}
		
		if(writepipe!=-1)
		{
			close(pipeA[writepipe][0]);
			dup2(pipeA[writepipe][1],STDOUT_FILENO);
			//close(pipeA[writepipe][1]);
		}
		else if (npipe_count!=-1)
		{
			//printf("dup Wnumberpipe[1] to stdout\n");
			if(dup2(Wnumberpipe[1],STDOUT_FILENO)<0)
				fprintf(stderr,"dup error\n");
			if(excla)
				dup2(Wnumberpipe[1],STDERR_FILENO);
		}
		else if (cmd.arrow)
		{
			//printf("**************dup fp1 to stdout\n");
			dup2(fileno(fp1),STDOUT_FILENO);
		}
		else
		{

			printf("dup sockfd to stdout\n");
			dup2(sockfd,STDOUT_FILENO);
		}
		if(!excla)
		{
			dup2(sockfd,STDERR_FILENO);
		}
		if(rnpipe==1)
		{
			if(!strcmp(cmd.arg[0],"noop"))
			{
				char tbuf[MAXBUFF];
				read(Rnumberpipe[0],tbuf,MAXBUFF);
				//printf("%s",tbuf);
			}
			else
				dup2(Rnumberpipe[0],STDIN_FILENO);
			close(Rnumberpipe[1]);


		}
		//printf("filename=%s\n",filename);
		//printf("cmd.arg[0]=%s\ncmd.arg[1]=%s\n",cmd.arg[0],cmd.arg[1]);
		if( execvp(filename,cmd.arg)<0)//,envp);
		{
			printf("Unknown command: [%s]\n",cmd.arg[0]);
		}
		//char* targ[]={"ls",(char*)0};
		//execve("/net/other/2017_1/0550722/ras/bin/ls",targ,envp);
		exit(0);

	}
	else//parent
	{
        setpgid(pid,pid);
		if(readpipe!=-1)
		{
			if(pipeA[readpipe][1]!=-1)
			{
				close(pipeA[readpipe][1]);
				//close(pipeA[readpipe][0]);
				pipeA[readpipe][1]=-1;//must assign -1 when close otherwise will be ambiguise
				//printf("assign pipeA[%d][1]=%d liao\n",readpipe,pipeA[readpipe][1]);
			}
			
			close(pipeA[readpipe][0]);
			pipeA[readpipe][0]=-1;
			//printf("assign pipeA[%d][0]=%d liao\n",readpipe,pipeA[readpipe][0]);
		}
		if(rnpipe==1)
		{
			close(Rnumberpipe[1]);
			Rnumberpipe[1]=-1;
		}
		if(writepipe!=-1)
			if(pipeA[writepipe][1]!=-1)
			{
				close(pipeA[writepipe][1]);
				pipeA[writepipe][1]=-1;
				//printf("assign pipeA[%d][1]=%d liao\n",writepipe,pipeA[writepipe][1]);
			}
		if(npipe_count!=-1)
		{
			close(Wnumberpipe[1]);//dont close when it does't be used in that cmd!
			Wnumberpipe[1]=-1;
		}
		//close(pipeA[readpipe][0]);
		//close(pipeA[readpipe][0]);
		//printf("before wait\n");
		wait(0);//problem
		if(npipe_count!=-1)
		{
			char tbuff[MAXBUFF]="";
			read(Wnumberpipe[0],tbuff,MAXBUFF);
			//printf("!~~~~~~~~~~~~tbuff=%s\n",tbuff);
			//printf("end print tbuff\n");
			strcat(npipe_msg[npipe_count].msgbuff,tbuff);
			npipe_msg[npipe_count].empty=false;
			//printf("npipe_msg[%d].msgbuff=%s\n",npipe_count,npipe_msg[npipe_count].msgbuff);
			//printf("end strcat\n");
		}
		//if(pipe(pipeA[0])<0||pipe(pipeA[1])<0)
		//	fprintf(stderr,"pipe create error\n");

		//char tbuff[20];
		//read(pipeA[writepipe][0],tbuff,20);
		//printf("!~~~~~~~~~~~~tbuff=%s\n",tbuff);

		//printf("after wait\n");
	}

	

}


int cmd_parser(char* line, cmd_t* cmd)
{
	int ind=0;
	cmd[0].wholeline=strtok(line,"|!\n\r");
	for(ind=1;ind<1000;ind++)
	{
		cmd[ind].wholeline=strtok(NULL,"|!\n\r");
		if(cmd[ind].wholeline==NULL)
			break;
		/*if(!strcmp(tmp_cmd[ind-1],">"))
		{
			printf("cmd_arg=%s\n",cmd_arg[ind]);
			cmd_arg[ind][strlen(cmd_arg[ind])-1]=0;
			fp1=fopen(cmd_arg[ind],"w+");
	
			arrow=1;
		}
		if(!strcmp(cmd_arg[ind],"|"))
		{
			stick=1;
			f_stick_ind=ind;
			//stickcommand=
		}*/
			
			//arg[i][strlen(arg[i])-1]=0;
			//printf("arg[%d]=%s length=%d\n",i,arg[i],strlen(arg[i]));
	}
	//sleep(1);
	//printf("arg[%d]=%s length=%d\n",1,arg[1],strlen(arg[1]));
	printf("now's ind=%d, tmp_cmd[ind]=%s ..\n",ind,cmd[ind].wholeline);
	//arg[i-1]=(char*)(0);
	//arg[i-1]="\n";
	//printf("After assign null to arg[i-1], arg[i-1]=%s ,,\n",arg[i-1]);
		
	//char* ttt="ls";
	//arg[1][strlen(arg[1])-1]=0;
	//printf("arg[0][2]=%s\n",arg[0][2]);
	//printf("arg[0]'s length=%u\n",strlen(arg[0]));
	
	//dup2(sockfd,STDERR_FILENO);

	//cmd[ind-1].wholeline[strlen(cmd[ind-1].wholeline)-1]=0;//set the last character to NULL(must do!!!)
	int i,j;
	for(i=0;i<ind;i++)
	{
		cmd[i].arrow=0;
		cmd[i].arg[0]=strtok(cmd[i].wholeline," \n");
		for(j=1;j<256;j++)
		{
			cmd[i].arg[j]=strtok(NULL," \n");
			if(!strcmp(cmd[i].arg[j-1],">"))
			{
				//strcpy(cmd[i].arg[j-1],cmd[i].arg[j]);
				//j--;
				fp1=fopen(cmd[i].arg[j],"w+");
				cmd[i].arrow=1;
			}
			else if(!strcmp(cmd[i].arg[j-1],">>"))
			{
				//strcpy(cmd[i].arg[j-1],cmd[i].arg[j]);
				//j--;
				fp1=fopen(cmd[i].arg[j],"a+");
				cmd[i].arrow=1;
			}

			if(cmd[i].arg[j]==NULL)
			{
				printf("at %d %d break\n",i,j);
				break;
			}
		}
		cmd[i].arg[j]=(char*)0;
		if(cmd[i].arrow==1)
		{
			cmd[i].arg[j-1]=(char*)0;

			cmd[i].arg[j-2]=(char*)0;
		}
	}
	//cmd[2].arg[j][strlen(cmd.[2].arg[j])-1]=0;	
	 //print out each command
	for(i=0;i<ind;i++)
		for(j=0;j<256&&cmd[i].arg[j]!=NULL;j++)
			printf("cmd[%d].arg[%d]=%s\n",i,j,cmd[i].arg[j]);
	
	return ind;

}
void move_npipemsg(npipe_msg_t* npipe_msg)
{
	/*int i;
	for(i=0;i<1000;i++)
	{
		if(i<999)
		{
			strcpy(npipe_msg[i].msgbuff,npipe_msg[i+1].msgbuff);	
			npipe_msg[i].empty=npipe_msg[i+1].empty;
		}
		else
		{
			strcpy(npipe_msg[i].msgbuff,"");	
			npipe_msg[i].empty=true;

		}	
	}*/
}
void reaper()
{
    int status;
    while(waitpid(0,&status,WNOHANG)>=0)
        ;
}

