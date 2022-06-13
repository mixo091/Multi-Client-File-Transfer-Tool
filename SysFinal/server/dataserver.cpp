#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <fstream>
#include <unistd.h>
#include <string>
#include <pthread.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dirent.h>
#include<inttypes.h>
#include <signal.h>

#include <thread>
#include "./Queue.h"
#define SERVERPORT 8096
#define SERVER_BACKLOG 100


#define QUEUE_SIZE 15
using namespace std;

//./dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>

pthread_mutex_t worker_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t socket_mutex=  PTHREAD_MUTEX_INITIALIZER ;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;
//pthread_t WorkingThreads[THREAD_POOL_SIZE];
//FileQueue Creation and it's mutex.
queue fileQueue(100);
mutex_list mutexes(0);

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cl_mutex = PTHREAD_MUTEX_INITIALIZER;
int block_size = 512;
int server_sock;



// Sending File to Client.
void send_file( FILE* fp,int client_socket,int block_size){
	char data[block_size] = { 0 };
	int n;
	while ( fgets(data , block_size , fp) != NULL){
		cout <<"[CONTENT] ::"<<data<<endl;
		if ((n = send(client_socket,data,sizeof(data),0)) == -1){
			cout<<"Error While Sending File "<<endl;
			exit(1);
		}
		//cout <<"wrote" <<n <<endl;
		bzero(data,block_size);
	}
	return;
}

// Function of Working Thread.
void* WorkOnFile(void* arg){
	char* file_name;
	int pclient;
	std::thread::id this_id = std::this_thread::get_id();
	//Wait if Queue is Empty.
	while(true){
	pthread_cond_wait(&condition_var, &worker_mutex);
    pthread_mutex_lock(&queue_mutex);
    file_name = fileQueue.dequeue(&pclient);
    pthread_mutex_unlock(&queue_mutex);
    pthread_mutex_t* client_mutex = mutexes.get_mutex(pclient);
    if (file_name != NULL) {
        cout<<"WorkingThread["<<this_id<<"]:Extracted :" << file_name <<endl;
    }

	pthread_mutex_lock(&(* client_mutex ));
	
	char* buff = "file";
   	int b_wroted = send(pclient, buff, block_size, 0);
    if ( b_wroted <= 0 ){cout<<"[Error]:something occured when Writing."<<endl;}
    buff = strdup(file_name);
    cout<<buff<<endl;
    cout<<"WorkingThread["<<this_id<<"]:Sending:" << file_name <<endl;
    b_wroted = send(pclient, buff, block_size, 0);
    if ( b_wroted <= 0 ){cout<<"[Error]:something occured when Writing."<<endl;}
 	FILE* fp = fopen(file_name,"r");
	if ( fp == NULL) { cout <<"Error Reading File"<<endl;exit(1);}
    send_file(fp,pclient,block_size);
	pthread_mutex_unlock(&(* client_mutex ));



	}

	return NULL;
}




void GetFiles(const char* dirname , queue& fileQueue , int client_socket) {
	// Open Requested directory.
    DIR* dir = opendir(dirname);
    if (dir == NULL) {
        return;
    }
    //printf("Reading ssfiles in: %s\n", dirname);
    struct dirent* entity;
    entity = readdir(dir);
    while (entity != NULL) {
        //printf("%hhd %s/%s\n", entity->d_type, dirname, entity->d_name);
        char file_name[100] = { 0 };
        char path[100] = { 0 };
        strcat(path, dirname);
        strcat(path, "/");
        strcat(path, entity->d_name);
        // Add File to Queue
        if ( entity->d_type == DT_REG ){
        	pthread_mutex_lock(&queue_mutex);
        	fileQueue.enQueue(client_socket , path);
        	pthread_mutex_unlock(&queue_mutex);
        	pthread_cond_signal(&condition_var);
        }

	    if (entity->d_type == DT_DIR && strcmp(entity->d_name, ".") != 0 && strcmp(entity->d_name, "..") != 0) {
            GetFiles(path,fileQueue,client_socket);
        }
        entity = readdir(dir);
    }

    closedir(dir);
}


