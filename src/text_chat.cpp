/**
  * @naveenmu_assignment1
  * @author  Naveen Muralidhar Prakash <naveenmu@buffalo.edu>
  * @version 1.0
  *
  * @section LICENSE
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License as
  * published by the Free Software Foundation; either version 2 of
  * the License, or (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful, but
  * WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  * General Public License for more details at
  * http://www.gnu.org/copyleft/gpl.html
  *
  * @section DESCRIPTION
  *
  * This contains the main function. Add further description here....
  */

#include<iostream>
#include<sstream>
#include<string>
#include<cstdlib>
#include<sys/types.h>
#include<netdb.h>
#include<sys/wait.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<signal.h>
#include<errno.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<ifaddrs.h>
#include<vector>
#include<algorithm>
#include<cctype>

#define PORT "9000"

#define STDIN 0

#include "../include/global.h"
#include "../include/logger.h"

using namespace std;


struct Block
{
	char IP[50];
};

struct message
{
	char info[1400];

};

/* State of each client is stored in this structure */
struct ListNode
{
        char IP[50];
        int sockfd;
        int port;
        bool log_status;
	int num_sent;
        int num_recv;
	bool reg;
        //bool blocked;
        vector<struct Block>BlockList;
	vector<struct message>buffer;
};

bool mycomp(ListNode a,ListNode b)
{

        return a.port<b.port;
}

 
 struct logList
{
        char sno[2],name[100],IP[200],port[10];
};


