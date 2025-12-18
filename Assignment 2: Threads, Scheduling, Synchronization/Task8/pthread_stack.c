#include <pthread.h> 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdatomic.h>
     
int num_threads = 0;
int node_id_counter = 0; 
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t stack_mutex = PTHREAD_MUTEX_INITIALIZER;
     
typedef struct node { 
     int node_id;      
     struct node *next;
} Node;

Node *top; 

//Generate unique node ID
int get_next_node_id() {
    pthread_mutex_lock(&counter_mutex);
    int id = node_id_counter++;
    pthread_mutex_unlock(&counter_mutex);
    return id;
}

/*Option 1: Mutex Lock*/
void push_mutex() { 
     Node *new_node = malloc(sizeof(Node));
     if (new_node == NULL) {
         perror("malloc failed");
         return;
     }
     
     new_node->node_id = get_next_node_id();
     
     pthread_mutex_lock(&stack_mutex);
     new_node->next = top;
     top = new_node;
     pthread_mutex_unlock(&stack_mutex);
     
     printf("Pushed node %d\n", new_node->node_id);
}


int pop_mutex() { 
     pthread_mutex_lock(&stack_mutex);
     if(top == NULL) {
         pthread_mutex_unlock(&stack_mutex);
         printf("Stack empty, cannot pop\n");
         return -1; // Indicate stack is empty
     }

     Node *old_node= top;
     int node_id = old_node->node_id;
     top = top->next;
     pthread_mutex_unlock(&stack_mutex);

     free(old_node);
     printf("Popped node %d\n", node_id);
     return node_id;
}

/*Option 2: Compare-and-Swap (CAS)*/
void push_cas() { 
     Node *new_node = malloc(sizeof(Node));
     if (new_node == NULL) {
         perror("malloc failed");
         return;
     }
     
     new_node->node_id = get_next_node_id();
     Node *old_top;
     
     do {
         old_top = top;
         new_node->next = old_top;
     } while (!__sync_bool_compare_and_swap(&top, old_top, new_node));
     
     printf("Pushed node %d\n", new_node->node_id);
}

int pop_cas() { 
     Node *old_top;
     Node *new_top;
     
     do {
         old_top = top;
         if (old_top == NULL) {
             printf("Stack empty, cannot pop\n");
             return -1;
         }
         new_top = old_top->next;
     } while (!__sync_bool_compare_and_swap(&top, old_top, new_top));
     
     int node_id = old_top->node_id;
     free(old_top);
     printf("Popped node %d\n", node_id);
     return node_id;
}

/* the thread function */
void *thread_func(void *arg) { 
     /* Assign each thread an id so that they are unique in range [0, num_thread -1 ] */
     int opt = *((int *)arg);
     int my_id= *((int *)arg +1); // get thread id from argument list

     printf("Thread %d starting\n", my_id);

     if(opt == 0){
          push_mutex();push_mutex();pop_mutex();pop_mutex();push_mutex();
     } else {
          push_cas();push_cas();pop_cas();pop_cas();push_cas();
     }

     printf("Thread %d: exit\n", my_id);
     pthread_exit(0);
}

void print_remaining_nodes(const char *version) {
     Node *current = top;
     int node_count = 0;

     printf("%s: Remaining nodes in stack:\n", version);
     while (current != NULL)
     {
         printf("Node ID: %d\n", current->node_id);
         current = current->next;
         if (current != NULL) {
             printf(" -> ");
         }
         node_count++;
     }
     
     if (node_count == 0) {
         printf("Stack is empty\n");
     }
     printf("\nTotal remaining nodes: %d\n", node_count);
}

void free_stack() {
  Node *current = top;
     while (current != NULL) {
          Node *temp = current;
          current = current->next;
          free(temp);
     }
     top = NULL;
}

void reset_globals() {
     free_stack();
     node_id_counter = 0;
}

int main(int argc, char *argv[])
{
     if (argc !=2)
     {
          printf("Usage: %s <num_threads>\n", argv[0]);
          exit(-1);
     }

     num_threads = atoi(argv[1]);
     printf("Number of threads: %d\n", num_threads);


     /* Option 1: Mutex */ 
     printf("Starting Mutex version\n");
     reset_globals();


     pthread_t *workers= (pthread_t *) malloc (num_threads * sizeof(pthread_t));
     int *thread_args = malloc (num_threads * 2 * sizeof(int)); // each thread gets two integers: option and thread_id
     for (int i = 0; i < num_threads; i++) { 
          pthread_attr_t attr;
          thread_args[i*2] = 0;     // opt = 0 for mutex
          thread_args[i*2 + 1] = i; // thread ID
          
          pthread_attr_init(&attr);
          pthread_create(&workers[i], &attr, thread_func, (void *)&thread_args[i*2]); 
     }

     for (int i = 0; i < num_threads; i++) 
          pthread_join(workers[i], NULL);

     //Print out all remaining nodes in Stack
     printf("Mutex: Remaining nodes \n");
     print_remaining_nodes("Mutex");

     /*free up resources properly */
     free(workers);
     free(thread_args);

     /* Option 2: CAS */ 
     printf("Starting CAS version\n");
     reset_globals();
     workers= (pthread_t *) malloc (num_threads * sizeof(pthread_t));
     thread_args = malloc (num_threads * 2 * sizeof(int)); // each thread gets

     for (int i = 0; i < num_threads; i++) { 
          pthread_attr_t attr;
          thread_args[i*2]= 1; // option 1 for CAS
          thread_args[i*2+1]= i; // set thread id

          pthread_attr_init(&attr);
          pthread_create(&workers[i],&attr,thread_func,(void*)&thread_args[i*2]); // set thread id
     }
     for (int i = 0; i < num_threads; i++) 
          pthread_join(workers[i], NULL);

     //Print out all remaining nodes in Stack
     printf("CAS: Remaining nodes \n");
     print_remaining_nodes("CAS");

     
     free(workers);
     free(thread_args);
     free_stack();
     pthread_mutex_destroy(&counter_mutex);
     pthread_mutex_destroy(&stack_mutex);
     return 0;
}