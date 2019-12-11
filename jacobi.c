/**
* Name: Sean Kearney
* Project: Jacobi iteration for CSCI347
*/

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <math.h>
#include <semaphore.h>
#include <time.h>


struct thread_st {
    int    NOTH;
    int    i;
    int    startRow;
    int    endRow;
};
sem_t lock;
sem_t* barrier;
int thdsWait = 0;
int bcnt = 0;

void* jacobi(void* arg);
double (*parse())[1024];
bool isConverged();
void printArr(double (*arr)[1024]);
int findNOTH();
void createThreads(int NOTH);
struct thread_st* mk_thread_st(int index, int NOTH);
void* thread_start(void *arg);
void barrier_func(int thdNum, int NOTH);

double (*old)[1024];
double (*new)[1024];
pthread_barrier_t mybarrier;
bool converged = false;

int main(int argc, char **argv){

    old = (double (*)[1024]) parse();
    new = (double (*)[1024]) parse();
    NOTH = findNOTH(); uncomment this to have scanf input for NOTH
    printf("NOTH: %d",NOTH);
    sem_t bmaker[NOTH];

    pthread_barrier_init(&mybarrier, NULL, NOTH);

    barrier = bmaker;
    for(int i = 0; i < NOTH; i++){
        sem_init(&barrier[i],0,0);
    }

    createThreads(NOTH);

    free(old);
    free(new);
    
    exit(0);
}

void* jacobi(void *arg){
    struct thread_st *p = arg;

    bool convergedCheck = false;

    //move, barrier, estimate, barrier, convergence, barrier, repeat
    while(!converged){

        for(int i = p->startRow; i < p->endRow; ++i){
            for(int j = 1; j < 1023; ++j){
                old[i][j] = new[i][j];
            }
        }

        pthread_barrier_wait(&mybarrier);

        //calculate approximation on new array
        for(int i = p->startRow; i < p->endRow; ++i){
            for(int j = 1; j < 1023; ++j){
                new[i][j] = (old[i+1][j] + old[i-1][j] + old[i][j+1] + old[i][j-1])/4;
                //old[i][j] = new[i][j];
            }
        }

        pthread_barrier_wait(&mybarrier);

        if(p->i == 0 && isConverged(old,new)){
            converged = true;
        }


        pthread_barrier_wait(&mybarrier);

    }
    return NULL;
}

bool isConverged(){

    double delta = 0;

    for(int i = 0; i < 1024; ++i){
        for(int j = 0; j < 1024; ++j){
            delta = fmax(delta, fabs(new[i][j]-old[i][j]));
        }
    }

    double epsilon = .00001;
    return (delta < epsilon);
}

double (*parse())[1024]{
    double (*arr)[1024] = malloc(sizeof(double)*1024*1024);
    FILE *fp;
    fp = fopen("input.mtx", "r");

    if(fp == NULL){
        perror("fopen err");
        exit(1);
    }

    double *val = malloc(sizeof(double));
    for(int i = 0; i < 1024; i++){
        for(int j = 0; j < 1024; j++){
            if(fscanf(fp,"%lf",val) == -1){
                perror("fscanf err");
            }
            arr[i][j] = *val;
        }
    }

    fclose(fp);

    return arr;
}

void createThreads(int NOTH){

    pthread_t thds[NOTH];
    sem_init(&lock,0,1);

    for(int i = 0; i < NOTH; i++){
        pthread_create (&thds[i], NULL,
                    jacobi,
                    mk_thread_st(i, NOTH));
    }

    for(int i = 0; i < NOTH; i++){
        void* p;
        pthread_join(thds[i], &p);
        free(p);
    }
    sem_destroy(&lock);
}

struct thread_st* mk_thread_st(int i, int NOTH){
    int th_rowStart = i*(1024/NOTH);
    int th_rowEnd = ((i+1)*(1024/NOTH))-1;

    if(i == 0){
        th_rowStart=1;
    }
    if(i+1 == NOTH){
        th_rowEnd = 1023;
    }

    struct thread_st* p = malloc(sizeof (struct thread_st));
    if(p != NULL) {
        p->i    = i;
        p->NOTH  = NOTH;
        p->startRow = th_rowStart;
        p->endRow = th_rowEnd;
    }
    else {
        perror("malloc");
        exit(1);
    }

    return p;
}

void* thread_start(void *arg){
    struct thread_st *p = arg;

    jacobi(p);

    return p;
}

void barrier_func(int thdNum, int NOTH){
    sem_wait(&lock);
    thdsWait++;
    sem_post(&lock);
    if(thdsWait < NOTH){
        sem_wait(&barrier[thdNum]);
    }
    else{
    bcnt++;
    thdsWait = 0;
    for(int j = 0; j < NOTH; j++){
        sem_post(&barrier[j]);
    }
    sem_wait(&barrier[thdNum]);
    }
}

void printArr(double (*arr)[1024]){
    for(int i = 0; i < 1024; i++){
        for(int j = 0; j < 1024; j++){
            printf("i: %d, j: %d ",i,j);
            printf("%.10lf\n",arr[i][j]);
        }
    }
}

int findNOTH(){
    printf("Number of threads?\n");
    int *NOTHptr = malloc(sizeof(int));
    scanf("%d",NOTHptr);
    printf("Running with %d threads\n",*NOTHptr);
    return *NOTHptr;
}
