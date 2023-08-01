#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

int m, n, p;
long long *output;
int *sh_mem1, *sh_mem2, *sh_mem3, *sh_mem5;

typedef struct{
    int start, end;
    int num_of_int;
}ind;

typedef struct{
    int a, b;
}row_col;

typedef struct{
    int old_r1, old_r2, new_r1, new_r2;
}old_new;

void *helper1(void *arg)
{
    ind *data = (ind *)arg;
    int num = data->num_of_int;
    int s = data->start, e = data->end;
    long l;
    for(int k=0; k<num; k++){
        int s1 = s + k;
        int a = s1/p, b = s1%p;
        output[a*p+b] = 0;
        for(int i=0; i<n; i++){
            output[a*p+b] += sh_mem1[a*n+i]*sh_mem2[b*n+i];
        }
    }
    return NULL;
}

void *multiplication(void *arg)
{
    row_col *data = (row_col *)arg;
    int a = data->a, b = data->b;
    output[a*p+b] = 0;
    for(int i=0; i<n; i++){
        output[a*p+b] += sh_mem1[a*n+i]*sh_mem2[b*n+i];
    }
    // printf("%lld ", output[a*p+b]);
    return NULL;
}

void *helper(int old_r1, int old_r2, int new_r1, int new_r2){
    // old_new *data = (old_new *)arg;
    // int old_r1 = data->old_r1, old_r2 = data->old_r2, new_r1 = data->new_r1, new_r2 = data->new_r2;
    int thread_count = 0;
    for(int i=0; i<new_r1; i++){
        if(i >= old_r1){
            pthread_t threads[old_r2];
            row_col arg[old_r2];
            for(int j=0; j<old_r2; j++){
                arg[j].a = i;
                arg[j].b = j;
                pthread_create(&threads[j], NULL, multiplication, &arg[j]);
                // multiplication(&arg);
            }
            for(int j=0; j<old_r2; j++){
                pthread_join(threads[j], NULL);
            }

        }
        pthread_t threads1[new_r2-old_r2];
        row_col arg2[new_r2-old_r2];
        for(int j=old_r2; j<new_r2; j++){
            arg2[j-old_r2].a = i;
            arg2[j-old_r2].b = j;
            // printf("%d %d\n", arg2.a, arg2.b);
            pthread_create(&threads1[j-old_r2], NULL, multiplication, &arg2[j-old_r2]);
            // break;
            // multiplication(&arg2);
        }
        for(int j=old_r2; j<new_r2; j++){
            pthread_join(threads1[j-old_r2], NULL);
            // break;
        }
    }
}

int main(int argc, char *argv[])
{
    struct timespec start, end;
    // int N = atoi(argv[1]);
    key_t k1 = 1, k2 = 2, k3 = 3, k5 = 5;
    int shm_id3;
    if((shm_id3 = shmget(k3, sizeof(int) * 5, 0666 | SHM_NORESERVE | IPC_CREAT)));
    sh_mem3 = shmat(shm_id3, 0, 0);
    m = sh_mem3[0];
    n = sh_mem3[1];
    p = sh_mem3[2];
    int shm_id1 = shmget(k1, sizeof(int) * m * n, 0666 | SHM_NORESERVE | IPC_CREAT);
    int shm_id2 = shmget(k2, sizeof(int) * n * p, 0666 | SHM_NORESERVE | IPC_CREAT);
    int shm_id5 = shmget(k5, sizeof(int), 0666 | SHM_NORESERVE | IPC_CREAT);
    sh_mem1 = shmat(shm_id1, 0, 0);
    sh_mem2 = shmat(shm_id2, 0, 0);
    sh_mem5 = shmat(shm_id5, 0, 0);
    sh_mem5[0] = 0;
    char *file = argv[1];
    FILE *fp = fopen("time2.csv", "w+");
    FILE *fpo = fopen(file, "w+");
    // printf("*%d %d %d*", m, n, p);
    // printf("%d", sh_mem3[0]);
    int old_1x = 0, old_2x = 0;
    // int filled_1x = m1 / n, filled_1y = m1 % n, filled_2x = m2 / p, filled_2y = m2 % p;
    output = (long long *)malloc(sizeof(long long)*m*p);

    while(1){
        int m1 = sh_mem3[3], m2 = sh_mem3[4];
        int filled_1x = m1 / n, filled_2x = m2 / n;  //rows till filled_1x and filled_2x are done
        // printf("%d %d\n", m1, m2);
        if((m1 == m*n-1) && (m2 == n*p-1)){          //we have received the last element in both arrays
            filled_1x += 1;
            filled_2x += 1;
        }
        if((filled_1x > old_1x || filled_2x > old_2x) && filled_1x > 0 && filled_2x > 0){

        // for(int k=1; k<1000; k+=2){
        //     int N = k;        //NUMBER OF THREADS
        //     clock_gettime(CLOCK_REALTIME, &start);
        //     pthread_t threads[N];
        //     ind my_args[N];
        //     int n_int_each_thread = (m*p)/N;
        //     if(n_int_each_thread < 1) n_int_each_thread = 1;
        //     int c = 0;
        //     int t = 0;
        //     for(int i=0; i<N; i++){
        //         my_args[i].start = i*n_int_each_thread;
        //         my_args[i].num_of_int = n_int_each_thread;
        //         if(i==N-1){
        //             my_args[i].num_of_int = m*p-c;
        //         }
        //         c += my_args[i].num_of_int;
        //         if(c > m*p){
        //             N = i+1;
        //             break;
        //         }
        //         pthread_create(&threads[i], NULL, helper1, &my_args[i]);
        //     }

        //     for(int i=0; i<N; i++){
        //         pthread_join(threads[i], NULL);
        //     }

        //     // clock_t t2 = clock() - t1;
        //     // double time_taken = ((double)t2) / CLOCKS_PER_SEC; // in seconds
        //     // printf("program took %f seconds to execute \n", time_taken);
        //     clock_gettime(CLOCK_REALTIME, &end);
        //     double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
        //     printf("program took %0.9lf seconds to execute \n", time_taken);
        //     fprintf(fp, "%d, %0.9lf\n", N, time_taken);
        // }

            helper(old_1x, old_2x, filled_1x, filled_2x);
            old_1x = filled_1x;
            old_2x = filled_2x;
        }
        if((m1 == m*n-1) && (m2 == n*p-1)) 
            break;
    }
    fclose(fp);
    // for (int i = 0; i < m; i++){
    //     for (int j = 0; j < n; j++)
    //         printf("%d ", sh_mem1[i * n + j]);
    //     printf("\n");
    // }
    // printf("-----------------------------\n");
    // for (int i = 0; i < p; i++){
    //     for (int j = 0; j < n; j++)
    //         printf("%d ", sh_mem2[i * n + j]);
    //     printf("\n");
    // }

    for (int i = 0; i < m; i++){
        for (int j = 0; j < p; j++){
            fprintf(fpo, "%lld ", output[i * p + j]);
            //printf("%lld ", output[i * p + j]);
        }
        fprintf(fpo, "\n");
        //printf("\n");
    }
    fclose(fpo);
    shmctl(shm_id1, IPC_RMID, NULL);
    shmctl(shm_id2, IPC_RMID, NULL);
    shmctl(shm_id3, IPC_RMID, NULL);
    sh_mem5[0] = 1;
    return 0;
}