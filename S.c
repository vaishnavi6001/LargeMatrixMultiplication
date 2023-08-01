#include<signal.h>
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

int p1,p2;
char *args[]={"./p1","./p2",NULL};
int *flag1,*flag2;

int main(int argv, char* argc[]){

    struct timespec TATp1s,TATp1e;
    struct timespec TATp2s,TATp2e;
    struct timespec WTp1s,WTp1e;
    struct timespec WTp2s,WTp2e;
    struct timespec CSTs,CSTe;

    double WTp1 = 0;
    double WTp2 = 0;
    double CSTt = 0;

    char *file1 = argc[4], *file2 = argc[5], *file3 = argc[6];
    char *argsP1[] = {"./p1", argc[1], argc[2], argc[3], file1, file2, NULL};
    char *argsP2[] = {"./p2", file3, NULL};
    key_t k4 = 4, k5 = 5;
    int shm_id4, shm_id5;
    if((shm_id4 = shmget(k4, sizeof(int), 0666 | SHM_NORESERVE | IPC_CREAT)) == -1){
        perror("shmget");
    }
    if((shm_id5 = shmget(k5, sizeof(int), 0666 | SHM_NORESERVE | IPC_CREAT)) == -1){
        perror("shmget");
    }
    flag1 = shmat(shm_id4, 0, 0);
    flag2 = shmat(shm_id5, 0, 0);
    p1=fork();
    
    clock_gettime(CLOCK_REALTIME, &TATp1s);
    if(p1==0){
        execvp(args[0],argsP1);
    }
    else{
        p2=fork();
        clock_gettime(CLOCK_REALTIME, &TATp2s);
        if(p2==0){
            execv(args[1],argsP2);
        }
        else{
            int i=0;
            int exec_count = 0;
            while(1){
                if(flag1[0]==1||flag2[0]==1){
                    clock_gettime(CLOCK_REALTIME, &TATp1e);
                    break;
                }
                kill(p2,SIGSTOP);
                clock_gettime(CLOCK_REALTIME, &WTp1s);
                usleep(2000);
                clock_gettime(CLOCK_REALTIME, &WTp1e);
                clock_gettime(CLOCK_REALTIME, &CSTs);
                exec_count++;
                if(flag1[0]==1) {
                    clock_gettime(CLOCK_REALTIME, &TATp1e);
                    clock_gettime(CLOCK_REALTIME, &CSTe);
                    kill(p2,SIGCONT);
                    CSTt = CSTt + (CSTe.tv_sec - CSTs.tv_sec) + (CSTe.tv_nsec - CSTs.tv_nsec) / 1000000000.0;
                    break;
                }
                kill(p2,SIGCONT);
                kill(p1,SIGSTOP);
                clock_gettime(CLOCK_REALTIME, &CSTe);
                CSTt = CSTt + (CSTe.tv_sec - CSTs.tv_sec) + (CSTe.tv_nsec - CSTs.tv_nsec) / 1000000000.0;
                clock_gettime(CLOCK_REALTIME, &WTp2s);
                usleep(2000);
                clock_gettime(CLOCK_REALTIME, &WTp2e);
                kill(p1,SIGCONT);
                WTp1 = WTp1 + (WTp1e.tv_sec - WTp1s.tv_sec) + (WTp1e.tv_nsec - WTp1s.tv_nsec) / 1000000000.0;
                WTp2 = WTp2 + (WTp2e.tv_sec - WTp2s.tv_sec) + (WTp2e.tv_nsec - WTp2s.tv_nsec) / 1000000000.0;
            }
            while(flag2[0]!=1){

            }
            if(flag2[0] == 1) {
                clock_gettime(CLOCK_REALTIME, &TATp2e);
            }
            if(exec_count==0) exec_count=1;
            double TATp1 = (TATp1e.tv_sec - TATp1s.tv_sec) + (TATp1e.tv_nsec - TATp1s.tv_nsec) / 1000000000.0;
            double TATp2 = (TATp2e.tv_sec - TATp2s.tv_sec) + (TATp2e.tv_nsec - TATp2s.tv_nsec) / 1000000000.0;
            //printf("%d\n", exec_count);
            double CST = 2*CSTt/(double)exec_count;
            printf("p1 TAT: %0.9lf | p2 TAT: %0.9lf \n",TATp1,TATp2);
            printf("p1  WT: %0.9lf | p2  WT: %0.9lf \n",WTp1,WTp2);
            printf("average CST: %0.9lf \n",CST);
        }
   }
   shmctl(shm_id4, IPC_RMID, NULL);
   shmctl(shm_id5, IPC_RMID, NULL);
   return 0;
}