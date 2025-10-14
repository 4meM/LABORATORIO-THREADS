#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

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

// ============================================================================
// CONFIGURACION DEL BENCHMARK
// ============================================================================

#define CLAVES_INICIALES 1000
#define TOTAL_OPERACIONES 100000
#define PORCENTAJE_MEMBER 80
#define PORCENTAJE_INSERT 10
#define PORCENTAJE_DELETE 10
#define VALOR_MAXIMO_CLAVE 65536

typedef enum {
    OPERACION_MEMBER,
    OPERACION_INSERT,
    OPERACION_DELETE
} TipoOperacion;

typedef struct {
    struct Node** cabeza_lista;
    int operaciones_por_hilo;
    int id_hilo;
    TipoOperacion* operaciones;
} ArgumentosHiloBenchmark;

double obtener_tiempo_microsegundos() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)(tv.tv_sec) * 1000000.0 + (double)(tv.tv_usec);
}

TipoOperacion obtener_operacion_aleatoria() {
    int valor_aleatorio = rand() % 100;
    
    if (valor_aleatorio < PORCENTAJE_MEMBER) {
        return OPERACION_MEMBER;
    } else if (valor_aleatorio < PORCENTAJE_MEMBER + PORCENTAJE_INSERT) {
        return OPERACION_INSERT;
    } else {
        return OPERACION_DELETE;
    }
}

// Genera un array con la distribución exacta de operaciones (80-10-10%)
TipoOperacion* generar_operaciones_exactas(int total_operaciones) {
    TipoOperacion* operaciones = malloc(total_operaciones * sizeof(TipoOperacion));
    
    // Calcular cantidades exactas
    int num_member = (total_operaciones * PORCENTAJE_MEMBER) / 100;
    int num_insert = (total_operaciones * PORCENTAJE_INSERT) / 100;
    int num_delete = total_operaciones - num_member - num_insert;
    
    // Llenar el array con las operaciones en orden
    int index = 0;
    for (int i = 0; i < num_member; i++) {
        operaciones[index++] = OPERACION_MEMBER;
    }
    for (int i = 0; i < num_insert; i++) {
        operaciones[index++] = OPERACION_INSERT;
    }
    for (int i = 0; i < num_delete; i++) {
        operaciones[index++] = OPERACION_DELETE;
    }
    
    // Mezclar aleatoriamente (algoritmo Fisher-Yates)
    for (int i = total_operaciones - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        TipoOperacion temp = operaciones[i];
        operaciones[i] = operaciones[j];
        operaciones[j] = temp;
    }
    
    return operaciones;
}

void inicializar_lista(struct Node** cabeza, int numero_claves) {
    for (int i = 0; i < numero_claves; i++) {
        int valor_aleatorio = rand() % VALOR_MAXIMO_CLAVE;
        insert_linked_list(cabeza, valor_aleatorio);
    }
}

void liberar_lista(struct Node* cabeza) {
    struct Node* actual = cabeza;
    while (actual != NULL) {
        struct Node* temporal = actual;
        actual = actual->next;
        free(temporal);
    }
}

// Ejecuta una operación sobre la lista
void ejecutar_operacion(struct Node** cabeza, TipoOperacion tipo_op, int valor) {
    switch (tipo_op) {
        case OPERACION_MEMBER:
            member(cabeza, valor);
            break;
        case OPERACION_INSERT:
            insert_linked_list(cabeza, valor);
            break;
        case OPERACION_DELETE:
            delete(cabeza, valor);
            break;
    }
}

// Ejecutar cada hilo del benchmark
void* funcion_hilo_benchmark(void* argumento) {
    ArgumentosHiloBenchmark* args = (ArgumentosHiloBenchmark*)argumento;
    
    for (int i = 0; i < args->operaciones_por_hilo; i++) {
        TipoOperacion tipo_operacion = args->operaciones[i];
        int valor = rand() % VALOR_MAXIMO_CLAVE;
        ejecutar_operacion(args->cabeza_lista, tipo_operacion, valor);
    }
    
    return NULL;
}

