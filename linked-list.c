#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


struct Node {
    int data;
    struct Node* next;
};

int insert_linked_list(struct Node** head, int value){
    struct Node* temp;
    pthread_mutex_lock(&head);
    temp = head;

    while(temp != NULL || temp-> data < value){
        
    }



}


int main(){

}