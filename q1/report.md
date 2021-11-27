* Run `gcc q1.c -lpthread` and then `./a.out` to run the program. 

**main()**:
1. Initially, the program takes all the required input. Then it creates a thread using pthread_create() for each course and for each student. 
   
2. Finally, main, waits for each thread (both for student and course) to finish using pthread_join().  

**How data is stored**:
1. For each course, I have maintained a struct which contains all information about it such as it's name, maximum no of slots, avg interest in the course. 
   
2. Similarly, for each student, I have maintained a struct which contains all information about it such as his/her id, time after which he fills preferences, his preferences etc. 
   
3. I have implemented the labs as an array of structs. Each struct contains the name of the labs, number of TAs, maximum no of times a student of the lab can TA any course. It also contains 2 arrays. One array indicates whether or not a TA is occupied (in a tutorial) and other array indicates the no of times each TA has TAed any course as of now. 
   
**Locks and semaphores**:
1. For each lab, I have maintained a lock, so that no two threads scan the lab for TAs simultaneously. 
   
2. There is also a print_lock() which is used to prevent the threads from printing onto the terminal simultaneously.  
   
3. Each course also contains an attend lock, which is used to ensure that every thread is actually waiting on the thread before starting to sem_post() that the tutorial has ended. 

4. Finally, there is an imaginary_lock, which is actually a semaphore. As the name suggests, we never sem_post it, rather, we perform, sem_timedwait() on the lock for some seconds. This is used to implement the waiting for students to fill their preferences. 

**Course Thread:** 
1. We initially conditionally wait till some students are waiting for course to start (to prevent tutorials from starting before students are ready). 

2. Then, we iterate through the list of labs, acquiring lock for each lab, then checking for each TA in the lab, whether they are free or not. If they are, then we make them occupied and ask them to take up the tutorial. Else, if no one is free or if they have exhausted their tutorials, we release the lock and go to the next lab. 

3. If in every lab, the TAs have exhausted their tutorials, we then, we withdraw the course. 

4. If course is withdrawn, we signal everyone waiting for the course to move on to the next preference. 

5. Otherwise, we allocate some random seats between 1 to max_seats. Then, we allocate seats to all people waiting on the tutorial (and stop if it reaches the seat_limit we just found) and start the tutorial. 

6. Then, after the end, we check if every student who has been allocated the tutorial is attending or not and then release all the students (sem_post()). Then the TA is made free. 

7. This process is repeated till the course is withdrawn. 

**Student Thread:**
1. Initially, we wait for a certain duration (given as input), using sem_timedwait() on an imaginary semaphore which will never be posted. Then, we print that the student is registered.

2. Then, the student is made to wait for a seat in the course of his first preference. Then, the student is woken up by the course thread. If the course was removed, he moves to the next preference. 
   
3. Else, he attends the tutorial and waits for it to end. Then, based on his interest in the course and his calibre, we randomly evaluate whether he will choose the course or not. 

4. If he chooses the thread, then he is done. Else, we move on to the next preference. If all his 3 preferences are exhausted, we exit the simulation. 

