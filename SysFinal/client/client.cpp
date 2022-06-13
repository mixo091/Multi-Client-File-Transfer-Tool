#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fstream>
#include <string.h>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <string>
#include <cstddef>
#include <bits/stdc++.h>
#include <sys/stat.h>
#include <fstream>
#include <dirent.h>
#include<inttypes.h>
#include <signal.h>

using std::cout; using std::ofstream;
using std::endl; using std::string;
using std::cerr;
using std::fstream;
using namespace std;
int sockfd;


string build_path(char* path){
    string dir_builder;
    string str(path);
    string path_str = str.substr(3);
    std::cout << " [PATH]: " << path_str << '\n';
    int check;
    
    size_t found = path_str.find_first_of("/");
    while( found!=std::string::npos ){
        std::cout << " Check: " << path_str.substr(0,found) << '\n';
        check = mkdir(path_str.substr(0,found).c_str(),0777);
  
    // check if directory is created or not
    if (!check)
        printf("Directory created\n");
    else {
        printf("Unable to create directory\n");
     
    }
        found = path_str.find_first_of("/",found+1);
        //cout<<"found : "<<found<<endl;
   }
    
    return path_str;
}



void sig_int_handler(int sig){
    cout<<"Closing Connection.."<<endl;
    close(sockfd);
    exit(1);

}


int main(int argc, char const* argv[]){

    if (signal(SIGINT, sig_int_handler) == SIG_ERR){
        cout<<"SIGINT ERROR"<<endl;
    }

    char * Directory;
    char* server_ip;
    int server_port;
    int BUFFSIZE = 1024;
   
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    //Parse Command Line Arguments.
    if( argc != 7 ){
        cout<<"[Wrong Usage]: [Please:Follow This Format]: "<<endl;
        cout<<"./remoteClient -i <server_ip> -p <server_port> -d <directory>"<<endl;
        exit(1);
    }


    for( int i = 1 ; i < argc ; i=i+2){

        if(strcmp(argv[i],"-i") == 0){
            servaddr.sin_addr.s_addr= inet_addr(argv[i+1]);
        }else if (strcmp(argv[i],"-p") == 0){
            servaddr.sin_port =  htons(atoi(argv[i+1]));
        }else if (strcmp(argv[i],"-d") == 0){
            Directory = strdup(argv[i+1]);
        }
    }


    cout<<"Directory:"<<Directory<<endl;



    sleep(1);
    //Create a socket for the client.
    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
        cerr<<"Socket Creation Failed."<<endl;
        exit(2);
    }


    //Connection of the client to the socket
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
        cerr<<"Connection Failed."<<endl;
        exit(3);
    }else{
        cout<<"Connected"<<endl;
    }



    char s[1024] = { 0 };
    int read_n = read(sockfd , s, 1024);
    printf("[Server]:Requested Directory : %s %d\n", s ,read_n);
    int block_size = atoi(s);
    cout<<"block_size:" << block_size <<endl;
    //Send Directory you want downloaded.
    
    int n;
    
    if ( (n =send(sockfd , Directory , strlen(Directory),0)) <= 0 ){
        cerr<<"Writing Failed."<<endl;
        exit(3);
    }
    cout<< n <<endl;


    char buffer[block_size] = { 0 };



    int bytes_read;
    char* file_name;
    string path;
    fstream MyFile;

    while( true ){

        bytes_read = read (sockfd, buffer, block_size) ;
        cout << "Bytes read :"<<bytes_read <<endl;

        

        if( strcmp(buffer , "file") == 0 ){


            if (MyFile.is_open()) {
                cout<<"[CLOSE] " <<file_name<<endl;
               MyFile.close();}else{cout<<"[ALREADY CLOSED]"<<endl;}
         
            
            bzero(buffer , block_size);
            bytes_read = read (sockfd, buffer,block_size) ;
            cout << "Bytes read :"<<bytes_read <<endl;
            cout<<"[FILE]:"<<buffer<<endl;
            file_name = strdup(buffer);
            path = build_path(buffer);
            file_name = strdup(path.c_str());
            cout<<"[CREATE]"<<file_name<<endl;
            MyFile.open(file_name,std::ios::app);
            bzero(buffer , block_size);

        }else{

            cout<<"["<<file_name<<":CONTENT]"<<endl;
            //cout << buffer<< endl;
            cout<<"[OPEN]"<<file_name<<endl;
            if (MyFile.is_open()) {
                cout<<"[OPENED]"<<file_name<<endl;
                cout<<"[APPEND]"<<buffer<<endl;
                MyFile << buffer ;
            }
          
            bzero(buffer , block_size);

        }



    }

return 0;
}
