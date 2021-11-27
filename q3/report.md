**To Run**:
- Run `make` first. 
- Then run `./server <number of worker threads>` and then `./client`. 

## Client:

**Data stored:**
- For each client thread, a struct is maintained, containing its id, message to be transmitted, and the time at which is has to be sent. 

**Locks and Semaphores:**
1. A print_lock is maintained to ensure that the output is printed by one thread at a time.
2. An imaginary semaphore is maintained to simulate waiting for some time interval using sem_timedwait(). 

**Main**:
1. We initially initialise all semaphores and locks. Then, we take the input and store it in the array of client structs. 

2. Then, we create the client threads and wait for them to finish. 

3. We destory the semaphores and locks and exit. 

**Client Thread:**
1. We first wait for some time given in the input. After that time, we get the socket descriptor using the socket() function call and perform some conversions for portability (converting to big endian). After that, we connect to the server using the connect() function call. 

2. Then, we send the message to the server using the write() function and perform some error checking. 

3. After this, we wait for a response from the server using the read() function and followed by some error checking, check that the message is in the expected format. 

4. Then, this message is printed on to the console. 

## Server: 

**Data stored:**
- A dictionary is present the server which contains the data that the clients are querying and modifying. The dictionary was implemented using an array. 

- A struct contains the information of each worker thread such as their id and thread_id. 

- A queue data structure is also used to queue all the messages that the clients are sending and the worker threads access this queue to provide a response. 

**Locks and Semaphores:**
1. A lock is present for the queue data structure. 

2. To ensure that no 2 clients simultaneously modify the data, we maintain a lock for each element in the dictionary.

3. A print_lock is maintained to ensure that the output is printed by one thread at a time.

4. An imaginary semaphore is maintained to simulate waiting for some time interval using sem_timedwait().  

**Main**:
1. After some error handling, we create the thread pool using pthread_create() and assign ids to them.  

2. Then, we create a socket for this server using socket(). Then, we perform some conversions for portability (converting to big endian). The port number is made to listen to any IP address. After that, we bind the socket to the port number using bind(). 

3. We listen to any connection requests using listen(). 

4. Whenever, the client sends any request, we accept the connection request using accept() and enqueue it in the queue and signal a worker thread waiting for work. 

5. We perform this until all connection requests are finished and all worker threads terminate. We finally destory the semaphores and locks, close the socket and exit. 

**Worker Thread:**
1. If the queue is empty, the thread conditionally waits for a client request. Any signal sent by the server is then received by this thread and the thread wakes up. 

2. This thread then dequeues the client request. Then, the thread reads the message from client and performs some error checking on the message. 

3. After this, if the command is to insert/update,the server inserts/updates the value corresponding to the key in the dictionary. if the command is to delete, then the thread deletes the values corrsponding to the key. 

4. If the command is to fetch the key, the thread, fetches the value corresponding to the key from the dictionary. Note that, for each of the above operations, we must acquire the corresponding lock to access the data. 

5. If the command is to concatenate, we acquire the locks for the 2 keys and concatenate the corresponding values and send back the 2nd value to the client. 

6. Note that, to avoid DEADLOCKS, we _always_ acquire the two locks in ascending order. Also, in any of the above operations, if the operands are illegal or if the value corresponding to the key does not exist (for update, fetch etc) and some other errors, we send an error message to the client. 

7. We send the return message to the client using write() and, after error checking, close the connection. The thread is now free for more work. 
