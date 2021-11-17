#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

// Some colors to judge different outputs when running.
#define NRM "\x1B[1;0m"
#define RED "\x1B[1;31m"
#define GRN "\x1B[1;32m"
#define YEL "\x1B[1;33m"
#define BLU "\x1B[1;34m"
#define MAG "\x1B[1;35m"
#define CYN "\x1B[1;36m"
#define WHT "\x1B[1;37m"

enum direction
{
    east = 1,
    west = 0
};

typedef struct Baboon
{
    int id;                     // Unique id for each baboon
    enum direction dir;         // Direction from which baboon came
    pthread_t baboon_thread_id; // unique identifier of the thread of a single baboon
} Baboon;

int waiting[2] = {0, 0};        // Number of baboons waiting from each direction
int travelling[2] = {0, 0};     // Number of baboons crossing the rope (only one of the direction will have positive value at a time)
sem_t sem[2];
pthread_mutex_t mutex;          // can also be a binary semaphore. Used to lock critical sections

void init()
{
    // Initializing semaphores to zero
    for (int i = 0; i < 2; i++)
        sem_init(&(sem[i]), 0, 0); 
    pthread_mutex_init(&mutex, NULL);
}

void destroy()
{
    // destroy semaphore/mutex after all the threads finished working
    pthread_mutex_destroy(&mutex);
    for (int i = 0; i < 2; i++)
    {  
        sem_destroy(&sem[i]); 
    }
}

void go_dir(Baboon *b)
{
    int id = b->id;
    enum direction dir = b->dir;

    int revdir = !dir;
    printf("%s%d is waiting from %d direction to get the rope\n", MAG, id, dir);

    sem_wait(&sem[dir]);

    pthread_mutex_lock(&mutex);
    // Update variables due to baboons that are ready to cross
    waiting[dir]--;
    travelling[dir]++;
    printf("%s%d Baboon started to cross from %d direction\n", YEL, id, dir);
    pthread_mutex_unlock(&mutex);

    // cross the rope
    sleep(2);

    pthread_mutex_lock(&mutex);

    travelling[dir]--;
    if (travelling[dir] == 0) // This is last baboon crossing
    {
        // If the current baboon that just crossed the rope was the last baboon, he will signal all the baboons waiting on the other direction at the current time to cross.
        // Example: if the baboon came from EAST and crossed to reach WEST, it will signal all the west baboons
        if (waiting[revdir] > 0)
        {
            // storing the number of baboons at the current time. Since multiple threads are working and changing waiting variable, storing this is necessary.
            int cnt = waiting[revdir]; 
            while (cnt--)
            {
                sem_post(&sem[revdir]);
            }
        }
        // if no baboon is waiting on the other side, signal all the baboons waiting on the side from which the last baboon came from to cross.
        else if (waiting[dir] > 0)
        {
            // same reasoning as above
            int cnt = waiting[dir];
            while (cnt--)
            {
                sem_post(&sem[dir]);
            }
        }
    }
    printf("%s%d Baboon crossed the rope from %d direction\n", CYN, id, dir);
    pthread_mutex_unlock(&mutex);
}

void *start(void *args)
{
    Baboon *b = (Baboon *)args;
    int id = b->id;
    enum direction dir = b->dir;

    int revdir = !dir;

    sleep(rand() % 2); // to mimic baboons coming at random time.

    pthread_mutex_lock(&mutex);
    // Since, the semaphores have been initialized to zero, to get started we allow the first thread (and any subseqent thread coming from the same direction) to increase the semaphore value
    if (travelling[dir] >= 0 && waiting[dir] == 0 && travelling[revdir] == 0 && waiting[revdir] == 0)
    {
        sem_post(&sem[dir]);
    }
    waiting[dir]++;
    printf("%s%d Baboon came from %d direction\n", BLU, id, dir);
    pthread_mutex_unlock(&mutex);

    go_dir(b);
    return NULL;
}

signed main()
{
    srand(time(0));

    init();
    int streams = 3;   // Just to simulate multiple streams of baboon.
    int baboon_id = 0; // to give unique id to each baboon
    while (streams--)
    {

        int x;
        printf("%sEnter the number of baboons: ", RED);
        scanf("%d", &x);
        Baboon t[x];
        for (int i = 0; i < x; i++)
        {
            t[i].id = ++baboon_id;
            t[i].dir = rand() & 1;
        }
        // Initiating a new thread for each baboon
        for (int i = 0; i < x; i++)
        {
            pthread_create(&(t[i].baboon_thread_id), NULL, start, &t[i]);
        }

        // Waiting for all threads to complete
        for (int i = 0; i < x; i++)
        {
            pthread_join(t[i].baboon_thread_id, NULL);
        }
    }

    destroy();
}