int main(int argc,char **argv)
{
	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/* Clear LOGFILE*/
    fclose(fopen(LOGFILE, "w"));

	/*Start Here*/


class Server
{
	int sockfd,newfd;
	vector<struct ListNode>remoteList;
	//vector<struct Buffer>BufMap;
	struct sockaddr_in res;
	struct sockaddr remoteaddr;
	int fdmax,yes;
	socklen_t addrlen;
	fd_set master,read_fds,write_fds;
	int port;
	
	public:
	void myIPgoogle() //Helper function to fetch the inet IP address and referred from Wenjun's Recitation Material
	{
		struct addrinfo ip_hints,*ip_res;
		struct sockaddr_in me;
		socklen_t mylen;
		char IP[2000];
		int tempfd;
		memset(&ip_hints,0,sizeof(ip_hints));
		ip_hints.ai_family=AF_INET;
		ip_hints.ai_flags=AF_UNSPEC;
		ip_hints.ai_socktype=SOCK_DGRAM;

		if(getaddrinfo("8.8.8.8","53",&ip_hints,&ip_res)==-1)
		{
			perror("ip getaddrinfo: ");
			exit(-1);
		}

		
		if((tempfd=socket(ip_res->ai_family,ip_res->ai_socktype,ip_res->ai_protocol))==-1)
		{
			perror("ip socket: ");
			exit(-1);
		}
	

		if(connect(tempfd,ip_res->ai_addr,ip_res->ai_addrlen)==-1)
		{
			perror("ip connect: ");
			exit(-1);
		}
		
		mylen=sizeof(me);

		if(getsockname(tempfd,(struct sockaddr*)&me,&mylen)==-1)
		{
			perror("getsock: ");
			exit(-1);
		}

		inet_ntop(AF_INET,&(me.sin_addr),IP,sizeof(IP));
		if(strcmp(IP,"127.0.0.1")!=0)
			cout<<IP<<endl<<flush;
		
		close(tempfd);
	}

	void myIP()
	{
		struct ifaddrs *ifa,*p;
		char IP[2000];
		char command_str[]="IP";
		if(getifaddrs(&ifa)==-1)		//Referred from man pages 
		{
			cse4589_print_and_log("[%s:ERROR]\n", command_str);
			fflush(NULL);
			cse4589_print_and_log("[%s:END]\n", command_str);
			fflush(NULL);
			return;
		}
		
		

		for(p=ifa;p!=NULL;p=p->ifa_next)
		{	
			struct sockaddr *s=p->ifa_addr;
			
			if(s->sa_family==AF_INET)
			{
				inet_ntop(AF_INET,&(((struct sockaddr_in*)s)->sin_addr),IP,sizeof(IP));

				if(strcmp(IP,"127.0.0.1")!=0)
				{	cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
					fflush(NULL);
					cse4589_print_and_log("IP:%s\n",IP);
					fflush(NULL);
					cse4589_print_and_log("[%s:END]\n", command_str);
					fflush(NULL);
					//cout<<IP<<endl	
					break;
				}
			}
		
		}
	}

	void myPort()
	{	char command_str[]="PORT";

			cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
			fflush(NULL);
			cse4589_print_and_log("PORT:%d\n",ntohs(res.sin_port));
			fflush(NULL);
			//cout<<ntohs(res.sin_port)<<endl<<flush;
			cse4589_print_and_log("[%s:END]\n", command_str);
			fflush(NULL);
	}
	

	char* myList(char* m)
	{	
		char *p=new char[500];
		p[0]='\0';
		
		int i;
		char host[200],service[200];
		struct sockaddr_in s;
		socklen_t slen=sizeof(s);
		
		sort(remoteList.begin(),remoteList.end(),mycomp);
		
		for(i=0;i<remoteList.size();i++)
		{
		 if(remoteList[i].reg==true&&remoteList[i].log_status==true)
		 {	strcat(p,"-");

			stringstream stream;
			stream<<(i+1); 
			string temp=stream.str();
			char *num=new char[temp.length()+1];
			strcpy(num,temp.c_str());

			//cout<<num<<endl;
			strcat(p,num);
			strcat(p," ");
			//cout<<p<<endl;
			inet_aton(remoteList[i].IP,&s.sin_addr);
			s.sin_family=AF_INET;
		
			getnameinfo((struct sockaddr*)&s,slen,host,sizeof(host),service,sizeof(service),NI_NAMEREQD);
			strcat(p,host);
			
			
			strcat(p," ");
			inet_ntop(AF_INET,&(s.sin_addr),host,INET_ADDRSTRLEN);

			strcat(p,host);
			strcat(p," ");
			stream.str("");
			stream<<remoteList[i].port;
			temp=stream.str();
			delete[] num;
			num=new char[temp.length()+1];
			strcpy(num,temp.c_str());
			strcat(p,num);
			delete[] num;
		 }
		}	
			

		strcpy(m,p);
	
		return m;
	}

	void printList()
	{	char hostname[200],service[100];
		sockaddr_in s;
		socklen_t size=sizeof(s);
		char command_str[]="LIST";
		cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
		fflush(NULL);
		for(int i=0;i<remoteList.size();i++)
		{	if(remoteList[i].reg==true && remoteList[i].log_status==true)
			{	inet_aton(remoteList[i].IP,&s.sin_addr);
                        	s.sin_family=AF_INET;
				getnameinfo((struct sockaddr*)&s,size, hostname,sizeof(hostname),service,sizeof service,NI_NAMEREQD);
				cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", i+1, hostname, remoteList[i].IP, remoteList[i].port);
				fflush(NULL);
			//cout<<i+1<<" "<<hostname<<" "<<remoteList[i].IP<<" "<<remoteList[i].port<<endl<<flush;
			}

		}	
		cse4589_print_and_log("[%s:END]\n", command_str);
		fflush(NULL);
	}
	
	Server(char *P)
	{	//Referred from Beej's guide 
		//port=atoi(P);
		yes=1;

		memset(&res,0,sizeof(res));
		res.sin_family=AF_INET;
		stringstream convertport(P);

		if(! (convertport>>port))
			port=0;

		res.sin_port=htons(port);
		res.sin_addr.s_addr=htonl(INADDR_ANY);
		
		//if(getaddrinfo(NULL,PORT,&hints,&res)==-1)
		//{
		//	perror("getaddrinfo: ");
		//	exit(-1);
		//}

		if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
		{
			perror("socket error: ");
			exit(-1);
		}
		

		if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))==-1)
		{
			perror("sockopt error: ");
			exit(-1);
		}
		

		if(bind(sockfd,(struct sockaddr*)&res,sizeof(res))==-1)
		{
			perror("bind error: ");
			exit(-1);
		}

		if(listen(sockfd,40)==-1)
		{
			perror("listener error: ");
			exit(-1);
		}

		
	}
	
	void sendall(int sock,char *type,char* buf)
	{	//Referred from beej's partial send() example
		char *message=new char[1048];
		strcat(type,buf);
		strcpy(message,type);
		int size=strlen(type);
		int n;
		int total=0;
		send(sock,&size,sizeof(size),0);
		while(total<size)
		{	//ut<<type<<endl<<flush;
			n=send(sock,message+total,size,0);
		
			if(n==-1)
			break;
			
			total+=n;
			size-=n;
		}
		delete[] message;
		//cout<<n<<"bytes were sent"<<endl<<flush;
	}

	 char* recvmessage(int sockfd,char *buf)
        {
         	//Referred from beej again       
                char *str=new char[1500];
                int size;
                if(recv(sockfd,&size,sizeof(size),0)==0)
		{	
			FD_CLR(sockfd,&master);

			for(int i=0;i<remoteList.size();i++)
			{
				if(remoteList[i].sockfd==sockfd)
					remoteList.erase(remoteList.begin()+i);

			}

			close(sockfd);

		}
		
		
                int total=0;
                int n;
                while(total<size)
                {
                        n=recv(sockfd,str+total,size,0);

                        if(n==-1)
                        break;

                        total+=n;
                        size-=n;

                }

                str[total]='\0';

	 return str;
        }

	char *extractFirst(char *input)
        {
                char *first=new char[30];
                int j=0;
                for(int i=0;input[i]!='\0'&&input[i]!=' ';i++,j++)
                        first[j]=input[i];
                first[j]='\0';

                return first;
        }

        char *extractSecond(char * input)
        {
                char *second=new char[300];
                int j=0;
                int i;
                for(i=0;input[i]!=' '&&input[i]!='\0';i++);

                for(;isalnum(input[i])==0&&input[i]!='\0';i++);

                for(;input[i]!=' '&&input[i]!='\0';i++,j++)
                        second[j]=input[i];

                second[j]='\0';

                return second;
        }

        char *extractThird(char *input)
        {
                char *third=new char[1024];
                int j=0;
                int i;
                for(i=0;input[i]!=' '&&input[i]!='\0';i++);

                //for(;isalnum(input[i])==0&&input[i]!='\0';i++);

                i++;
		for(;input[i]!=' '&&input[i]!='\0';i++);
		
                //for(;isalnum(input[i])==0&&input[i]!='\0';i++);

		i++;
                for(;input[i]!='\0';i++,j++)
                        third[j]=input[i];

                third[j]='\0';

                return third;
        }

	int findClient(char *IP)
	{
		for(int i=0;i<remoteList.size();i++)
			if(strcmp(remoteList[i].IP,IP)==0)
			{
				return i;
			}
		
		return -1;
	}
		

	int findClient(int sock)
	{

		for(int i=0;i<remoteList.size();i++)
			if(remoteList[i].sockfd==sock)
				return i;

		return -1;
	}

	bool isBlocked(int sendsock,int rind)
	{
		int index=findClient(sendsock);
		
		for(int i=0;i<remoteList[rind].BlockList.size();i++)
			if(strcmp(remoteList[index].IP,remoteList[rind].BlockList[i].IP)==0)
				return true;
			
		return false;
	}

	void removeBlock(int sockfd, char *IP)
	{	int index;
		int eind;
		index=findClient(sockfd);
		
		for(int i=0;i<remoteList[index].BlockList.size();i++)
		{	
			if(strcmp(remoteList[index].BlockList[i].IP,IP)==0)
			{	eind=i;	
				break;
			}

		}

		remoteList[index].BlockList.erase(remoteList[index].BlockList.begin()+eind);

	}


	void printStatistics()
	{
		char hostname[200],service[100];
                sockaddr_in s;
                socklen_t size=sizeof(s);
		char log[20];
		cse4589_print_and_log("[%s:SUCCESS]\n","STATISTICS");
		for(int i=0;i<remoteList.size();i++)
		{ 
		     if(remoteList[i].reg==true)
			{
				inet_aton(remoteList[i].IP,&s.sin_addr);
                        	s.sin_family=AF_INET;
                        	getnameinfo((struct sockaddr*)&s,size, hostname,sizeof(hostname),service,sizeof service,NI_NAMEREQD);

				if(remoteList[i].log_status==true)
				{
					 strcpy(log,"logged-in");
					//cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n",i+1, hostname, remoteList[i].num_sent, remoteList[i].num_recv, "logged-in")
					//cout<<i+1<<" "<<remoteList[i].IP<<" "<<remoteList[i].num_sent<<" "<<remoteList[i].num_recv<<" "<<"logged-in"<<endl<<flush;

				}
				else
				{	strcpy(log,"logged-out");
					//cout<<i+1<<" "<<remoteList[i].IP<<" "<<remoteList[i].num_sent<<" "<<remoteList[i].num_recv<<" "<<"logged-out"<<endl<<flush;
				}

			cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n",i+1, hostname,remoteList[i].num_sent, remoteList[i].num_recv,log);
			fflush(NULL);
			}
		}
		cse4589_print_and_log("[%s:END]\n","STATISTICS");
	}
	
	
	void printBlockList(char *IP)
	{
		char hostname[200],service[100];
                sockaddr_in s;
                socklen_t size=sizeof(s);
		int i;
		vector<ListNode>tempBlock;
		char command_str[]="BLOCKED";
		int index=findClient(IP);
		if(index!=-1 && remoteList[index].reg==true)
		{	cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
			fflush(NULL);

			for(i=0;i<remoteList[index].BlockList.size();i++)
			{	 int j=findClient(remoteList[index].BlockList[i].IP);
			   	  tempBlock.push_back(remoteList[j]);
			 	

			}
		
			sort(tempBlock.begin(),tempBlock.end(),mycomp);
			for(i=0;i<tempBlock.size();i++)
			{
				inet_aton(tempBlock[i].IP,&s.sin_addr);
                        	s.sin_family=AF_INET;
                        	getnameinfo((struct sockaddr*)&s,size, hostname,sizeof(hostname),service,sizeof service,NI_NAMEREQD);

				cse4589_print_and_log("%-5d%-35s%-20s%-8d\n",i+1,hostname,tempBlock[i].IP,tempBlock[i].port);
				fflush(NULL);
			}
			cse4589_print_and_log("[%s:END]\n", command_str);
			fflush(NULL);
		}
		else
		{	//Non-existent/Incorrect IP address

			cse4589_print_and_log("[%s:ERROR]\n", command_str);
			fflush(NULL);
			cse4589_print_and_log("[%s:END]\n", command_str);
			fflush(NULL);
		}
		
	}

	char *prepareBuffer (int rind,char *message)
	{
		char *mod=new char[1024];
		char* third=new char[1500];
		third=extractThird(message);	
		mod[0]=message[0];
		mod[1]=message[1];
		strcat(mod,remoteList[rind].IP);
		strcat(mod," ");
		strcat(mod,third);
		cout<<mod<<endl;	
		delete[] third;
		return mod;	 
	}

	bool isValidIP(char *IP)
	{
		//referred from beej's example on how to use inet_pton(),inet_ntop() functions

		struct sockaddr_in s;
		
		int i=inet_pton(AF_INET,IP,&(s.sin_addr));
		
		if(i==0)
		return false;

		return true;
		

	}
	void runServer()
	{
			
		fdmax=sockfd;
		FD_SET(STDIN,&master);
		FD_SET(sockfd,&master);
		char buf[500],remoteIP[INET6_ADDRSTRLEN];
		int buflen;
		int i;
		int index;
		while(true)
		{	//Select part from beej's example and Wenjun's material
			read_fds=master;
			//write_fds=master; //copy it --> why??
			if(select(fdmax+1,&read_fds,NULL,NULL,NULL)==-1)
			{
				perror("select: ");
				exit(-1);
			}
		//	if(fdmax,&read_fds)
		//	{ cout<<"I can be here only once "<<endl;}

			for(i=0;i<=fdmax;i++)
			{
				if(FD_ISSET(i,&read_fds))
				{
					if(i==sockfd)
					{	//vector<struct sockaddr>list=myList();
						addrlen=sizeof(remoteaddr);
						newfd=accept(sockfd,&remoteaddr,&addrlen);
						struct ListNode temp;
						 
						if(newfd==-1)
						{
							perror("accept: ");
							exit(-1);
						}
						else
						{	
							FD_SET(newfd,&master);
							(newfd>fdmax)?fdmax=newfd:fdmax;
							inet_ntop(AF_INET,&(((struct sockaddr_in*)&remoteaddr)->sin_addr),remoteIP,INET_ADDRSTRLEN);
							recv(newfd,&temp.port,sizeof(int),0);
							 
							 if((index=findClient(remoteIP))==-1)
							 {	strcpy(temp.IP,remoteIP);
								temp.sockfd=newfd;
								temp.num_sent=0;
								temp.num_recv=0;
								temp.reg=true;
								temp.log_status=true;
								//cout<<"IP"<<endl;						 
							 	remoteList.push_back(temp);
							 }
							else
							{	//cout<<"Second Login"<<endl;	
								remoteList[index].reg=true;
								remoteList[index].log_status=true;
								strcpy(remoteList[index].IP,remoteIP);
								remoteList[index].sockfd=newfd;
								remoteList[index].port=temp.port;
							}

							//sendbuffer(newfd,remoteIP);
							
							char list[500],*l;
							l=myList(list);		

							char type[]="L ";
							//cout<<list<<endl<<flush;
							sendall(newfd,type,list);
							
						}
					}
					else if(i==STDIN)
					{
						char input[500]="";
						
						
							fgets(input,sizeof(input),stdin);
							input[strlen(input)-1]='\0';
							//cout<<"Echo: "<<input<<endl;
							char *first=extractFirst(input);

							if(strcmp(first,"AUTHOR")==0)
							{	char command_str[]="AUTHOR";
								cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
								fflush(NULL);
								cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", "roshansh");
								fflush(NULL);
								cse4589_print_and_log("[%s:END]\n", command_str);
								fflush(NULL);
							}
						
							else if(strcmp(first,"IP")==0)
							{	
								
								myIP();
								
							}
			
							else if(strcmp(first,"PORT")==0)
							{	
								
								myPort();
								
							}
			
							else if(strcmp(first,"LIST")==0)
							{	char *p,l[500];
								p=myList(l);
								printList();
							}
							else if(strcmp(first,"STATISTICS")==0)
							{
								printStatistics();
								
								
							}
							else if(strcmp(first,"BLOCKED")==0)
							{	char command_str[]="BLOCKED";

								char *IP=extractSecond(input);
								
								if(isValidIP(IP)==true)
								{	printBlockList(IP);
								
								}
								else
								{
									cse4589_print_and_log("[%s:ERROR]\n", command_str);
									fflush(NULL);
									cse4589_print_and_log("[%s:END]\n", command_str);
									fflush(NULL);
								}
							}

							else if(strcmp(input,"EXIT")==0)
							{	
								close(sockfd);
								exit(-1);
							}	
						
					 }

						
					else
					{
						char* message;
						char response[1500];
						message=recvmessage(i,response);
						
						
						if(message[0]=='0')
						{	char list[500];
							char type[1024]="0 ";
							char* p=myList(list);
							//cout<<list<<endl<<flush;
							sendall(i,type,p);
						}
						else if(message[0]=='1')
						{	char m[1400];
							int send=findClient(i);
							remoteList[send].num_sent++;
							//char str[]="RELAYED";
							char *IP=extractSecond(message);
							char *third=extractThird(message);	
							int j=findClient(IP);
							if(j!=-1)
							{
								
								if(isBlocked(i,j)==false)
								{	char type[]="";
									
									strcpy(m,message);

									if(remoteList[j].log_status==true)
									{	char cmd[]="RELAYED";
										char *prep=prepareBuffer(send,m);
										sendall(remoteList[j].sockfd,prep,type);
										remoteList[j].num_recv++;
										//cout<<"In Relaying Send: "<<m<<endl;
										
										cse4589_print_and_log("[%s:SUCCESS]\n",cmd);
										fflush(NULL);
                                                                		cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n",remoteList[send].IP,remoteList[j].IP,third);
                                                                		fflush(NULL);
                                                        //message
                                                                		cse4589_print_and_log("[%s:END]\n",cmd);
                                                                		fflush(NULL);

										
									}
									else
									{
										struct message mbuf;
										//strcpy(mbuf.info,message);
										char *prep=prepareBuffer(send,message);
										strcpy(mbuf.info,prep);
										remoteList[j].buffer.push_back(mbuf);
										//cout<<"Am I here"<<endl<<flush;
									}
								}
								
							}
							else
							{
								strcpy(m,message);
								struct ListNode temp2;
								strcpy(temp2.IP,IP);
								temp2.reg=false;
								temp2.log_status=false;
								temp2.num_sent=0;
								temp2.num_recv=0;
								remoteList.push_back(temp2);
								int ind=findClient(temp2.IP);
								//cout<<ind<<" "<<remoteList[ind].IP<<"X"<<flush;
								if(ind!=-1)
								{	struct message mbuf;
									char *prep=prepareBuffer(send,message);
									strcpy(mbuf.info,prep);
									remoteList[ind].buffer.push_back(mbuf);
								}
								
								//cout<<"Am I here 2"<<endl<<flush;
							}
							
			

						}

						else if(message[0]=='2')
						{		int send=findClient(i);
								remoteList[send].num_sent++;
								bool flag=false;
								int client;
								char command_str[]="RELAYED";
							for(int j=1;j<=fdmax;j++)
							{	char trailer[]="";
								client=findClient(j);
								if(FD_ISSET(j,&master))
								{	
									if(j!=i && j!=sockfd )
									{	
									
										if(remoteList[client].log_status==true && isBlocked(i,client) == false)
										{	sendall(j,message,trailer);
								 			flag=true;
											
											remoteList[client].num_recv++;
										}	
									}
								}
								
								else if(client!=-1 && isBlocked(i,client)==false)
								{	//BUffering 

									struct message m;
									strcpy(m.info,message);
									remoteList[client].buffer.push_back(m);
								}
							}
							if(flag==true)
							{	char *third=extractThird(message);
								cse4589_print_and_log("[RELAYED:SUCCESS]\n");
								fflush(NULL);
								cse4589_print_and_log("msg from:%s, to:255.255.255.255\n[msg]:%s\n",remoteList[send].IP,third);
								fflush(NULL);
							//message
								cse4589_print_and_log("[RELAYED:END]\n",command_str);
								fflush(NULL);
							}

						}
						else if(message[0]=='3')
						{
							Block tempBlock;
							index=findClient(i);
							//if(index==-1)
							//cout<<"youre an invalid client"<<endl<<flush;
							if(index!=-1)
							{ char* second=extractSecond(message);
							  strcpy(tempBlock.IP,second);
							  remoteList[index].BlockList.push_back(tempBlock);

							}
						}

						else if(message[0]=='4')
						{

							Block tempBlock;
							index=findClient(i);
					
							//if(index==-1)
							//cout<<"youre an invalid client"<<endl<<flush;
							if(index!=-1)
							{
								char* second=extractSecond(message);
								
								removeBlock(i,second);
							}
						}
						else if(message[0]=='9')
						{
							int ind=findClient(i);
							remoteList[ind].log_status=false;
							remoteList[ind].reg=false;
							remoteList[ind].num_sent=0;
							remoteList[ind].num_recv=0;
							FD_CLR(i,&master);
							close(i);

						}
						else if(message[0]=='X')
						{
							int ind=findClient(i);
							//cout<<remoteList[ind].IP<<" logged out"<<endl;
							remoteList[ind].log_status=false;
							FD_CLR(i,&master);
							close(i);		
	
						}
						else if(message[0]=='B')
						{	int j=findClient(i);
							//cout<<"Am I getting this req "<<endl<<flush;
							//cout<<"Buffer Size"<<remoteList[j].buffer.size()<<endl<<flush;
							int c=-1;
							char *from,*message;
							char command_str[]="RELAYED";
							int size=remoteList[j].buffer.size();
							if(size>1)
							{	char type[1024]="B";
								sendall(remoteList[j].sockfd,type,remoteList[j].buffer[0].info);
								from=extractSecond(remoteList[j].buffer[0].info);
								message=extractThird(remoteList[j].buffer[0].info);
								remoteList[j].num_recv++;
								remoteList[j].buffer.erase(remoteList[j].buffer.begin());
								
								if(remoteList[j].buffer[0].info[0]=='2')
									c=1;
								else
									c=0;
						
							}
							
							else if(size==1)
							{	char type[1024]="F";
								sendall(remoteList[j].sockfd,type,remoteList[j].buffer[0].info);
								remoteList[j].num_recv++;
								from=extractSecond(remoteList[j].buffer[0].info);
								message=extractThird(remoteList[j].buffer[0].info);
								remoteList[j].buffer.erase(remoteList[j].buffer.begin());
							
								if(remoteList[j].buffer[0].info[0]=='2')
                                                                        c=1;
                                                                else
                                                                        c=0;
							}
							else 
							{
								//size =0;
								char type[]="N";
								char resp[]=" ";
								sendall(remoteList[j].sockfd,type,resp);
							}


							if(c==1)
							{
								//char *third=extractThird(message);
								cse4589_print_and_log("[RELAYED:SUCCESS]\n",command_str);
								fflush(NULL);
                                                                cse4589_print_and_log("msg from:%s, to:255.255.255.255\n[msg]:%s\n",from,message);
                                                                fflush(NULL);
                                                        //message
                                                                cse4589_print_and_log("[RELAYED:END]\n",command_str);
                                                                fflush(NULL);
								//Broadcast

							}
							else if(c==0) 
							{	//cout<<from<<endl;
								//char *third=extractThird(message);
								cse4589_print_and_log("[RELAYED:SUCCESS]\n");
								fflush(NULL);
                                                                cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n",from,remoteList[j].IP,message);
                                                                fflush(NULL);
                                                        //message
                                                                cse4589_print_and_log("[RELAYED:END]\n");
                                                                fflush(NULL);

							}

						} delete[] message;
					}
				}
			}
		}

	}
};


