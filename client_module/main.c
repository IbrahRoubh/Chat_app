#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include<winsock2.h>//bind,accepte,listen,receive,send,socket
#include<pthread.h>

#define MAX_len 2000
#define PORT 8080

void creatWsaData();
void creatSocket();
void start_message();
void start_session(char *message);
void session();
void name_handel(char* name, int length);
void server_connect(char *ip);
void *receive_message(void *arg);

WSADATA wsa;
SOCKET server_socket;
struct sockaddr_in server;

int main(){
    char IP[]="";//enter the IP address where the server being luched
    char userName[100],buf[MAX_len];
    pthread_t t;//id for thread
    int re_size;

    start_message();

    scanf("%[^\n]s",userName);
    name_handel(userName,100);

    creatWsaData();
    creatSocket();
    server_connect(IP);

    send(server_socket, userName , strlen(userName) , 0);
    re_size=recv(server_socket,buf,sizeof(buf),0);
    buf[re_size]='\0';

    pthread_create(&t,NULL,receive_message,NULL);
    start_session(buf);
    return 0;
}
void creatSocket(){
	if((server_socket = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d" , WSAGetLastError());
		exit(1);
	}
}

void creatWsaData(){
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		printf("Failed. Error Code : %d",WSAGetLastError());
		exit(1);
	}
}

void start_message(){
    char message[]="\t\t\t\tWelcome to chatApp\nRules\n"
    "1-To start session enter your username, if you"
    " do not have an account just write a new username.\n"
    "2-TO send message enter(<recipient_name> <message_text>).\n"
    "3-To end the session enter(END_SESSION).\n"
    "\nUser name:";
    printf("%s",message);
}

void start_session(char *message){
    printf("\n\n\t\t\t\tSession Started\n");
    if(strcmp(message,"NONE")==0)
        printf("You have no waiting messages\n");
    else
        printf("%s",message);
    session();
}

void session(){
    char request[MAX_len],buf[MAX_len],message_build[2][1000];

    while(1){
        bool flag=false,inner_flag=true;
        int i=0,counter=0,pos,len;

        memset(request,'\0',sizeof(request));//clear the string
        memset(message_build[0],'\0',sizeof(message_build[0]));
        memset(message_build[1],'\0',sizeof(message_build[1]));
        //message=

        scanf(" %[^\n]s",request);//take the request from the user
        //<ibr> <hi>
        //END_SESSION
        // hi how are you

        if(strcmp(request,"END_SESSION")==0){exit(1);}//case: if the user want to end the session

        while(request[i]!='\0'){
            if(request[i]=='<'){
                i++;
                pos=i;
                len=0;
                while(request[i]!='>'){
                            len++;
                            i++;
                            if(request[i]=='\0')
                            {
                                inner_flag=false;
                                break;
                            }
                    }
                    if(inner_flag==true){
                    strncpy(message_build[counter],request+(pos),len);
                    counter++;
                    flag=true;
                    }else{
                        flag=false;
                    }
            }else if(request[i]!=' '){
                flag=false;
                break;
            }
            i++;
            if(flag==false||counter>2)
            {
                flag=false;//we set flag to false because if the counter>2; <1..> <2..> <3..>
                break;
            }
            }
    if(flag==false){
        printf("An Invalid Message(pleas check the rules, spelling, uppercase and lowercase letters)\n");
    }else{
        sprintf(buf,"%s: %s",message_build[0],message_build[1]);
        send(server_socket,buf,sizeof(buf),0);
    }
    }
}

void server_connect(char *ip){
    server.sin_addr.s_addr = inet_addr(ip);//IP
	server.sin_family = AF_INET;//IP family
	server.sin_port = htons( PORT );
    if (connect(server_socket , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		printf("connect error");
		exit(1);
	}
}

void name_handel(char* name, int length) {
    for (int i=0; i<length; i++) {
        if (name[i] == '\n') {
            name[i] = '\0';
            break;
        }
    }
    if (strlen(name) <2 || strlen(name) >= 30) {
        printf("\nName must be more than one and less than thirty characters.\n");
        exit(EXIT_FAILURE);
    }
}

void *receive_message(void *arg){
    int receive_size;
    char message[2000];
    while(1){
        receive_size=recv(server_socket,message,sizeof(message), 0);
        message[receive_size]='\0';
        if ( receive_size > 0 )
            printf("%s\n",message);
        else if( receive_size == 0 ){
            printf("Connection closed\n");
            exit(0);
        }
        else
        {
            printf("recv failed: %d\n", WSAGetLastError());
            exit(1);
        }
    }
}

