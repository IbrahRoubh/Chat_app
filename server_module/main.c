#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<time.h>
#include<winsock2.h>//socket function
#include<pthread.h>//thread function


#define PORT 8080//the port number should be the same with client side

typedef struct client{
    char user_name[30];//store user name
    char massege[2000];//to store the messages when the user are offline
    struct client* next;
}Client;//linked list for all client

typedef struct active_client{
    char user_name[30];//store user name
    SOCKET user_socket;//store user socket
    struct active_client* next;
}Active_client;//linked list for active client

//functions
void creat_socket();
void Bind();
Client *search_user(char *name);
void Add(char *name);
Active_client *Add_active_c(char *name,SOCKET s);
void* accept_client(void *arg);
void* receive_message(void *arg);
void printlistActive();
void delet_activ_client(SOCKET s);
void message_to_send(char *ms,SOCKET s);
SOCKET search_avtive_c(char *name);
void extraction_recive_name(char *message,char *name);
void extraction_message_content(char *message,char *mc);



Client *c_list;//linked list for all client
Active_client *A_C_list;//linked list for active client
SOCKET server_socket,client_socket;
struct sockaddr_in server_addr,client_addr;


int main(int argc , char *argv[])
{
    WSADATA wsa;
    pthread_t t;
    int c;
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0){
		printf("Failed. Error Code : %d",WSAGetLastError());
		return 1;
		}
    creat_socket();
	Bind();
	if(listen(server_socket,4)<0){
        printf("listen filed\n");
        exit(1);
    }

    c_list=search_user("head");//create the head of the list
    A_C_list=Add_active_c("head",000);//create the head of the list

    c = sizeof(struct sockaddr_in);

	printf("Waiting for incoming connections...\n");
    while(1)//this while loop will run until we close our server
	{
	     client_socket= accept(server_socket,(struct sockaddr *)&client_addr, &c);
	     if(client_socket==INVALID_SOCKET){
            break;
	     }
		pthread_create(&t,NULL,receive_message,client_socket);
	}

	//close connect
    closesocket(server_socket);
	WSACleanup();
    return 0;
}

void creat_socket(){
    if((server_socket = socket(AF_INET , SOCK_STREAM ,0)) == INVALID_SOCKET){
		printf("Could not create socket : %d" , WSAGetLastError());
		exit(1);
	}
}