// Ejecuta el benchmark con un numero de hilos N
double ejecutar_benchmark(int numero_hilos) {
    struct Node* cabeza = NULL;
    pthread_t* hilos = malloc(numero_hilos * sizeof(pthread_t));
    ArgumentosHiloBenchmark* args_hilos = malloc(numero_hilos * sizeof(ArgumentosHiloBenchmark));
    
    inicializar_lista(&cabeza, CLAVES_INICIALES);
    
    TipoOperacion* todas_operaciones = generar_operaciones_exactas(TOTAL_OPERACIONES);
    
    int operaciones_por_hilo = TOTAL_OPERACIONES / numero_hilos;
    
    double tiempo_inicio = obtener_tiempo_microsegundos();
    
    for (int i = 0; i < numero_hilos; i++) {
        args_hilos[i].cabeza_lista = &cabeza;
        args_hilos[i].operaciones_por_hilo = operaciones_por_hilo;
        args_hilos[i].id_hilo = i;
        args_hilos[i].operaciones = &todas_operaciones[i * operaciones_por_hilo];
        
        if (pthread_create(&hilos[i], NULL, funcion_hilo_benchmark, &args_hilos[i]) != 0) {
            perror("Error al crear hilo");
            exit(1);
        }
    }
    
    for (int i = 0; i < numero_hilos; i++) {
        pthread_join(hilos[i], NULL);
    }
    
    double tiempo_fin = obtener_tiempo_microsegundos();
    double tiempo_transcurrido = (tiempo_fin - tiempo_inicio) / 1000000.0;
    
    liberar_lista(cabeza);
    free(todas_operaciones);
    free(hilos);
    free(args_hilos);
    
    return tiempo_transcurrido;
}

void imprimir_configuracion_benchmark() {
    printf("========================================\n");
    printf("BENCHMARK DE LISTA ENLAZADA\n");
    printf("========================================\n");
    printf("Claves iniciales: %d\n", CLAVES_INICIALES);
    printf("Total de operaciones: %d\n", TOTAL_OPERACIONES);
    printf("Mezcla de operaciones:\n");
    printf("  - Member (buscar): %d%%\n", PORCENTAJE_MEMBER);
    printf("  - Insert (insertar): %d%%\n", PORCENTAJE_INSERT);
    printf("  - Delete (eliminar): %d%%\n", PORCENTAJE_DELETE);
    printf("========================================\n\n");
}

void imprimir_resultados(int cantidad_hilos[], double tiempos[], int numero_pruebas) {
    printf("RESULTADOS:\n");
    printf("----------------------------------------\n");
    printf("Hilos | Tiempo (s) | Velocidad | Eficiencia\n");
    printf("------|------------|---------|------------\n");
    
    double tiempo_base = tiempos[0];
    
    for (int i = 0; i < numero_pruebas; i++) {
        double speedup = tiempo_base / tiempos[i];
        double eficiencia = (speedup / cantidad_hilos[i]) * 100.0;
        
        printf("%5d | %10.4f | %7.2fx | %9.2f%%\n", 
               cantidad_hilos[i], tiempos[i], speedup, eficiencia);
    }
    
    printf("----------------------------------------\n");
}

int main() {
    srand(time(NULL));
    
    int cantidad_hilos[] = {1, 2, 4, 8};
    int numero_pruebas = 4;
    double tiempos[4];
    
    imprimir_configuracion_benchmark();
    
    for (int i = 0; i < numero_pruebas; i++) {
        fflush(stdout);
        tiempos[i] = ejecutar_benchmark(cantidad_hilos[i]);
    }
    
    printf("\n");
    imprimir_resultados(cantidad_hilos, tiempos, numero_pruebas);
    
    return 0;
}