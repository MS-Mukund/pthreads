#include "q1.h"

int main()
{
    srand(time(0));

    init();

    int num_students, num_labs, num_courses;

    scanf("%d%d%d", &num_students, &num_labs, &num_courses);

    all_courses = (course *)malloc(num_courses * sizeof(course));
    assert(all_courses != NULL);

    for( int i = 0; i < num_courses; i++)
    {
        scanf("%s%f%d", all_courses[i].name, &all_courses[i].interest, &all_courses[i].num_labs);

        all_courses[i].labs = (int *)malloc( num_labs*sizeof(int) );
        assert(all_courses[i].labs != NULL );

        for( int j = 0; j < all_courses[i].num_labs; j++)
        {
            scanf("%d", all_courses[i].labs[j]);
        }

        pthread_cond_init( &(all_courses[i].attend), NULL);
        pthread_mutex_init( &(all_courses[i].attend_lock), NULL);
        all_courses[i].num_stud = 0;

        sem_init(&all_courses[i].wait_for_seat, 0, 0);
        sem_init(&all_courses[i].wait_end_tute, 0, 0);
        pthread_create(&(all_courses[i].course_thr_id), NULL, start_c, &(all_courses[i]) );
    }

    stud st[num_students];
    int st_id = 0;
    for( int i = 0; i < num_students; i++ )
    {
        st[i].id = st_id++;
        st[i].state = not_registered;
        scanf("%f%d%d%d%d", &st[i].calibre, &st[i].pref[0], &st[i].pref[1], &st[i].pref[2], &st[i].time);

        pthread_create(&(st[i].stud_thread_id), NULL, start_stud, &st[i]);
    }

    all_labs = (lab *)malloc(num_labs*sizeof(lab));
    assert(all_labs != NULL);

    lab_locks = (sem_t *)malloc( num_labs*sizeof(sem_t) );
    assert(lab_locks != NULL);

    for( int i = 0; i < num_labs; i++)
    {
        scanf("%s%d%d", all_labs[i].name, &all_labs[i].num_tas, &all_labs[i].max_no_times );

        all_labs[i].num_taed = (int *)malloc( all_labs[i].num_tas*sizeof(int) );
        assert(all_labs[i].num_taed != NULL );

        all_labs[i].occupied = (bool *)malloc( num_labs*sizeof(bool) );
        assert(all_labs[i].occupied != NULL );

        for( int j = 0; j < all_labs[i].num_tas; j++)
        {
            all_labs[i].num_taed[j] = 0;
            all_labs[i].occupied[j] = false;
        }

        sem_init(&(lab_locks[i]), 0, 1);
    }
    
    for( int i = 0; i < num_students; i++ )
    {
        pthread_join(st[i].stud_thread_id, NULL);
    }
    
    for( int i = 0; i < num_courses; i++ )
    {
        pthread_cancel(all_courses[i].course_thr_id);
    }

    destroy();
}

void *start_c(void *args)
{
    course *c = (course *)args;
    
    while(1)
    {
        int status = 0;
        int cur_ta_id = -1;
        int cur_lab_id = -1;
        char *cur_lab_name = NULL;
        // TA allocation
        for( int i = 0; i < c->num_labs; i++)
        {
            int lab_id = c->labs[i];
            if( lab_id == -1 )
            {
                continue;
            }

            int lab_stat = 0;       // 1 - lab is free, 0 - lab is full
            cur_lab_id = lab_id;

            sem_wait( &lab_locks[lab_id] );
            cur_lab_name = all_labs[lab_id].name;

            for( int j = 0; j < all_labs[lab_id].num_tas; j++)
            {
                if( all_labs[lab_id].num_taed[j] < all_labs[lab_id].max_no_times )
                {
                    if( all_labs[lab_id].occupied[j] == false )
                    {
                        all_labs[lab_id].num_taed[j]++;
                        cur_ta_id = j;
                        all_labs[lab_id].occupied[j] = true;

                        status = 1;
                        break;
                    }

                    else
                    {
                        lab_stat = 1;
                    }
                }
            }
            sem_post( &lab_locks[lab_id] );

            if( status == 1 )
            {
                break;
            }
            else if( lab_stat == 0)
            {
                print_E12(cur_lab_name);
                c->labs[i] = -1;
            }
        }

        // course withdrawn
        if( status == 0 )
        {
            print_E10(c->name);
            return NULL;
        }

        // TA assigned
        else
        {
            print_E11(cur_ta_id, cur_lab_name, c->name);

            int cur_seats = ( rand() % c->max_slots ) + 1;
            int num_alloc = 0;
            print_E7(cur_lab_name, cur_seats);

            // allocating seats to students
            int i;
            for( i = 0; i < cur_seats; i++ )
            {
                // check if any student is waiting for seat
                int ret = sem_trywait(&c->wait_for_seat);
                if( ret < 0 && errno == EAGAIN )
                {   
                    print_E8(c->name, i, cur_seats);
                    break;
                }
    
                // wake up student threads waiting for seats
                sem_post( &c->wait_for_seat );            
            }
            num_alloc++;

            // tute
            sleep(1);

            // end tute
            acquire(&(c->attend_lock));
            while( c->num_stud < num_alloc )
                pthread_cond_wait( &(c->attend), &(c->attend_lock) );
            release(&(c->attend_lock));

            for( int i = 0; i < num_alloc; i++ )
            {
                sem_post( &c->wait_end_tute );
            }

            // make TA free
            sem_wait( &lab_locks[cur_lab_id] );
            all_labs[cur_lab_id].occupied[cur_ta_id] = false;
            sem_post( &lab_locks[cur_lab_id] );
            print_E9(cur_ta_id, cur_lab_name, c->name);
        }
    } 
}