class Client
{
	vector<logList> L;
	vector<struct Block> MyBlock;
	int sockfd,p2pfd,fdmax,listsize;
	int port;
	char myAddr[20];
	struct addrinfo hints,*myres,*res,newhints;
	fd_set master,read_fds;
	bool loginflag;
 	struct sockaddr_in peer;	

	public:
	Client(char *arg)
	{	loginflag=false;
		port=atoi(arg);
		memset(&hints,0,sizeof(hints));
		hints.ai_flags=AI_PASSIVE;
		hints.ai_family=AF_INET;
		hints.ai_socktype=SOCK_STREAM;
		//Referred from Beej		
		if(getaddrinfo(NULL,arg,&hints,&myres)==-1)
		{
			perror("getaddrinfo: ");
			//exit(-1);
		}
		
		if((p2pfd=socket(AF_INET,SOCK_STREAM,0))==0)
		{

			perror("p2p socket: ");
			
		}

		memset(&peer,0,sizeof(peer));
                peer.sin_family=AF_INET;
		peer.sin_port=htons(port);
                peer.sin_addr.s_addr=htonl(INADDR_ANY);
		
		int yes=1;

		if(setsockopt(p2pfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))==-1)
                {
                        perror(" peer sockopt error: ");
                        //exit(-1);
                }


