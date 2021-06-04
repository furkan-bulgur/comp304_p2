# t Seconds of Fame

We fully implemented the first part. Second part is not working yet, but we tried. 
Third part is working with a different implementation advised on the PDF. 

## How to Run

<pre>
A Makefile is included in the project. It can be executed by using 'make run' command with the specified arguments on the project description. 
To run the program with different parameters, use the following format: ./tv_debate.o -n 4 -p 0.75 -q 5 -t 3 -b 0.05

-n for number of commentator threads
-p for probability of giving answer of commentators
-q for number of questions
-t for maximum speak time of commentators
-b for probability of intervention of breaking news thread
</pre>


## Our İmplementation

<pre>
Main in tv_debate.cpp file initializes values and creates threads. By default, there are 4 commentator and 1 moderator threads. 
Moderator thread does its job in the moderate function, and commentators use request function.
After moderator asks a question, commentators generate answers with the predefined probability. If they have answer, they push 
their request into a global request queue.
Moderator thread begins to give permisson to speak to commentators by popping from the queue one by one. After queue becomes empty,
moderator asks its next question. It continues until all questions are asked.
</pre>

## Synchronization and Shared Data Protection

<pre>
We used 3 mutex locks, and a barrier.

- access_global_queue_mutex is to reserv the queue for only one thread at a time
- talk_mutex is to prevent intervention of other commentator threads while one is talking
- question_mutex is to wait moderator to ask its question

ask_to_answer_barrier is to wait all the commentator threads to reach a state before asking a new question

We also used 2 condition variables.
</pre>


