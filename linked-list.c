#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


pthread_mutex_t lista = PTHREAD_MUTEX_INITIALIZER;

struct Node {
    int data;
    struct Node* next;
};

int member(struct Node** head, int value){
    struct Node* current = *head;
    pthread_mutex_lock(&lista);
    while(current != NULL && current -> data < value){
        current = current -> next;
    }
    if(current != NULL && current -> data == value){
        pthread_mutex_unlock(&lista);
        return 1;
    }
    pthread_mutex_unlock(&lista);
    return 0;
}

int insert_linked_list(struct Node** head, int value){
    struct Node* temp;
    struct Node* pred = NULL;
    struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
    new_node -> data = value;
    pthread_mutex_lock(&lista);
    temp = *head;
    while(temp != NULL && temp-> data < value){
        pred = temp;
        temp = temp -> next;  
    }

    if(temp != NULL && temp->data == value){
        pthread_mutex_unlock(&lista);
        free(new_node);
        return 0;
    }

    if(pred == NULL){
        new_node -> next = *head;
        *head = new_node;
        pthread_mutex_unlock(&lista);
    }
    else{
        new_node -> next = temp;
        pred -> next = new_node;
        pthread_mutex_unlock(&lista);
    }
    return 1;
}

int delete(struct Node** head, int value){
    struct Node* current = *head;
    struct Node* pred = NULL;
    pthread_mutex_lock(&lista);
    while(current != NULL && current->data < value){
        pred = current;
        current = current -> next;
    }
    if(current == NULL || (current -> data) != value){
        pthread_mutex_unlock(&lista);
        return 0;
    }
    if(pred == NULL){
        pred = *head;
        *head = (*head) -> next;
        pthread_mutex_unlock(&lista);
        free(pred);
    }
    else{
        pred -> next = current -> next;
        pthread_mutex_unlock(&lista);
        free(current);
    }
    return 1;    
}
struct ThreadArgs {
    struct Node** head;
    int value;
};

void* thread_insert(void* arg) {
    struct ThreadArgs* args = (struct ThreadArgs*)arg;
    insert_linked_list(args->head, args->value);
    free(args);
    return NULL;
}

int main(){
    struct Node* head = NULL;
    #define N 8
    pthread_t threads[N];
    int valores[N] = {5, 1, 9, 3, 7, 2, 8, 4};

    for(int i = 0; i < N; i++){
        struct ThreadArgs* args = malloc(sizeof(struct ThreadArgs));
        args->head = &head;
        args->value = valores[i];
        
        if(pthread_create(&threads[i], NULL, thread_insert, (void*)args) != 0){
            perror("Error al crear thread");
            return 1;
        }
    }

    for(int i = 0; i < N; i++){
        pthread_join(threads[i], NULL);
    }

    printf("Lista enlazada ordenada: ");
    struct Node* current = head;
    while(current != NULL){
        printf("%d -> ", current->data);
        current = current->next;
    }
    printf("NULL\n");

    current = head;
    while(current != NULL){
        struct Node* temp = current;
        current = current->next;
        free(temp);
    }

    return 0;
}