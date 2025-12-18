#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

// Represents a single page in memory
typedef struct page {
     int page_id;
     int reference_bit;      // Set to 1 when accessed, cleared by checker
     int total_referenced;   // Statistics: counts how many times the checker found the reference_bit set
     struct page *next;
} Node;

// Represents a linked list of pages (either active or inactive)
typedef struct list {
     Node *head;
     Node *tail;
     int size;
} List;

List *active_list;      
List *inactive_list;    
int *reference_string;  
int N, M;               // N: total page numbers, M: checker sleep time in microseconds
pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for the (active_list, inactive_list, player_done flag)
volatile int player_done = 0; // Flag to signal the checker thread to terminate


// Helper Functions
// Creates and initializes a new, empty list.
List* create_list() {
     List *list = (List*)malloc(sizeof(List));
     list->head = NULL;
     list->tail = NULL;
     list->size = 0;
     return list;
}

// Creates and initializes a new page node.
Node* create_node(int page_id) {
     Node *node = (Node*)malloc(sizeof(Node));
     node->page_id = page_id;
     node->reference_bit = 0;
     node->total_referenced = 0;
     node->next = NULL;
     return node;
}

// Adds a page node to the tail (rear) of a list in O(1).
void add_to_rear(List *list, Node *node) {
     if (list->tail == NULL) { // List is empty
          list->head = node;
          list->tail = node;
     } else {
          list->tail->next = node;
          list->tail = node;
     }
     node->next = NULL;
     list->size++;
}

// Removes a page node from the head (front) of a list in O(1).
Node* remove_from_front(List *list) {
     if (list->head == NULL) return NULL;

     Node *node = list->head;
     list->head = list->head->next;
     if (list->head == NULL) { // List became empty
          list->tail = NULL;
     }
     node->next = NULL; // Detach the node from the list
     list->size--;
     return node;
}

// Finds a page by its ID within a list, removes it, and returns it.
Node* find_and_remove_page(List *list, int page_id) {
     if (list->head == NULL) return NULL;

     // Handle case where the node to remove is the head
     if (list->head->page_id == page_id) {
          return remove_from_front(list);
     }

     Node *prev = list->head;
     Node *curr = list->head->next;

     while (curr != NULL) {
          if (curr->page_id == page_id) {
               prev->next = curr->next;
               if (curr == list->tail) { // Update tail if the last element is removed
                    list->tail = prev;
               }
               curr->next = NULL; // Detach the node
               list->size--;
               return curr;
          }
          prev = curr;
          curr = curr->next;
     }
     return NULL; // Page not found
}

//Thread Functions
// Simulates a program accessing pages, driving the memory management logic.
void *player_thread_func() {
     for (int i = 0; i < 1000; i++) {
          int page_id = reference_string[i];

          pthread_mutex_lock(&list_mutex);

          // Find the referenced page, removing it from its current list.
          Node *page = find_and_remove_page(active_list, page_id);
          if (page == NULL) {
               page = find_and_remove_page(inactive_list, page_id);
          }

          // If the page doesn't exist in any list, create it.
          if (page == NULL) {
               page = create_node(page_id);
          }

          // Mark the page as referenced and move it to the rear of the active list.
          page->reference_bit = 1;
          add_to_rear(active_list, page);

          // If the active list exceeds 70% of N, move the oldest 20% to the inactive list.
          if (active_list->size > (int)(0.7 * N)) {
               int pages_to_move = (int)(0.2 * N);
               if (pages_to_move < 1) pages_to_move = 1; // Ensure at least one page is moved

               for (int j = 0; j < pages_to_move && active_list->size > 0; j++) {
                    Node *node_to_move = remove_from_front(active_list);
                    if (node_to_move != NULL) {
                         add_to_rear(inactive_list, node_to_move);
                    }
               }
          }

          pthread_mutex_unlock(&list_mutex);
          usleep(10); // Sleep for 10 microseconds to simulate work
     }

     // Signal the checker thread that the simulation is done.
     pthread_mutex_lock(&list_mutex);
     player_done = 1;
     pthread_mutex_unlock(&list_mutex);

     pthread_exit(0);
}

// Periodically scans the active list to update reference stats and clear reference bits.
void *checker_thread_func() {
     while (1) {
          usleep(M); // Sleep for the specified period.

          pthread_mutex_lock(&list_mutex);

          // Scan the active list.
          Node *curr = active_list->head;
          while (curr != NULL) {
               if (curr->reference_bit == 1) {
                    curr->total_referenced++; // Increment stat if page was used.
                    curr->reference_bit = 0;    // Reset the bit for the next cycle.
               }
               curr = curr->next;
          }

          // Safely check the termination flag.
          if (player_done) {
               pthread_mutex_unlock(&list_mutex);
               break; // Exit the loop if the player is finished.
          }
          pthread_mutex_unlock(&list_mutex);
     }
     pthread_exit(0);
}


int main(int argc, char *argv[])
{
     if (argc != 3) {
        fprintf(stderr, "Usage: %s <N: total_pages> <M: sleep_microseconds>\n", argv[0]);
        return 1;
     }

     N = atoi(argv[1]);
     M = atoi(argv[2]);

     // --- Setup ---
     srand(time(NULL)); // Seed the random number generator
     reference_string = (int*)malloc(1000 * sizeof(int));
     for (int i = 0; i < 1000; i++) {
          reference_string[i] = rand() % N; // Generate random page references
     }

     active_list = create_list();
     inactive_list = create_list();

     // --- Thread Creation & Execution ---
     pthread_t player;
     pthread_t checker;

     pthread_create(&player, NULL, player_thread_func, NULL);
     pthread_create(&checker, NULL, checker_thread_func, NULL);

     // Wait for both threads to complete
     pthread_join(player, NULL);
     pthread_join(checker, NULL);

     // --- Reporting ---
     printf("Page_Id, Total_Referenced\n");

     // Collect all unique pages from both lists to print comprehensive stats.
     Node *all_pages[N];
     for (int i = 0; i < N; i++) {
          all_pages[i] = NULL;
     }
     Node *curr = active_list->head;
     while (curr != NULL) {
          all_pages[curr->page_id] = curr;
          curr = curr->next;
     }
     curr = inactive_list->head;
     while (curr != NULL) {
         // This check is not strictly necessary as pages are unique, but it's safe.
          if (all_pages[curr->page_id] == NULL) {
               all_pages[curr->page_id] = curr;
          }
          curr = curr->next;
     }

     // Print stats for every page that was referenced.
     for (int i = 0; i < N; i++) {
          if (all_pages[i] != NULL) {
               printf("%d, %d\n", all_pages[i]->page_id, all_pages[i]->total_referenced);
          }
     }

     // Print the final state of the lists.
     printf("\nPages in active list: ");
     curr = active_list->head;
     while (curr != NULL) {
          printf("%d ", curr->page_id);
          curr = curr->next;
     }
     printf("\n");

     printf("Pages in inactive list: ");
     curr = inactive_list->head;
     while (curr != NULL) {
          printf("%d ", curr->page_id);
          curr = curr->next;
     }
     printf("\n");

     // --- Cleanup ---
     free(reference_string);

     // Free all nodes in the active list
     while (active_list->head != NULL) {
          Node *node = remove_from_front(active_list);
          free(node);
     }
     free(active_list);

     // Free all nodes in the inactive list
     while (inactive_list->head != NULL) {
          Node *node = remove_from_front(inactive_list);
          free(node);
     }
     free(inactive_list);

     pthread_mutex_destroy(&list_mutex);

     return 0;
}
