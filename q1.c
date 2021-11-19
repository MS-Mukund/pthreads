#include "q1.h"

int main()
{
    srand(time(0));

    init();

    int num_students, num_labs, num_courses;

    scanf("%d%d%d", &num_students, &num_labs, &num_courses);

    course cs[num_courses];

    for( int i = 0; i < num_courses; i++)
    {
        scanf("%s%f%d", cs[i].name, &cs[i].interest, &cs[i].num_labs);

        cs[i].labs = (int *)malloc(num_labs*sizeof(int));
        assert(cs[i].labs != NULL );
        for( int j = 0; j < cs[i].num_labs; j++)
        {
            scanf("%d", cs[i].labs[j]);
        }

        pthread_create(&(cs[i].course_thr_id, NULL, start_c, &(cs[i]) ), );
    }

    stud st[num_students];
    int st_id = 0;
    for( int i = 0; i < num_students; i++ )
    {
        st[i].id = st_id++;
        st[i].state = waiting;

        scanf("%f%d%d%d%d", &st[i].calibre, &st[i].pref[0], &st[i].pref[1], &st[i].pref[2], &st[i].time);
        pthread_create(&(st[i].stud_thread_id), NULL, start_stud, &st[i]);
    }
    
    for( int i = 0; i < num_students; i++ )
    {
        pthread_join(st[i].stud_thread_id, NULL);
    }
    
    destroy();
}

void *start_c(void *args)
{
    course *c = (course *)args;
    int cur_ta_id = -1;
    int cur_ta_lab = -1;


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
    printf( "Student %d has selected course %d permanently\n", id, course_no);
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
