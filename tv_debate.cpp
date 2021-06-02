#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <queue>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <fstream>
#include <semaphore.h>
#include <math.h>

using namespace std;

#define MAX_NUMBER_COMMENTATORS 25
struct Request {
  int id;
  float speak_time;
  int commantator_num; //which commantator's answer is this
};

queue<Request> request_queue;

//command line arguments -p, -n, -q, t
float prob_to_answer;
float prob_to_breaking_news;
int num_commentators;
int num_questions;
time_t max_speak_time;
int req_id = 0;
time_t start_time;
clock_t start;


//barrier
pthread_barrier_t question_barrier;
//threads
pthread_t commentators[MAX_NUMBER_COMMENTATORS];
pthread_cond_t comment_conds[MAX_NUMBER_COMMENTATORS];
pthread_t moderator;

pthread_cond_t question_asked;
pthread_mutex_t access_global_queue_mutex;
pthread_mutex_t question_mutex;

pthread_attr_t thread_attribute;



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

void print_time(){
  clock_t end = clock();
  double elapsed = (double)(end-start)*CLOCKS_PER_SEC;
  printf("%lf\n",elapsed);
  double milisec = (int)elapsed%1000;
  double temp_second = floor(elapsed/1000.0);
  double second = (int)temp_second%60;
  double minute = floor(temp_second/60.0);
  printf("[%.0lf:%.0lf:%.0lf] ",minute,second,milisec);
}


// Posts new request to the queue.
int post_new_request_to_queue(int com_num, float speak_time) {
  struct Request new_req;
  new_req.id = req_id;
  new_req.speak_time = speak_time;
  new_req.commantator_num = com_num;
  //queue<Commentator> *req_queue = &request_queue;
  request_queue.push(new_req);
  req_id = req_id + 1;
  return new_req.id;
}

void *request(void *com_num) {
    int commentator_num = (long)com_num;
    // Random speak time between 1 and max_speak_time
    float speak_time = (float)rand()/(float)(RAND_MAX/(max_speak_time - 1)) + 1;
    int counter = num_questions;
    while(counter>0){
      //printf("Ben tıkandım question condda %d\n",commentator_num);
      pthread_cond_wait(&question_asked,&access_global_queue_mutex);
      if((rand()/(float)RAND_MAX) < prob_to_answer) {
        int position_in_queue = post_new_request_to_queue(commentator_num,speak_time);
        //print_time();
        printf("Commentator #%d generates answer, position in queue: %d\n",commentator_num,position_in_queue);
        pthread_mutex_unlock(&access_global_queue_mutex);
        pthread_barrier_wait(&question_barrier);
        pthread_mutex_lock(&access_global_queue_mutex);
        pthread_cond_wait(&comment_conds[commentator_num],&access_global_queue_mutex);
        pthread_sleep(speak_time);
        //print_time();
        printf("Commentator #%d finishes speaking.\n",commentator_num);
        pthread_mutex_unlock(&access_global_queue_mutex);
      }else{
        pthread_barrier_wait(&question_barrier);
        pthread_mutex_unlock(&access_global_queue_mutex);
      }
      counter--;
    }

    pthread_exit(0);
}

void *moderate(void *vargp) {

  for(int i=0; i<num_questions; i++){
    //print_time();
    pthread_sleep(1);
    printf("Moderator asks question %d\n", i+1);
    pthread_cond_broadcast(&question_asked);
    //printf("Ben tıkandım barierde\n");
    pthread_barrier_wait(&question_barrier);
    //printf("Ben tıkandım global mutexte\n");
    pthread_mutex_lock(&access_global_queue_mutex);
    while(request_queue.size()!=0) {
      struct Request first_req = request_queue.front();
      int com_num = first_req.commantator_num;
      float time = first_req.speak_time;
      request_queue.pop();
      //print_time();
      printf("Comentator #%d's turn to speak for %.3f seconds\n",com_num,time);
      pthread_mutex_unlock(&access_global_queue_mutex);
      pthread_cond_signal(&comment_conds[com_num]);
      pthread_mutex_lock(&access_global_queue_mutex);
    }
    req_id = 0;
    pthread_mutex_unlock(&access_global_queue_mutex);
  }

  pthread_exit(0);
}

bool initialize_values(int argc, char *argv[]){

  num_commentators = 4;
  prob_to_answer = 1;
  num_questions = 5;
  max_speak_time = 3;
  prob_to_breaking_news = 0.05;
  if(argc%2 == 1){
    for(int i=1; i < argc; i+=2){
      if(argv[i][0] == '-'){
        if(strcmp(argv[i],"-n") == 0){
          num_commentators = atoi(argv[i+1]);
        }else if(strcmp(argv[i],"-p") == 0){
          prob_to_answer = strtof(argv[i+1], NULL);
        }else if(strcmp(argv[i],"-q") == 0){
          num_questions = atoi(argv[i+1]);
        }else if(strcmp(argv[i],"-t") == 0){
          max_speak_time = strtol(argv[i+1],NULL,0);
        }else if(strcmp(argv[i],"-b") == 0){
          prob_to_breaking_news = strtof(argv[i+1], NULL);
        }else{
          return false;
        }
      }else{
        return false;
      }
    }
  }else{
    return false;
  }
  return true;
}

bool initialize_threads(){


  // initialize mutex, attr and cond_var.
  pthread_barrier_init(&question_barrier,NULL,num_commentators+1);

  pthread_mutex_init(&access_global_queue_mutex, NULL);
  pthread_mutex_init(&question_mutex, NULL);
  pthread_cond_init(&question_asked,NULL);




  for(long i=0; i<num_commentators; i++){
    if(pthread_cond_init(&comment_conds[i], NULL) != 0)
      return false;
    if(pthread_create(&commentators[i], &thread_attribute, request, (void *)i) != 0)
      return false;
  }

  if(pthread_create(&moderator, &thread_attribute, moderate, NULL) != 0)
    return false;

  return true;

}


int main(int argc, char *argv[]) {
  start = clock();
  if(!initialize_values(argc,argv)){
    printf("Argument Error. Exiting.\n");
  }
  if(!initialize_threads()){
    printf("Thread Error. Exiting.\n");
  }

  // join created threads.
  for(long i=0; i<num_commentators; i++){
    pthread_join(commentators[i], NULL);
    pthread_cond_destroy(&comment_conds[i]);
  }

  pthread_join(moderator, NULL);

  // destroy attr and mutex.
  // pthread_attr_destroy(&thread_attribute);
  pthread_barrier_destroy(&question_barrier);
  pthread_mutex_destroy(&access_global_queue_mutex);
  pthread_mutex_destroy(&question_mutex);
  pthread_cond_destroy(&question_asked);
  pthread_exit(NULL);
  return 0;
}
