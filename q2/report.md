* Run `gcc q2.c -lpthread` and then `./a.out` to run the program. 

**Main**:
1. We initialize all the locks and semaphores and take the input given. We store the input in the variables (details given below). 

2. Then, for each fan, we create a thread and pass these variables as arguments to it. 

3. To simulate goal scoring, we create a thread. 

4. Finally, we wait for the threads to exit and then destroy all locks and semaphores. 

**How data is stored**:
1. The information of each group is stored in a struct. This struct contains the group id, the number of friends remaining (while waiting for friends to exit (bonus)) and a POINTER to the list of all the friends in the group. 

2. The information of each fan is stored in a struct 'fans', containing their thread id, zone, name, current state (watching match, waiting for seat etc), patience time etc. 

3. All the goal information such as goal time, probability of scoring, by which team is stored in an array of structs. 
   
**Locks and semaphores**:
1. There is a semaphore for each zone, which indicates whether or not a seat is available in that zone. 

2. There is a conditional variable and lock for each kind of fan, home, away and neutral, via which a fan of that kind standing in a queue can be signalled that a seat is empty. 

3. There is a lock and a conditional var which signals whenever a goal has been scored. 

4. [bonus] Each friend also waits for his group to arrive at the exit gate using a conditional variable. 
   
5. There is also a print_lock() which is used to prevent the threads from printing onto the terminal simultaneously.  
   
6. Finally, there is an imaginary_lock, which is actually a semaphore. As the name suggests, we never sem_post it, rather, we perform, sem_timedwait() on the lock for some seconds. This is used to implement the waiting for students to fill their preferences. 

**Fan Thread:** 
1. First we wait for some time (mentioned in input), using sem_timedwait() on the imaginary semaphore. 

2. Then, depending on the type of fan he is, we wait for a seat in the queue for one or multiple zones. If he could not get his seat in the patience time he has, he exits the simulation. 

3. Else, if he receives a signal during his patience time, we check which from which zone was the signal coming from (i.e. the zone where the seat was free). Note that this is only when there are multiple zones, for home and neutral fans. 

4. Then, the fan enters the stadium and starts watching. He is made to wait using pthread_cond_timedwait() for at max the spectating time. If in between, the number goals scored by the opposite team is greater than or equal to his rage limit (not applicable for neutral fans), he proceeds to the exit gate to wait for his friends. 

5. If it doesn't exceed his rage limit, he spectates the match for the spectating time only. Before going to the exit gate, we signal that a seat in this zone is free. 

6. Depending on the zone, we signal one or more of the 3 kinds of fans. The fans who are currently waiting in the queue will get the chance to watch the match. 

7. Then, using a conditional variable, we conditionally wait for everyone in the group to arrive at the exit gate. The last person arriving will print that they are leaving for dinner and wakes up all threads by using pthread_cond_broadcast().

**Goal Thread:**
1. We wait for the difference between the current goal oppurtunity and next goal oppurtunity. 

2. We check if the probability of scoring is greater than a random number. If it is, we signal all the spectators supporting opposite team (to check if they rage leave) using pthread_cond_broadcast(). 

3. We repeat till all oppurtunities have been exhausted. 

