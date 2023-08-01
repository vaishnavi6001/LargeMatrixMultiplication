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

int *sh_mem1, *sh_mem2, *sh_mem3, *sh_mem4, m, n, p;

typedef struct {
    int s1, s2, e1, e2;
    FILE *fp1, *fp2;
}thread_args;


typedef struct {
    FILE *fp;
    int num_ints, last;
}thread_args2;

void *mem1, *mem2;

int s1 = 0, s2 = 0, m1 = 0, m2 = 0;
pthread_mutex_t lock1, lock2;

void *read_file1(void *args){
    pthread_mutex_lock(&lock1);
    thread_args2 *params = (thread_args2*) args;
    FILE* fp = params->fp;
    fseek(fp, s1, SEEK_SET);
    int c = 0;
    if(params->last==0){
        int num = params->num_ints;
        while(c < num){
            // int aa = 0 ;
            // fscanf(fp, "%d", &aa);
            fscanf(fp, "%d ", &sh_mem1[m1]);
            sh_mem3[3] = m1;
            // printf("%d: %d\n", sh_mem1[m1], c);
            m1++; 
            c++;
        }
    }
    else{
        int num = m*n-m1;
        while(c < num){
            // int aa = 0 ;
            // fscanf(fp, "%d", &aa);
            fscanf(fp, "%d ", &sh_mem1[m1]);
            sh_mem3[3] = m1;
            // printf("%d: %d\n", sh_mem1[m1], c);
            m1++; 
            c++;
        }
    }
    s1 = ftell(fp);
    pthread_mutex_unlock(&lock1);
    return NULL;
}

void *read_file2(void *args){
    pthread_mutex_lock(&lock2);
    thread_args2 *params = (thread_args2*) args;
    FILE* fp = params->fp;
    //Offest to s1
    fseek(fp, s2, SEEK_SET);
    int c = 0;
    if(params->last==0){
        int num = params->num_ints;
        while(c < num){
            // int aa = 0 ;
            // fscanf(fp, "%d", &aa);
            fscanf(fp, "%d", &sh_mem2[m2]);
            sh_mem3[4] = m2;
            // printf("%d: %d\n", sh_mem2[m2], c);
            m2++; 
            c++;
        }
    }
    else{
        int num = n*p-m2;
        while(c < num){
            // int aa = 0 ;
            // fscanf(fp, "%d", &aa);
            fscanf(fp, "%d", &sh_mem2[m2]);
            sh_mem3[4] = m2;
            // printf("%d: %d\n", sh_mem2[m2], c);
            m2++; 
            c++;
        }
    }
    s2 = ftell(fp);
    pthread_mutex_unlock(&lock2);
    return NULL;
}

int index1 = 0, index2 = 0;

int main(int argc, char* argv[]) {
    struct timespec start, end;
    m = atoi(argv[1]);
    n = atoi(argv[2]);
    p = atoi(argv[3]);
    char *file1 = argv[4], *file2 = argv[5];

    FILE* fp1 = fopen(file1, "rb");    
    FILE *fp2 = fopen(file2, "rb");
    FILE *fp3 = fopen("time1.csv", "w+");

    key_t k1 = 1, k2 = 2, k3 = 3, k4 = 4;
    int shm_id1, shm_id2, shm_id3, shm_id4;
    if((shm_id1 = shmget(k1, sizeof(int)*m*n, 0666 | SHM_NORESERVE | IPC_CREAT)) == -1){
        perror("shmget");
    }
    if((shm_id2 = shmget(k2, sizeof(int)*n*p, 0666 | SHM_NORESERVE | IPC_CREAT)) == -1){
        perror("shmget");
    }
    if((shm_id3 = shmget(k3, sizeof(int)*5, 0666 | SHM_NORESERVE | IPC_CREAT)) == -1){
        perror("shmget");
    }
    if((shm_id4 = shmget(k4, sizeof(int), 0666 | SHM_NORESERVE | IPC_CREAT)) == -1){
        perror("shmget");
    }
    if((sh_mem1 = shmat(shm_id1, 0, 0)) == (int *)-1){
        perror("shmat");
    }
    if((sh_mem2 = shmat(shm_id2, 0, 0)) == (int *)-1){
        perror("shmat");
    }
    if((sh_mem3 = shmat(shm_id3, 0, 0)) == (int *)-1){
        perror("shmat");
    }
    if((sh_mem4 = shmat(shm_id4, 0, 0)) == (int *)-1){
        perror("shmat");
    }
    sh_mem4[0] = 0;
    sh_mem3[0] = m;
    sh_mem3[1] = n;
    sh_mem3[2] = p;
    //for(int k=1; k<=1000; k+=2){
        //clock_t t1 = clock();
        clock_gettime(CLOCK_REALTIME, &start);
        int N = 1;     //NUMBER OF THREADS
        s1 = 0;
        s2 = 0;
        m1 = 0; 
        m2 = 0;
        int n_int_each_thread1 = (m*n)/N, n_int_each_thread2 = (n*p)/N;
        if(n_int_each_thread1 < 1) n_int_each_thread1 = 1;
        if(n_int_each_thread2 < 1) n_int_each_thread2 = 1;
        pthread_t threads1[N];
        pthread_t threads2[N];
        thread_args2 *my_args1;
        thread_args2 *my_args2;
        my_args1 = (thread_args2 *)malloc(sizeof(thread_args2)*(N));
        my_args2 = (thread_args2 *)malloc(sizeof(thread_args2)*(N));

        for (int i = 0; i < N; i++) {
            my_args1[i].fp = fp1;
            my_args1[i].num_ints = n_int_each_thread1;
            my_args2[i].fp = fp2;
            my_args2[i].num_ints = n_int_each_thread2;
            my_args1[i].last = 0;
            my_args2[i].last = 0;
            if(i == N-1){
                my_args1[i].last = 1;
                my_args2[i].last = 1;
            }
            if(pthread_create(&threads1[i], NULL, read_file1, &my_args1[i])){
                perror("thread error\n");
            }
            if(pthread_create(&threads2[i], NULL, read_file2, &my_args2[i])){
                perror("thread error\n");
            }
        }

        for (int j = 0; j < N; j++){
            pthread_join(threads1[j], NULL);
            pthread_join(threads2[j], NULL);
        }

        //clock_t t2 = clock() - t1;
        //double time_taken = ((double)t2) / CLOCKS_PER_SEC; // in seconds
        clock_gettime(CLOCK_REALTIME, &end);
        double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
        // printf("program took %0.9lf seconds to execute \n", time_taken);
        fprintf(fp3, "%d, %0.9lf\n", N, time_taken);
    //}
    fclose(fp1);
    fclose(fp2);
    sh_mem4[0] = 1;
    return 0;
}