                if(bind(p2pfd,(struct sockaddr*)&peer,sizeof(peer))==-1)
                {
                        perror(" peer bind error: ");
                        //exit(-1);
                }

                if(listen(p2pfd,40)==-1)
                {
                        perror("peer listener error: ");
                        //exit(-1);
                }
	
	}


	void myPort()
        {
               
		char command_str[]="PORT";

		cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
		fflush(NULL);
		cse4589_print_and_log("PORT:%d\n",port);
		fflush(NULL);
		cse4589_print_and_log("[%s:END]\n", command_str);
		fflush(NULL);
        }


	void myIP()
        {
                struct ifaddrs *ifa,*p;
                char IP[2000];
		char command_str[]="IP";
                if(getifaddrs(&ifa)==-1)
                {
                        cse4589_print_and_log("[%s:ERROR]\n", command_str);
                        fflush(NULL);
			cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
			fflush(NULL);
                	
			return;
                }

                for(p=ifa;p!=NULL;p=p->ifa_next)
                {
                        struct sockaddr *s=p->ifa_addr;

                        if(s->sa_family==AF_INET)
                        {
                                inet_ntop(AF_INET,&(((struct sockaddr_in*)s)->sin_addr),IP,sizeof(IP));

                                if(strcmp(IP,"127.0.0.1")!=0)
                                {
					cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
					fflush(NULL);
					strcpy(myAddr,IP);
					cse4589_print_and_log("IP:%s\n", IP);
					fflush(NULL);
					cse4589_print_and_log("[%s:END]\n", command_str);
					fflush(NULL);
					break;
				}
                        }
                }

        }

	bool myIPAdd()
        {
                struct ifaddrs *ifa,*p;
                char IP[2000];
                char command_str[]="IP";
                if(getifaddrs(&ifa)==-1)
                {
                       // cse4589_print_and_log("[%s:ERROR]\n", command_str);
                       // cse4589_print_and_log("[%s:SUCCESS]\n", command_str);

                        return false;
                }

                for(p=ifa;p!=NULL;p=p->ifa_next)
                {
                        struct sockaddr *s=p->ifa_addr;

                        if(s->sa_family==AF_INET)
                        {
                                inet_ntop(AF_INET,&(((struct sockaddr_in*)s)->sin_addr),IP,sizeof(IP));

                                if(strcmp(IP,"127.0.0.1")!=0)
                                {
                                        //cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
                                        strcpy(myAddr,IP);
                                        //cse4589_print_and_log("IP:%s\n", IP);
                                        //cse4589_print_and_log("[%s:END]\n", command_str);
                                        return true;
                                }
                        }
                }

		return false;

        }


	char *extractFirst(char *input)
	{
		char *first=new char[30];
		int j=0;
		for(int i=0;input[i]!='\0'&&input[i]!=' ';i++,j++)
		 	first[j]=input[i];
		first[j]='\0';

		return first;
	}

	char *extractSecond(char * input)
	{
		char *second=new char[300];
		int j=0;
		int i;
		for(i=0;input[i]!=' '&&input[i]!='\0';i++);
	
		//for(;isalnum(input[i])==0&&input[i]!='\0';i++);
		i++;
		for(;input[i]!=' '&&input[i]!='\0';i++,j++)
			second[j]=input[i];

		second[j]='\0';
		
		return second;
	}
			
	char *extractThird(char *input)
	{
		char *third=new char[1000];
		int j=0;
		int i;
		for(i=0;input[i]!=' '&&input[i]!='\0';i++);

		//for(;isalnum(input[i])==0&&input[i]!='\0';i++);
			i++;
		for(;input[i]!=' '&&input[i]!='\0';i++);
		i++;
		//for(;isalnum(input[i])==0&&input[i]!='\0';i++);

		for(;input[i]!='\0';i++,j++)
			third[j]=input[i];
			
		third[j]='\0';

		return third;
	}

	char* broadExtract(char *input)
	{
		char *third=new char[1024];
                int j=0;
                int i;
                for(i=0;input[i]!=' '&&input[i]!='\0';i++);

                //for(;isalnum(input[i])==0&&input[i]!='\0';i++);
		i++;
                for(;input[i]!='\0';i++,j++)
                        third[j]=input[i];

                third[j]='\0';

                return third;



	}

	void splitList(char *p)
        {
		char str[500];
		strcpy(str,p);
                //cout<<str<<endl;
		char *printer;
		vector<char*>s;
		//int num=0;
             	printer=strtok(str,"-");
		//cout<<printer<<endl;
		//s.push_back(printer);
		//cout<<p<<endl;
		while(printer!=NULL)
		{
			s.push_back(printer);
			printer=strtok(NULL,"-");
			
		}

		//cout<<s[0]<<endl;
		 updateList(s); 
        }

	void updateList(vector<char*>s)
	{
		
		struct logList ll;
		L.clear();
		listsize=s.size();
		char str[500];
		char *p;
		for(int i=0;i<listsize;i++)
		{	strcpy(str,s[i]);
			//cout<<str<<endl;
			p=strtok(str," ");
			strcpy(ll.sno,p);
			p=strtok(NULL," ");
			strcpy(ll.name,p);
			p=strtok(NULL," ");
			strcpy(ll.IP,p);
			p=strtok(NULL," ");
			strcpy(ll.port,p);
			//cout<<ll.sno<<" "<<ll.name<<" "<<ll.IP<<" "<<flush;//<<ll[i].port<<endl;
		
			L.push_back(ll);
		}

	
	}
	
	char* recvmessage(int sockfd,char *message)
	{
		char *buf;
		char str[1500];
		int size;
		recv(sockfd,&size,sizeof(size),0);

		int total=0;
		int n;
		while(total<size)
		{
			n=recv(sockfd,str+total,size,0);
	
			if(n==-1)
			break;
		
			total+=n;
			size-=n;

		}

		str[total]='\0';

		//cout<<str<<endl;
		strcpy(message,str);
		//cout<<buf;
		return message;
	}
	
	void sendall(int sockfd,char *type,char* buf)
        {	char *message=new char[1048];
                strcat(type,buf);
		strcpy(message,type);
                int size=strlen(message);
                int n;
                int total=0;
                send(sockfd,&size,sizeof(size),0);
                while(total<size)
                {
                        n=send(sockfd,message+total,size,0);

                        if(n==-1)
                        break;

                        total+=n;
                        size-=n;
                }
		delete[] message;	
                //cout<<n<<"bytes were sent"<<endl<<flush;
        }

	void printList()
	{
		char command_str[]="LIST";
		cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
		fflush(NULL);
		
		for(int i=0;i<L.size();i++)
		cse4589_print_and_log("%-5d%-35s%-20s%-8s\n", i+1,L[i].name,L[i].IP,L[i].port);
		fflush(NULL);
		//cout<<L[i].sno<<" "<<L[i].name<<" "<<L[i].IP<<" "<<L[i].port<<endl<<flush;
		cse4589_print_and_log("[%s:END]\n", command_str);
		fflush(NULL);
	}


	 bool isValidIP(char *IP)
        {
                //referred from beej's example on how to use inet_pton(),inet_ntop() functions

                struct sockaddr_in s;

                int i=inet_pton(AF_INET,IP,&(s.sin_addr));

                if(i==0)
                return false;

                return true;


        }

	bool isValidPort(char *Portent)
	{
		bool flag=true;

		for(int i=0;i<strlen(Portent);i++)
			if(isdigit(Portent[i])==0)
				{ flag=false; break;}

		if(flag==false)
			return false;

		long v=atol(Portent);
		if(v<0 ||  v>65335)
			flag=false;

		if(flag==false)
			return false;
		return true;

	}

	bool isPresentInList(char *IP)
	{
		if(myIPAdd()==true)
		{	for(int i=0;i<L.size();i++)
			{
				if(strcmp(L[i].IP,IP)==0 && strcmp(L[i].IP,myAddr)!=0)
					return true;
			}
		}

		return false;
	}

	char *findPort(char* IP)
	{
		int port=-1;
		char *portchar;
		for(int i=0;i<L.size();i++)
		{
			if(strcmp(L[i].IP,IP)==0)
				{ portchar=L[i].port;
				   break;
				}
		}

		return portchar;
	
	}

	bool isBlocked(char *IP)
	{
		for(int i=0;i<MyBlock.size();i++)
			if(strcmp(MyBlock[i].IP,IP)==0)
			{
				return true;
			}

		return false;

	}

	bool removeBlock(char *IP)
	{
		for(int i=0;i<MyBlock.size();i++)
			if(strcmp(MyBlock[i].IP,IP)==0)
			{
				MyBlock.erase(MyBlock.begin()+i);

			}
	}

	void sendFile(int *sock,char *fileName)
	{
		FILE *fp;
		//cout<<fileName<<" "<<strlen(fileName)<<endl;	
		fp=fopen(fileName,"rb");
		int lsock=*sock;
		unsigned long nbytes;
		
		//send(lsock,&nbytes,sizeof(nbytes),0);
		
		//send(lsock,fileName,nbytes,0);

		if(fp==NULL)
		 {	//cout<<"File does not exist"<<endl;
			return ;
		 }

		fseek(fp,0L,SEEK_END);
		nbytes=ftell(fp);
		unsigned long nobytes=nbytes;
		//if(nbytes<MAX)
		//{
				
		//}
		//char size[10]=ltoa(nbytes);
		//int n=strlen(size);
		unsigned char c;
	//	cout<<nbytes<<endl;
		unsigned long total=0;
 
		//int size=sizeof(nbytes);
		//while(total<size)
		//{	
		send(lsock,&nbytes,sizeof(nbytes),MSG_DONTWAIT);
			//(n==-1)
			//break;

			//total+=n;
			

			
		fseek(fp,0L,SEEK_SET);
		
		unsigned long freads=1000;
		if(freads>nobytes)
		freads=nobytes;
		
		
		unsigned long tot=0;	
		//unsigned long nread=fread(c,1,nbytes,fp);
		//cout<<nbytes<<endl;	
		while(tot<nobytes)
		{	
			//cout<<"reading large file"<<endl;
			c=fgetc(fp);	
			//cout<<tot<<" "<<nobytes<<endl;
			//total=0;
			int n=send(lsock,&c,sizeof(c),0);

			
			tot+=n;
			//nbytes-=nread;

			//if(freads>nobytes)
			//freads=nobytes;
		}	
			
 
		//cout<<" bytes sent: "<<tot<<endl;
		fclose(fp);		
		int n=strlen(fileName);	
		send(lsock,&n,sizeof(n),MSG_DONTWAIT);
		send(lsock,fileName,n,MSG_DONTWAIT);
		//close(sock);
		//fclose(fp);
		//cout<<"end "<<endl;
			
		
	}

	void recvFile(int sock)
	{

		FILE *fp,*real;
		char filename[50];
		int n;	
		unsigned char c;
		int nsize;
		//recv(sock,&nsize,sizeof(nsize),0);
		//recv(sock,filename,nsize,0);
		//filename[nsize]='\0';
		unsigned long nbytes;
		recv(sock,&nbytes,sizeof(nbytes),0);
		//cout<<nbytes<<" ";

		
		
		unsigned long total=0;
		fp=fopen("temp","w+b");
				
			while(total<nbytes)
			{	//c=new unsigned char[6000];
			
		
			unsigned long n=recv(sock,&c,sizeof(c),0);
			if(n<=0)
			 { 	perror("error:");
				break;
			 }
			//char a='P';
			//send(sock,&a,sizeof(char),MSG_DONTWAIT);
			//cout<<n<<endl;
			//fwrite(c,1,n,stdout);
			//fwrite(c,1,n,fp);
			fputc(c,fp);
			//fflush(fp);
			total+=n;
			//delete[] c; 		
			}

			
		//cout<<total<<" "<<endl;
		fflush(fp);
		fclose(fp);
		int  nobytes;
		recv(sock,&nobytes,sizeof(nobytes),0);
		int t=0;
		int end=nobytes;
		while(t<nobytes)	
		{
			int n=recv(sock,filename,nobytes,0);
			t+=n;
			nobytes-=n;
		}
		filename[end]='\0';
		//rename("temp",filename);
		//cout<<filename<<flush<<endl;
		fp=fopen("temp","rb");
 		//fseek(fp, 0, SEEK_SET); 
		
	
		real=fopen(filename,"w+b");

		total=0;
		while(total<nbytes)
		{
			c=fgetc(fp);
			fputc(c,real);	
		//fread(c,1,total,fp);
		//fwrite(c,1,total,real);
		//fflush(real);
			total++;
		}
		//fflush(real);
		fclose(real);
		//fclose(fp);
		//fclose(fp);
		unlink("temp");	
	
		}

	void myClient()
	{	FD_ZERO(&master);
		fdmax=p2pfd;
		FD_SET(STDIN,&master);
		FD_SET(p2pfd,&master);
		char input[1024];
		int i;
		while(1)
		{	//cout<<"one last time"<<endl;
			read_fds=master;
			if(select(fdmax+1,&read_fds,NULL,NULL,NULL)==-1)
			{
				//perror("select: ");
				continue;
			}
			
			for(i=0;i<=fdmax;i++)
			{
				if(FD_ISSET(i,&read_fds))
				{
					 if(i==STDIN)
					{
						fgets(input,1023,stdin);
						input[strlen(input)-1]='\0';
						char *first,*second,*third;
						
						//cout<<input<<endl;		
						first=extractFirst(input);
						//cout<<first<<endl;
	
						if(strcmp(first,"IP")==0)
						{
							myIP();
						}
						else if(strcmp(first,"PORT")==0)
						{
							myPort();
						}
						else if(strcmp(first,"AUTHOR")==0)
						{	char command_str[]="AUTHOR";
							cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
							fflush(NULL);
							cse4589_print_and_log("I, roshansh, have read and understood the course academic integrity policy.\n");
							fflush(NULL);
							cse4589_print_and_log("[%s:END]\n",command_str);
							fflush(NULL);
						}
						else if(strcmp(first,"LOGIN")==0)
						{
							memset(&newhints,0,sizeof(newhints));
							newhints.ai_family=AF_INET;
							newhints.ai_socktype=SOCK_STREAM;
							//newhints.ai_flags=AI_PASSIVE;

							second=extractSecond(input);
							third=extractThird(input);
							loginflag=loginflag;
							//cout<<second<<endl<<third<<endl<<loginflag<<endl;
						   if(isValidIP(second)==true && isValidPort(third)==true)
							{
							   if(loginflag==false)	
						 	   {	if(getaddrinfo(second,third,&newhints,&res)==-1)
								{
									//perror("getaddrinfo: ");
									//exit(-1);
								}
								int yes=1;
							
								if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
								{
									//perror("socket: ");
									//exit(-1);
								}
						
							
								if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
 									//perror("setsockopt");
 									//exit(1);
								}

									
								if(connect(sockfd,res->ai_addr,res->ai_addrlen)==-1)
								{
									//perror("Connect error: ");
									//exit(-1);
								}

								FD_SET(sockfd,&master);
								//cout<<"Login Check: "<<endl;
								loginflag=true;
								(sockfd>fdmax)?fdmax=sockfd:fdmax;
								send(sockfd,&port,sizeof(port),0);
							    }
							   else
								{

								
                                                                cse4589_print_and_log("[LOGIN:ERROR]\n");
                                                                fflush(NULL);
                                                                cse4589_print_and_log("[LOGIN:END]\n");
                                                                fflush(NULL);


								}
							}
						   else
							{
								//char command_str[]="LOGIN";
								cse4589_print_and_log("[LOGIN:ERROR]\n");
								fflush(NULL);
                                                                cse4589_print_and_log("[LOGIN:END]\n");
                                                                fflush(NULL);


							}
						
						}
						else if(strcmp(first,"LIST")==0)
						{
							if(loginflag==true)
							{
								printList();
							}
							
						}

						else if(strcmp(first,"REFRESH")==0)
						{	char command_str[]="REFRESH";
							//cout<<"Inside refresh"<<endl;
							if(loginflag==true)
							{
								char request[1024]="0";
								char trailer[1024]=" ";
								sendall(sockfd,request,trailer);
								cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
								fflush(NULL);
                                                                cse4589_print_and_log("[%s:END]\n",command_str);
                                                                fflush(NULL);
							}
							else
							{
								cse4589_print_and_log("[%s:ERROR]\n", command_str);
								fflush(NULL);
                                                                cse4589_print_and_log("[%s:END]\n",command_str);
                                                                fflush(NULL);

							}

						}
						else if(strcmp(first,"SEND")==0)
						{	char command_str[]="SEND";
							second=extractSecond(input);
							if(isValidIP(second)==true && isPresentInList(second)==true && loginflag==true)
							{ 
								char request[1024]="1 ";
								strcat(request,second);
								strcat(request," ");
								third=extractThird(input);
								//cout<<third<<endl<<flush;
								sendall(sockfd,request,third);
								cse4589_print_and_log("[SEND:SUCCESS]\n");
								fflush(NULL);
								cse4589_print_and_log("[SEND:END]\n");
								fflush(NULL);
							}
							else
							{

								cse4589_print_and_log("[SEND:ERROR]\n");
								fflush(NULL);
                                                                cse4589_print_and_log("[SEND:END]\n");
                                                                fflush(NULL);

							}
							//cout<<"Input is fine: "<<input<<endl;
							//ut<<isValidIP(second)<<" "<<isPresentInList(second)<<endl;
						}
						else if(strcmp(first,"BROADCAST")==0)
						{
							third=broadExtract(input);
							char request[1024]="2 ";
							myIPAdd();
							strcat(request,myAddr);
							strcat(request," ");
							sendall(sockfd,request,third);

							char command_str[]="BROADCAST";

							 cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
							 fflush(NULL);
                                                         cse4589_print_and_log("[%s:END]\n",command_str);
                                                         fflush(NULL);
						}
						else if(strcmp(first,"BLOCK")==0)
						{	char command_str[]="BLOCK";
							
							second=extractSecond(input);
							if(isValidIP(second)==true && isPresentInList(second)==true && isBlocked(second)==false)
							  {	char request[]="3 ";
								struct Block temp;
								sendall(sockfd,request,second);
								strcpy(temp.IP,second);
								MyBlock.push_back(temp);
								cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
								fflush(NULL);
                                                                cse4589_print_and_log("[%s:END]\n",command_str);
                                                                fflush(NULL);

							  }
							else
							  {
								cse4589_print_and_log("[%s:ERROR]\n", command_str);
								fflush(NULL);
                                                                cse4589_print_and_log("[%s:END]\n",command_str);
                                                                fflush(NULL);

							  }
						}
						else if(strcmp(first,"UNBLOCK")==0)
						{	char command_str[]="UNBLOCK";
							
							second=extractSecond(input);
							if(isValidIP(second)==true && isPresentInList(second)==true && isBlocked(second)==true)
							{	char request[]="4 ";
								sendall(sockfd,request,second);
								removeBlock(second);
								cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
								fflush(NULL);
                                                                cse4589_print_and_log("[%s:END]\n",command_str);
                                                                fflush(NULL);

							}
							else
							{
								cse4589_print_and_log("[%s:ERROR]\n", command_str);
								fflush(NULL);
                                                                cse4589_print_and_log("[%s:END]\n",command_str);
                                                                fflush(NULL);

							}

							
					
						}
						else if(strcmp(first,"SENDFILE")==0)
						{
							struct addrinfo myhints,*pres;
							memset(&myhints,0,sizeof(myhints));
                                                        myhints.ai_family=AF_INET;
                                                        myhints.ai_socktype=SOCK_STREAM;
							int tempfd;
							char *clientIP=extractSecond(input);
							char *fileName=extractThird(input);
							char *pno=findPort(clientIP);
							if(getaddrinfo(clientIP,pno,&myhints,&pres)==-1)
                                                                {
                                                                        //perror("getaddrinfo: ");
                                                                        //exit(-1);
                                                                }
                                                                int yes=1;

                                                                if((tempfd=socket(AF_INET,SOCK_STREAM,0))==-1)
                                                                {
                                                                        //perror("socket: ");
                                                                        //exit(-1);
                                                                }


                                                                if (setsockopt(tempfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
                                                                        //perror("setsockopt");
                                                                        //exit(1);
                                                                }


                                                                if(connect(tempfd,pres->ai_addr,pres->ai_addrlen)==-1)
                                                                {
                                                                        //perror("Connect error: ");
                                                                        //exit(-1);
                                                                }
							//cout<<" "<<clientIP<<" "<<fileName<<" "<<pno<<" "<<endl;

							sendFile(&tempfd,fileName);
							cse4589_print_and_log("[%s:SUCCESS]\n", "SENDFILE");
                                                        fflush(NULL);
                                                        cse4589_print_and_log("[%s:END]\n","SENDFILE");
                                                        fflush(NULL);

							close(tempfd);
						}

									
						
						else if(strcmp(first,"LOGOUT")==0)
						{
							char command_str[]="LOGOUT";
							char request[]="X";
							char type[]=" ";
							loginflag=false;
							sendall(sockfd,request,type);
							FD_CLR(sockfd,&master);
							close(sockfd);
							cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
							fflush(NULL);
                                                        cse4589_print_and_log("[%s:END]\n",command_str);
                                                        fflush(NULL);

							
						}	
						else if(strcmp(first,"EXIT")==0)
						{
							char command_str[]="EXIT";
							char request[]="9";
							char type[]=" ";
							sendall(sockfd,request,type);
							FD_CLR(sockfd,&master);
							close(sockfd);
							cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
							fflush(NULL);
                                                        cse4589_print_and_log("[%s:END]\n",command_str);
                                                        fflush(NULL);
							exit(0);
						}
						
						
					}

					else if(i==sockfd)
					{	char r[500];
						char *m=recvmessage(sockfd,r);				
						if(m[0]=='0')
						{ 
						 splitList(m+2);
						 
						}
						 
						else if(m[0]=='1')
						{  
						   char command_str[]="RECEIVED";
						   char *sender=extractSecond(m);
						   char *message=extractThird(m);
						    cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
						    fflush(NULL);
						    cse4589_print_and_log("msg from:%s\n[msg]:%s\n",sender,message);
						    fflush(NULL);
                                                    cse4589_print_and_log("[%s:END]\n",command_str);
                                                    fflush(NULL);


						}
						else if(m[0]=='2')
						{	char *sender=extractSecond(m);
                                                 	char *message=extractThird(m);
							
						    char command_str[]="RECEIVED";
                                                    cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
                                                    fflush(NULL);
                                                    cse4589_print_and_log("msg from:%s\n[msg]:%s\n",sender ,message);
                                                    fflush(NULL);
                                                    cse4589_print_and_log("[%s:END]\n",command_str);
                                                    fflush(NULL);

						}
						else if(m[0]=='L')
						{
							splitList(m+2);
							char type[]="B";
							char req[]=" ";
							sendall(sockfd,type,req);
						}
						else if(m[0]=='B')
						{	char *sender=extractSecond(m+1);
                                                        char *message=extractThird(m+1);
							char command_str[]="RECEIVED";
                                                        cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
                                                        fflush(NULL);
                                                        cse4589_print_and_log("msg from:%s\n[msg]:%s\n",sender ,message);
                                                        fflush(NULL);
                                                        cse4589_print_and_log("[%s:END]\n",command_str);
                                                        fflush(NULL);

							char type[]="B";
							char req[]=" ";
							sendall(sockfd,type,req);
						}
						else if(m[0]=='F')
						{
							char *sender=extractSecond(m+1);
                                                        char *message=extractThird(m+1);
						    char command_str[]="RECEIVED";
                                                    cse4589_print_and_log("[%s:SUCCESS]\n", command_str);fflush(NULL);
                                                    cse4589_print_and_log("msg from:%s\n[msg]:%s\n",sender ,message);fflush(NULL);
                                                    cse4589_print_and_log("[%s:END]\n",command_str);fflush(NULL);

						    char command[]="LOGIN";
                                                    cse4589_print_and_log("[%s:SUCCESS]\n", command);fflush(NULL);
                                                    cse4589_print_and_log("[%s:END]\n",command);fflush(NULL);

						}
						else if(m[0]=='N')
						{
						    char command[]="LOGIN";
                                                    cse4589_print_and_log("[%s:SUCCESS]\n", command);fflush(NULL);
                                                    cse4589_print_and_log("[%s:END]\n",command);fflush(NULL);

						}
						
					}

					else if(i==p2pfd)
					{
						int newfd;
						sockaddr peeraddr;
						socklen_t plen=sizeof(peeraddr);
						 newfd=accept(p2pfd,&peeraddr,&plen);
						// cout<<"Connected "<<endl;
						if(newfd==-1)
						{
							perror("newfd :");
							continue;
						}
						else
						{
							recvFile(newfd);
							close(newfd);
						}

						
					}
						
				}
			}
		}
	}
};


//My actual main starts here

	if(argc==3)
	{
		if(strcmp(argv[1],"s")==0)
		{ Server S(argv[2]);
	  	S.runServer();
		}
		else if(strcmp(argv[1],"c")==0)
		{
			Client C(argv[2]);
			C.myClient();
		}
	}

	else
	cout<<"Enter right command"<<endl;

 return 0;
}
									

					