//Communication Thread Functionality : Manage Request.
void * Manage_Request(void* arg){
	int client_socket = *((int*)arg);
	cout<<block_size<<endl;
	free(arg);
	char buffer[block_size] = { 0 };
	int read_n = read(client_socket, buffer, block_size);
    printf("[Server]:A Client Requested Directory : %s %d\n", buffer ,read_n);
    GetFiles(buffer,fileQueue,client_socket);
    return NULL;
}

void sig_int_handler(int sig){
	cout<<"[Server]:Shutting down .."<<endl;
	shutdown(server_sock, SHUT_RDWR);
	exit(1);

}


int main(int argc , char** argv){

	int server_port;
	int clients_connected = 0;
	int queue_size = 0 ;
	int Thread_pool_Size = 10 ;
	block_size = 1024;

	//___ Parse command line arguments. ___//
	if( argc != 9 ){
        cout<<"[Wrong Usage]: [Please:Follow This Format]: /dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>"<<endl;
        cout<<""<<endl;
        exit(1);
    }

    for( int i = 1 ; i < argc ; i=i+2){
        if(strcmp(argv[i],"-q") == 0){
            queue_size= atoi(argv[i+1]);
        }else if (strcmp(argv[i],"-p") == 0){
           	server_port = atoi(argv[i+1]);
        }else if (strcmp(argv[i],"-s") == 0){
            Thread_pool_Size = atoi(argv[i+1]);
        }else if (strcmp(argv[i],"-b") == 0){
            block_size = atoi(argv[i+1]);
        }
    }   

	// Thread Pool of WorkingThreads Creation.
	pthread_t WorkingThreads[Thread_pool_Size];
	for (int i = 0; i < Thread_pool_Size; i++) {
    	pthread_create(&WorkingThreads[i], NULL, WorkOnFile, NULL);
    }

    const int opt = 1 ;
    // Socket for listening creation.
    int server_socket , client_socket , addr_size;
	struct sockaddr_in server_addr , client_addr ;
	if ((server_socket = socket (AF_INET, SOCK_STREAM, 0)) < 0){
  		cerr<<"[Error] : creating the socket"<<endl;
  		exit(1);
 	}
 	server_sock=server_socket;
 	server_addr.sin_family = AF_INET;
 	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
 	server_addr.sin_port = htons(server_port);
 	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

 	// Bind && Listen steps.
 	if (bind (server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 ){
 		cerr<<"[Error]:binding socket"<<endl;
  		exit(1);
 	}
 	if ( listen (server_socket,  SERVER_BACKLOG) < 0 ){
 		cerr<<"[Error]:Unsuccesfull Listening"<<endl;
  		exit(1);
 	}

 	//SIGINT Handler.
	if (signal(SIGINT, sig_int_handler) == SIG_ERR){
    	cout<<"SIGINT ERROR"<<endl;
	}


 	//Server is Ready For Requests.
 	while(true){
 		cout<<"SERVER -- WAITING FOR CONNNECTIONS .."<<endl;
 		addr_size = sizeof(struct sockaddr_in);

 		// Accept new Connection.
 		if (  (client_socket = accept( server_socket ,  (struct sockaddr*)&client_addr , (socklen_t*)&addr_size ) ) < 0){
 			cout<<"[Error]:Connection Denied."<<endl;
 		}else{
 			
 			clients_connected ++;

 		}

 		//Create a Communication thread to Manage new Connection.
 		pthread_t t;
 		int * p = (int*)malloc(sizeof(int));
 		int * blk_size = (int*)malloc(sizeof(int));
 		*p = client_socket;
 		mutexes.add_mutex(client_socket);


    	std::string tmp = std::to_string(block_size);
    	const char  *num_char = tmp.c_str();

    	

    	pthread_mutex_t* client_mutex = mutexes.get_mutex(client_socket);
		pthread_mutex_lock(&(* client_mutex ));
		int b_wroted = send(client_socket, num_char, 1024, 0);
		if ( b_wroted <=0 ){
			cout<<"Error Writing Block_Size "<<endl;
		}
		pthread_mutex_unlock(&(* client_mutex ));
 		pthread_create(&t,NULL,Manage_Request,p);

 	}

	return 0  ;
}
