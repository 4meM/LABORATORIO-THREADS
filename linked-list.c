#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


pthread_mutex_t lista = PTHREAD_MUTEX_INITIALIZER;

struct Node {
    int data;
    struct Node* next;
};

int insert_linked_list(struct Node** head, int value){
    struct Node* temp;
    pthread_mutex_lock(&lista);
    temp = *head;
    while(temp != NULL && temp-> data < value){
        temp = temp -> next;  
    }

    if(temp != NULL && temp -> data == value){
        struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
        new_node -> data = value;
    }
    pthread_mutex_unlock(&lista);

}


int main(){

}