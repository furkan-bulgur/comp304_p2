#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

/**
 * pthread_sleep takes an integer number of seconds to pause the current thread
 * original by Yingwu Zhu
 * updated by Muhammed Nufail Farooqi
 * updated by Fahrican Kosar
 */
int pthread_sleep(double seconds){
    pthread_mutex_t mutex;
    pthread_cond_t conditionvar;
    if(pthread_mutex_init(&mutex,NULL)){
        return -1;
    }
    if(pthread_cond_init(&conditionvar,NULL)){
        return -1;
    }

    struct timeval tp;
    struct timespec timetoexpire;
    // When to expire is an absolute time, so get the current time and add
    // it to our delay time
    gettimeofday(&tp, NULL);
    long new_nsec = tp.tv_usec * 1000 + (seconds - (long)seconds) * 1e9;
    timetoexpire.tv_sec = tp.tv_sec + (long)seconds + (new_nsec / (long)1e9);
    timetoexpire.tv_nsec = new_nsec % (long)1e9;

    pthread_mutex_lock(&mutex);
    int res = pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&conditionvar);

    //Upon successful completion, a value of zero shall be returned
    return res;
}

int commentator_thread_number = 1;
int question_number = 5;
float requesting_answer_probability = 0.75;
float max_talking_time = 3;
float breaking_event_probability = 0.05;

int victim = 3;

sem_t mutex;

void *comentator_thread(void *vargp){
  sem_wait(&mutex);
  printf("Entered Commentator\n");

  victim++;

  printf("Exiting Commentator %d\n",victim);
  sem_post(&mutex);
}

void *moderator_thread(void *vargp){
  sem_wait(&mutex);
  printf("Entered Moderator\n");

  victim--;

  printf("Exiting Moderator %d\n",victim);
  sem_post(&mutex);
}

int main(int argc, char *argv[]){

  sem_init(&mutex,0,1);
  pthread_t t1,t2;

  pthread_create(&t1, NULL, moderator_thread, NULL);
  pthread_create(&t2, NULL, comentator_thread, NULL);

  pthread_join(t1,NULL);
  pthread_join(t2,NULL);
  sem_destroy(&mutex);



  return 0;

}
