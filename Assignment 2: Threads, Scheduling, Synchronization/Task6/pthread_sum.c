#include <pthread.h> 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>


#define ARRAY_SIZE 1000000
int num_threads = 0;
float *array; // array to be summed
float *partial_sums; 

typedef struct{
     int thread_id;
     int start_index;
     int end_index;
} thread_data_t;

     
void *thread_func(void *arg); /* the thread function */

int main(int argc, char *argv[])
{
     if (argc !=2)
     {
          printf("Usage: %s <num_threads>\n", argv[0]);
          exit(-1);
     }
     
     
     num_threads = atoi(argv[1]);


     /* Initialize an array of random values */ 
     array= (float *) malloc (ARRAY_SIZE * sizeof(float));
     if(array==NULL){
          printf("Error allocating memory for the array\n");
          exit(-1);
     }

     srand(time(NULL));//seed the random number generator
     for(int i=0;i<ARRAY_SIZE;i++){
          array[i]= (float)rand()/(float)RAND_MAX; //random float in [0,1]
     }

     /* Perform Serial Sum */
     float  sum_serial = 0.0;
     struct timeval start_serial, end_serial;

     //Timer Begin
     gettimeofday(&start_serial, NULL);
     for(int i=0;i<ARRAY_SIZE;i++){
          sum_serial += array[i];
     }


     //Timer End
     gettimeofday(&end_serial, NULL);
     double time_serial = (end_serial.tv_sec - start_serial.tv_sec) +
                          (end_serial.tv_usec - start_serial.tv_usec)/1000000.0;
    printf("Serial Sum = %f, time = %.6f \n", sum_serial, time_serial);

     
     /* Create a pool of num_threads workers and keep them in workers */ 
     pthread_t *workers= (pthread_t *) malloc (num_threads * sizeof(pthread_t));
     thread_data_t *thread_data= (thread_data_t *) malloc (num_threads * sizeof(thread_data_t));
     partial_sums= (float *) malloc (num_threads * sizeof(float));
     
     if(workers==NULL || thread_data==NULL || partial_sums==NULL){
          printf("Error allocating memory for threads\n");
          exit(-1);
     }

     //Calculate chunk size for each thread
     int chunk_size = ARRAY_SIZE / num_threads;
     int remainder = ARRAY_SIZE % num_threads;

     struct timeval start_parallel, end_parallel;
     double time_parallel = 0.0;
     double sum_parallel = 0.0;

     //Timer Begin
     gettimeofday(&start_parallel, NULL);

     // Create threads with their assigned work ranges 
     for (int i = 0; i < num_threads; i++) 
     { 
        thread_data[i].thread_id = i;
        thread_data[i].start_index = i * chunk_size;
        thread_data[i].end_index = (i + 1) * chunk_size;
        
        // Distribute remainder among first few threads
        if (i < remainder) {
            thread_data[i].end_index++;
        }
        
        // Adjust subsequent start indices
        if (i > 0) {
            thread_data[i].start_index = thread_data[i-1].end_index;
        }
        

          pthread_attr_t attr;
          pthread_attr_init(&attr);
          int rc = pthread_create(&workers[i], NULL, thread_func, (void *)&thread_data[i]);
          
          if (rc)
          {
               printf("Error:unable to create thread, %d\n", rc);
               exit(-1);
          }
          
     }

     // Wait for all threads to complete
     for (int i = 0; i < num_threads; i++){ 
          pthread_join(workers[i], NULL);
     }

     //Combine partial sums from all threads
     for(int i=0;i<num_threads;i++){
          sum_parallel += partial_sums[i];
     }


     //Timer End
     gettimeofday(&end_parallel, NULL);
     time_parallel = (end_parallel.tv_sec - start_parallel.tv_sec) + 
                   (end_parallel.tv_usec - start_parallel.tv_usec) / 1000000.0;
     printf("Parallel Sum = %f, time = %.6f \n", sum_parallel, time_parallel);
     
     free(array);
     free(workers);
     free(thread_data);
     free(partial_sums);
     pthread_exit(NULL);
     
}

void *thread_func(void *arg) { 
     /* Assign each thread an id so that they are unique in range [0, num_thread -1 ] */
     thread_data_t *data = (thread_data_t *)arg;
     int my_id= data->thread_id;


     /* Perform Partial Parallel Sum Here */
    float my_sum = 0.0;
    for (int i = data->start_index; i < data->end_index; i++) {
        my_sum += array[i];
    }
    
    partial_sums[my_id] = my_sum;
    
    printf("Thread %d sum = %f (indices %d to %d)\n", my_id, my_sum, data->start_index, data->end_index - 1);
    pthread_exit(NULL);
}