void *start_stud(void *args)
{
    stud *s = (stud *)args;
    int id = s->id;
    
    // students fills his preferences after 't' seconds
    if( s->state == not_registered )
    {
        sem_timedwait(&imaginary_lock, &(s->time));
        print_E1(id);
    }
    s->state = waiting;

    int pref_ind = 0;

    while( pref_ind < 3 )
    {
        int ret = sem_wait(&(all_courses[s->pref[pref_ind]].wait_for_seat));
        if( ret == -1)
        {
            perror("sem_wait error");
        }
        print_E2(id, all_courses[s->pref[pref_ind]].name);
        s->state = attending;

        acquire( &(all_courses[s->pref[pref_ind]].attend_lock) );
        all_courses[s->pref[pref_ind]].num_stud++;
        pthread_cond_signal( &(all_courses[s->pref[pref_ind]].attend) );
        release( &(all_courses[s->pref[pref_ind]].attend_lock) );
        ret = sem_wait(&(all_courses[s->pref[pref_ind]].wait_end_tute));
        if( ret == -1)
        {
            perror("sem_wait error");
        }

        // decide if he wants to continue
        int rand_num = rand() % 1001;
        if( rand_num < (all_courses[s->pref[pref_ind]].interest)*(s->calibre)*1000 )
        {
            print_E3(id, all_courses[s->pref[pref_ind]].name);
            pref_ind++;
            
            if( pref_ind == 3 )
            {
                print_E6(id);
                return NULL;
            }
            else
                print_E4(id, pref_ind, all_courses[ s->pref[pref_ind-1] ].name, all_courses[ s->pref[pref_ind] ].name );
        }
        else
        {
            print_E5(id, all_courses[ s->pref[pref_ind] ].name);
            s->state = finished;
            return NULL;
        }
    }

    // student didn't find any course
    if( pref_ind >= 3 )
    {
        print_E6(id);
        return NULL;
    }
}

void init()
{
    // Initializing semaphores to zero
    for (int i = 0; i < 2; i++)
        sem_init(&(sem[i]), 0, 0); 

    sem_init(&imaginary_lock, 0, 0);
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

void print_E6( int id )
{
    pthread_mutex_lock(&print_lock);
    printf( "Student %d could not get any of his preferred courses\n", id);
    pthread_mutex_unlock(&print_lock);
}

void print_E7( char *name, int seats )
{
    pthread_mutex_lock(&print_lock);
    printf( "Course %s has been allocated %d seats\n", name, seats);
    pthread_mutex_unlock(&print_lock);
}

void print_E8( char *name, int filled, int seats)
{
    pthread_mutex_lock(&print_lock);
    printf( "Tutorial has started for Course %s with %d seats filled out of %d\n", name, filled, seats);
    pthread_mutex_unlock(&print_lock);
}

void print_E9( int ta_id, int l_name, char *name )
{
    pthread_mutex_lock(&print_lock);
    printf( "TA %d from lab %s has completed the tutorial and left the course %s\n", ta_id, l_name, name);
    pthread_mutex_unlock(&print_lock);
}

void print_E10( char *name )
{
    pthread_mutex_lock(&print_lock);
    printf( "Course %s doesn’t have any TA’s eligible and is removed from course offerings\n", name);
    pthread_mutex_unlock(&print_lock);
}

void print_E11( int ta_id, char *l_name, char *name )
{
    pthread_mutex_lock(&print_lock);
    printf( "TA %d from lab %s has been allocated to course %s for nth TA ship\n", ta_id, l_name, name);
    pthread_mutex_unlock(&print_lock);
}

void print_E12( int l_name )
{
    pthread_mutex_lock(&print_lock);
    printf( "Lab %s no longer has students available for TA ship\n", l_name);
    pthread_mutex_unlock(&print_lock);
}