void Bind(){
    server_addr.sin_family =AF_INET;//type of IP address
	server_addr.sin_addr.s_addr = INADDR_ANY;//set the IP address //INADDR_ANY return the device IP
	server_addr.sin_port = htons( PORT );//set the port number 8080
    if( bind(server_socket ,(struct sockaddr *)&server_addr , sizeof(server_addr)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d" , WSAGetLastError());
		exit(1);
	}
}

void* receive_message(void *arg){
    SOCKET *c_socket=(SOCKET *)arg;
    int receive_size;
    char message[2000];

    take_username(c_socket);
    while(1){
        receive_size=recv(c_socket,message,sizeof(message), 0);
        message[receive_size]='\0';
        if ( receive_size > 0 )
            printf("Message received: %s\n",message);
        else if( receive_size == 0 )
        {
            printf("Connection closed\n");
            delet_activ_client(c_socket);
            break;
        }
        else
        {
            printf("recv failed: %d\n", WSAGetLastError());
            delet_activ_client(c_socket);
            break;
        }
        message_to_send(message,c_socket);
    }
}

void take_username(SOCKET cs){
    int receive_size;
    char username[200],BUFF[2000];
    Client *c;

    receive_size= recv(cs,username,sizeof(username),0);
    username[receive_size]='\0';

    c=search_user(username);
    if(strlen(c->massege)==0){
        strcpy(BUFF,"NONE");
    }else{
        strcpy(BUFF,c->massege);
    }
    send(cs,BUFF,sizeof(BUFF),0);
    strcpy(c->massege,"");
    Add_active_c(username,cs);
    printlistActive();
}


Client *search_user(char *name){
    if(c_list==NULL){
            Add(name);
            return c_list;
    }//if their is no client in the list
    Client *temp=c_list;
    while(temp!=NULL){
        if(strcmp(temp->user_name,name)==0){
            return temp;
        }
        temp=temp->next;
    }
    Add(name);
    return search_user(name);
}

void Add(char *name){
    Client *newclient=NULL;
    newclient=malloc(sizeof(Client));
    if(newclient==NULL){
        printf("error, can not allocate memorey!!\n");
        exit(1);
    }
    strncpy(newclient->user_name, name, 30);
    newclient->next=NULL;
    if(c_list==NULL){
        c_list=newclient;
        return;
    }
    Client *tempe=c_list;
    while(tempe->next!=NULL){
        tempe=tempe->next;
    }
    tempe->next=newclient;
}

Active_client *Add_active_c(char *name,SOCKET s){
    Active_client *newActive_c=malloc(sizeof(Active_client));
    if (newActive_c==NULL){
            printf("can not allocate memorey for client!!\n");
            return;
    }
    newActive_c->next=NULL;
    newActive_c->user_socket=s;
    strncpy(newActive_c->user_name,name,30);
    if(A_C_list==NULL){
        return newActive_c;
    }
    Active_client *tem=A_C_list;
    while(tem->next!=NULL){
            tem=tem->next;
    }
    tem->next=newActive_c;
    return A_C_list;
}

SOCKET search_avtive_c(char *name){
    Active_client *tem=A_C_list;
    while(tem!=NULL){
            if(strcmp(name,tem->user_name)==0){
                return tem->user_socket;
            }
            tem=tem->next;
    }
    return NULL;//if we can not find the recive in online
}

void delet_activ_client(SOCKET s){
    Active_client *prevc=A_C_list,*cur=A_C_list;
    //head->ibr->ali->os
    while(cur!=NULL){
            if(s==cur->user_socket){
                prevc->next=cur->next;
                free(cur);
                return ;
            }
            prevc=cur;
            cur=cur->next;
    }
}

void message_to_send(char *ms,SOCKET s){
    SOCKET res;
    char resiv_name[30],*sender_name,message_con[1800],final_message[2000];
    time_t now=time(NULL);
    struct tm *time_struct=gmtime(&now);

    search_username_by_socket(s,sender_name);//search about the sender name
    extraction_recive_name(ms,resiv_name);
    extraction_message_content(ms,message_con);
    sprintf(final_message,"%d:%d:%d-%s%s\n",time_struct->tm_hour,time_struct->tm_min,time_struct->tm_sec,sender_name,message_con);


    res=search_avtive_c(resiv_name);
    if(res!=NULL){
            send(res,final_message,sizeof(final_message),0);
    }else{
        Client *c=search_user(resiv_name);
        strcat(c->massege,final_message);
    }
}

void search_username_by_socket(SOCKET s,char *name){
    Active_client *tem=A_C_list;
    while(tem!=NULL){
        if(s==tem->user_socket){
            strcpy(name,tem->user_name);
            return;
        }
        tem=tem->next;
    }
}

void extraction_recive_name(char *message,char *name){
    int len=0;
        while(message[len]!=':'){
        len++;
    }
    strncpy(name,message,len);
}

void extraction_message_content(char *message,char *mc){
    int len=0,len2;
        while(message[len]!=':'){
        len++;
    }
    len2=len;
    while(message[len2]!='\0'){
        len2++;
    }
    strncpy(mc,message+len,len2);
}

void printlist(){
    Client *temp=c_list;
    while(temp!=NULL){
        printf("%s->",temp->user_name);
        temp=temp->next;
    }
    printf("NULL\n");
}

void printlistActive(){
    Active_client *temp=A_C_list;
    printf("active list:");
    while(temp!=NULL){
        printf("%s->",temp->user_name);
        temp=temp->next;
    }
    printf("NULL\n");
}
