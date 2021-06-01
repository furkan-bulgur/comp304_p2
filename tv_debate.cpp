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

using namespace std;

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

//threads
pthread_t commentators[5];  //change it to num_commentators
pthread_t moderator;
sem_t question_asked;
pthread_mutex_t access_a_queue_mutex;
pthread_attr_t thread_attribute;


int victim = 3;

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


// Posts new request to the queue.
void post_new_request_to_queue(int com_num) {
    //pthread_cond_signal(&police_stopped_playing_with_his_cell_phone);
    struct Request new_req;
    new_req.id = req_id;
    new_req.speak_time = 0.0;
    new_req.commantator_num = com_num;
    //queue<Commentator> *req_queue = &request_queue;
    request_queue.push(new_req);
    req_id = req_id + 1;
}

void *request(void *com_num) {
    int commentator_num = (long)com_num;

    while(num_questions>0){
      sem_wait(&question_asked);
      pthread_mutex_lock(&access_a_queue_mutex);
      //if it wants to answer, posts a request to the queue
      if((rand()/(float)RAND_MAX) < prob_to_answer) {
        post_new_request_to_queue(commentator_num);
      }
      printf("Exiting Commentator\n");
      pthread_mutex_unlock(&access_a_queue_mutex);
    }

    pthread_exit(0);
}

void *moderate(void *vargp) {

  for(int i=num_questions; i>0; i--){
    pthread_mutex_lock(&access_a_queue_mutex);
    sem_post(&question_asked);
    printf("Exiting Moderator\n");
    pthread_mutex_unlock(&access_a_queue_mutex);
    pthread_sleep(3);

    pthread_mutex_lock(&access_a_queue_mutex);
    while(request_queue.size()!=0) {
      struct Request first_req = request_queue.front();
      float t = first_req.speak_time;
      request_queue.pop();
      pthread_sleep(t);
    }
    req_id = 0;
    pthread_mutex_unlock(&access_a_queue_mutex);
  }

  pthread_exit(0);
}

bool initialize_values(int argc, char *argv[]){
  printf("argc : %d \n",argc);
  num_commentators = 4;
  prob_to_answer = 0.75;
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


int main(int argc, char *argv[]) {
  if(!initialize_values(argc,argv)){
    printf("Argument Error. Exiting.\n");
  }

  printf("Argument validity check:\n%d %f %d %ld %f\n", num_commentators,prob_to_answer,num_questions,
  max_speak_time,prob_to_breaking_news);
  
    //   // initialize mutex, attr and cond_var.
    //   pthread_mutex_init(&access_a_queue_mutex, NULL);
    //   sem_init(&question_asked,0,1);
    //   //pthread_attr_init(&thread_attribute);
    //
    //  pthread_create(&moderator, &thread_attribute, moderate, NULL);
    //  for(long i=0; i<num_commentators; i++)
    //   pthread_create(&commentators[i], &thread_attribute, request, (void *)i);
    //
    //  // join created threads.
    //  for(long i=0; i<num_commentators; i++)
    //   pthread_join(commentators[i], NULL);
    //  pthread_join(moderator, NULL);
    //
    //  // destroy attr and mutex.
    // // pthread_attr_destroy(&thread_attribute);
    //  pthread_mutex_destroy(&access_a_queue_mutex);
    //  sem_destroy(&question_asked);
    //  pthread_exit(NULL);
    //  return 0;
}
