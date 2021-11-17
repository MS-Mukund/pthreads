#include "q1.h"

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

int main()
{
    srand(time(0));

    init();
    // int streams = 3;   // Just to simulate multiple streams of baboon.
    // int baboon_id = 0; // to give unique id to each baboon
    // while (streams--)
    // {

    //     int x;
    //     printf("%sEnter the number of baboons: ", RED);
    //     scanf("%d", &x);
    //     Baboon t[x];
    //     for (int i = 0; i < x; i++)
    //     {
    //         t[i].id = ++baboon_id;
    //         t[i].dir = rand() & 1;
    //     }
    //     // Initiating a new thread for each baboon
    //     for (int i = 0; i < x; i++)
    //     {
    //         pthread_create(&(t[i].baboon_thread_id), NULL, start, &t[i]);
    //     }

    //     // Waiting for all threads to complete
    //     for (int i = 0; i < x; i++)
    //     {
    //         pthread_join(t[i].baboon_thread_id, NULL);
    //     }
    // }
    int num_students, num_labs, num_courses;

    scanf("%d%d%d", &num_students, &num_labs, &num_courses);

    stud st[num_students];
    int st_id = 0;
    for( int i = 0; i < num_students; i++ )
    {
        st[i].id = st_id++;
        st[i].state = waiting;

        scanf("%f%d%d%d%d", &st[i].calibre, &st[i].pref[0], &st[i].pref[1], &st[i].pref[2], &st[i].time);
        pthread_create(&(st[i].id), NULL, start_stud, &st[i]);
    }
    
    for( int i = 0; i < num_students; i++ )
    {
        pthread_join(st[i].id, NULL);
    }
    
    destroy();
}

void *start_stud(void *args)
{
    stud *s = (stud *)args;
    int id = s->id;
    int cur_pref = s->pref[0];

    // if student is not registered yet, keep waiting, give up cpu
    if( s->time < ticks )
    {
        sched_yield();
    }

    if( s->pref[0] == -1)
    {
        cur_pref = s->pref[1];

        if( s->pref[1] == -1)
        {
            cur_pref = s->pref[2];
            if( s->pref[2] == -1)     // student must exit simulation.
            {
                s->state = exited;
                printf( "Student %d could not get any of his preferred courses\n", id );
                return NULL;
            }
        }
    }

}

void init()
{
    // Initializing semaphores to zero
    for (int i = 0; i < 2; i++)
        sem_init(&(sem[i]), 0, 0); 
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&print_lock, NULL);
}

void destroy()
{
    // destroy semaphore/mutex after all the threads finished working
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&print_lock);
    for (int i = 0; i < 2; i++)
    {  
        sem_destroy(&sem[i]); 
    }
}

void print_E1( int id )
{
    pthread_mutex_lock(&print_lock);
    printf( "Student %d has filled in preferences for course registration\n", id);
    pthread_mutex_unlock(&print_lock);
}

void print_E2( int id, int course_no )
{
    pthread_mutex_lock(&print_lock);
    printf( "Student %d has been allocated a seat in course %d\n", id, course_no);
    pthread_mutex_unlock(&print_lock);
}

void print_E3( int id, int course_no )
{
    pthread_mutex_lock(&print_lock);
    printf( "Student %d has withdrawn from course %d\n", id, course_no);
    pthread_mutex_unlock(&print_lock);
}

void print_E4( int id, int priority, char *pref1, char *pref2 )
{
    pthread_mutex_lock(&print_lock);
    printf( "Student %d has changed current preference from %s (priority %d) to %s (priority %d)\n", id, pref1, priority, pref2, priority+1);
    pthread_mutex_unlock(&print_lock);
}

void print_E5( int id, int course_no )
{
    pthread_mutex_lock(&print_lock);
    printf( "Student %d has been allocated a seat in course %d\n", id, course_no);
    pthread_mutex_unlock(&print_lock);
}

void print_E6( int id, int course_no )
{
    pthread_mutex_lock(&print_lock);
    printf( "Student %d has been allocated a seat in course %d\n", id, course_no);
    pthread_mutex_unlock(&print_lock);
}

void print_E7( int id, int course_no )
{
    pthread_mutex_lock(&print_lock);
    printf( "Student %d has been allocated a seat in course %d\n", id, course_no);
    pthread_mutex_unlock(&print_lock);
}

void print_E8( int id, int course_no )
{
    pthread_mutex_lock(&print_lock);
    printf( "Student %d has been allocated a seat in course %d\n", id, course_no);
    pthread_mutex_unlock(&print_lock);
}

void print_E9( int id, int course_no )
{
    pthread_mutex_lock(&print_lock);
    printf( "Student %d has been allocated a seat in course %d\n", id, course_no);
    pthread_mutex_unlock(&print_lock);
}

void print_E10( int id, int course_no )
{
    pthread_mutex_lock(&print_lock);
    printf( "Student %d has been allocated a seat in course %d\n", id, course_no);
    pthread_mutex_unlock(&print_lock);
}

void print_E11( int id, int course_no )
{
    pthread_mutex_lock(&print_lock);
    printf( "Student %d has been allocated a seat in course %d\n", id, course_no);
    pthread_mutex_unlock(&print_lock);
}

void print_E12( int id, int course_no )
{
    pthread_mutex_lock(&print_lock);
    printf( "Student %d has been allocated a seat in course %d\n", id, course_no);
    pthread_mutex_unlock(&print_lock);
}
