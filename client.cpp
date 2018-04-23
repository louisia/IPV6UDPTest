#include <stdio.h>  
#include <arpa/inet.h>  
#include <unistd.h>  
#include <sys/socket.h>  
#include <string.h>  
#define REMOTEPORT 9000  
#define REMOTEIP "2001:250:5032:e:20c:29ff:fe90:1efc"
int main(int argc,char *argv[])  
{  
    int mysocket,len;  
    int i=0;  
    struct sockaddr_in6 addr;  
    int addr_len;  
    char msg[200];  
    if((mysocket=socket(AF_INET6,SOCK_DGRAM,0))<0)  
    {  
        perror("error:");  
        return(1);  
    }  
    else  
    {  
        printf("socket created ...\n");  
        printf("socket id :%d \n",mysocket);  
        printf("rmote ip : %s\n",REMOTEIP);  
        printf("remote port :%d \n",REMOTEPORT);  
    }  
  
    addr_len=sizeof(struct sockaddr_in6);  
    bzero(&addr,sizeof(addr));  
    addr.sin6_family=AF_INET6;  
    addr.sin6_port=htons(REMOTEPORT);  
    inet_pton(AF_INET6,REMOTEIP,&addr.sin6_addr);  
  
    while(1)  
    {  
        bzero(msg,sizeof(msg));  
        len=read(STDIN_FILENO,msg,sizeof(msg));  
        if(sendto(mysocket,msg,sizeof(msg),0,(struct sockaddr *)&addr,addr_len)<0)  
        {  
            printf("error");  
            return(1);  
        }  
        len=recvfrom(mysocket,msg,sizeof(msg),0,(struct sockaddr *)&addr,(socklen_t*)&addr_len);  
        printf("%d:",i);  
        i++;  
        printf("Received message : %s\n",msg);  
    }  
} 
