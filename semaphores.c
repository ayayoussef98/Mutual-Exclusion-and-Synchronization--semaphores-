#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<pthread.h>
#include<semaphore.h>

typedef struct{
int data;
struct Node*next;
}Node;

typedef struct{
 Node *front, *rear;

}Queue;

//DEFINING NEW NODE//
 Node*newNode(int value)
 {
     Node *n= (Node*) malloc(sizeof(Node));
     n->data=value;
     n->next=NULL;
     return n;
 }
 //INITIALIZING A NEW QUEUE//
void initializeQueue(Queue *q)
{
    q->front=q->rear=NULL;
}

  //PUTTING A NUMBER IN THE QUEUE//
void enqueue(Queue *q,int value)
{
    Node *ptr=newNode(value);
     if(q->front==NULL)
        q->front=q->rear=ptr;
     else
     {
         q->rear->next=ptr;
         q->rear=ptr;
     }
}

  //REMOVING A NUMBER FROM THE QUEUE//
int dequeue(Queue *q)
{
     int x;
     if(q->front==NULL)
        exit(1);
     else
     {
         x=q->front->data;
         q->front=q->front->next;
         return x;
     }
}


Queue *buffer;
int counter;
int noofthreads;
int currentnoofmsgs=0;
int bufferSize;
int prodPosition=0;
int conPosition=0;
int bufferFull=0;

sem_t isFull; //if buffer is full then producer shouldnt add to it
sem_t isEmpty; //if buffer is empty then consumer shouldnt take from it
sem_t mcounter; //if producer is checking the counter then counter value must not change
sem_t ProCon; //consumer and producer cant access buffer in one time


void *mcounterr(void *arg)
{
 while(1){
int i=(int)arg;

printf("\nCounter thread %d: waiting to write\n",i); //waiting to write after sleep duration ends
sleep((rand()%5)+1);

sem_wait(&mcounter);
printf("\nCounter thread %d: received a message\n",i);
counter++;
printf("\nCounter thread %d: now adding to counter, counter value=%d\n",i,counter);
sem_post(&mcounter);
 }
}


void *mmonitor(void *arg)
{
 while(1){

if(prodPosition>=bufferSize)
prodPosition=0;

if(bufferFull==0)
printf("\nMonitor thread: Buffer full!\n");

 printf("\nMonitor thread: waiting to read counter\n"); //waiting to read after sleep duration ends
 sleep((rand()%5)+1);

 sem_wait(&ProCon);
 sem_wait(&mcounter);
 sem_wait(&isFull);


 printf("\nMonitor thread: reading a count value of %d\n",counter);
 enqueue(buffer,counter);
 prodPosition++;
 printf("\nMonitor thread: writing to buffer at position %d\n",prodPosition);
 //currentnoofmsgs=currentnoofmsgs+counter;
 //printf("%d\n",currentnoofmsgs);
 counter=0;

 sem_post(&ProCon);
 sem_post(&mcounter);
 sem_post(&isEmpty); 
 bufferFull--; 
 }
}

void *mcollector(void *arg)
{ int x;
 while(1){

 if(buffer->front==NULL)
printf("\nCollector thread: nothing is in the buffer!\n");

 sleep((rand()%5)+1);

 sem_wait(&ProCon);
 sem_wait(&isEmpty);

 x=dequeue(buffer);
 conPosition++;
printf("\nCollector thread: reading from buffer at position %d\n",conPosition);
 
 sem_post(&ProCon);
 sem_post(&isFull);
 bufferFull++;
 }
}


int main()
{
int x,i;
printf("Number of threads: ");
scanf("%d",&noofthreads);
printf("\nBuffer size: ");
scanf("%d",&bufferSize);
printf("\n\n");

bufferFull=bufferSize;
pthread_t *threadM;
threadM=(pthread_t *)malloc(noofthreads*sizeof(pthread_t));
pthread_t threadP;
pthread_t threadC;

sem_init(&isFull,0,bufferSize);
sem_init(&isEmpty,0,0);
sem_init(&mcounter,0,1);
sem_init(&ProCon,0,1);

buffer=malloc(sizeof(Queue));
initializeQueue(buffer);

for(i=0;i<noofthreads;i++){
pthread_create(&threadM[i],NULL,mcounterr,(void*)i);
//printf("thread %d\n",i);
}
pthread_create(&threadP,NULL,mmonitor,NULL);
pthread_create(&threadC,NULL,mcollector,NULL);


for(i=0;i<noofthreads;i++)
pthread_join(threadM[i],NULL);

pthread_join(threadP,NULL);
pthread_join(threadC,NULL);

sem_destroy(&isFull);
sem_destroy(&isEmpty);
sem_destroy(&mcounter);
sem_destroy(&ProCon);

    return 0;
}


