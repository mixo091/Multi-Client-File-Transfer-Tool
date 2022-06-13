#include <stdio.h>
#include <iostream>
#include <string>
#include <pthread.h>
#pragma once 

using namespace std;


struct node
{
    char* file_name;
    int client_socket;
    struct node *next;
};
typedef struct node node;

class queue{
    node *front = NULL, *rear = NULL;
    int counter = 0;
    int size;
    public:
        queue(int size_){
            this->size =size_;
            cout<<"Queue Initialized"<<endl;
        }

        void enQueue(int client_socket ,char* file_name_)
        {   
            //cout<<"inserting" <<file_name_<<endl;
            struct node *newNode = ( struct node * ) malloc(sizeof(struct node));
            newNode->client_socket = client_socket;
            newNode->file_name = strdup(file_name_);
            newNode->next = NULL;

            //if it is the first node
            if(front == NULL && rear == NULL)
                //make both front and rear points to the new node
                front = rear = newNode;
            else
            {
                //add newnode in rear->next
                rear->next = newNode;

                //make the new node as the rear node
                rear = newNode;
            }
            counter++;
        }

        char* dequeue(int* client_socket )
        {
            //used to freeing the first node after dequeue
            struct node *temp;

            if(front == NULL)
                 return NULL;
            else
            {
                //take backup
                temp = front;

                //make the front node points to the next node
                //logically removing the front element
                front = front->next;

                //if front == NULL, set rear = NULL
                if(front == NULL)
                    rear = NULL;

               //free the first node
                *client_socket = temp->client_socket;
                char* file_name = strdup(temp->file_name);
               // cout << "Extracting "<<temp->file_name<<endl;
         

               free(temp);
               return file_name;
            }

        }




        void printList()
        {
            struct node *temp = front;
            cout << "___File Queue___"<<endl;
            if ( temp == NULL){
                return;
            }
            while(temp)
            {   

                cout<<"[" << temp->client_socket<<","<<temp->file_name<<"] <-";
                temp = temp->next;
            }
            printf("NULL\n");
        }
            
};



struct mutex_node{
    int client_socket;
    pthread_mutex_t socket_mutex;
    mutex_node* next;
};
typedef struct mutex_node mutex_node;

class mutex_list{
    
    public:
        mutex_node *head ;
        int size ;
        mutex_list(int size){
            cout<<"initialized"<<endl;
            head = NULL;
            size = size;  
        }
        void add_mutex(int client_socket){
            mutex_node* new_node = (mutex_node*)malloc(sizeof(mutex_node));
            new_node->socket_mutex = PTHREAD_MUTEX_INITIALIZER;
            new_node->client_socket = client_socket;
            if ( head == NULL){
                head = new_node;
            }else{
                new_node->next = head;
                head = new_node;
            }
            size++;
            return;
        }

        pthread_mutex_t * get_mutex(int client_socket){
            mutex_node* temp = head;
            while( temp != NULL){
                if( temp->client_socket == client_socket){
                    return &(temp->socket_mutex);
                  
                }
                temp = temp->next;
            }
            return NULL;
        }

        void print_state(){
            mutex_node* temp = head;
            while( temp != NULL){
                
                    cout<<"ClientSocket["<<temp->client_socket<<"]-";
                    
                
                temp = temp->next;
            }
            cout<<endl;
            return;
        }





};