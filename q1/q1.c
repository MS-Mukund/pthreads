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
        all_courses[i].name = (char *)malloc(sizeof(char *));
        scanf("%s%f%d%d", all_courses[i].name, &all_courses[i].interest, &all_courses[i].max_slots, &all_courses[i].num_labs);
        // printf("%s %f %d\n", all_courses[i].name, all_courses[i].interest, all_courses[i].num_labs);

        all_courses[i].labs = (int *)malloc( num_labs*sizeof(int) );
        assert(all_courses[i].labs != NULL );

        for( int j = 0; j < all_courses[i].num_labs; j++)
        {
            scanf("%d", &all_courses[i].labs[j]);
            // printf("%d \n", all_courses[i].labs[j]);
        }

        pthread_cond_init( &(all_courses[i].attend), NULL);
        pthread_mutex_init( &(all_courses[i].attend_lock), NULL);
        pthread_mutex_init( &(all_courses[i].ppl_lock), NULL);
        all_courses[i].num_stud = 0;
        
        pthread_mutex_lock( &(all_courses[i].ppl_lock) );
        all_courses[i].ppl_there = 0;
        pthread_mutex_unlock( &(all_courses[i].ppl_lock) );
        pthread_cond_init( &(all_courses[i].ppl_sig), NULL);

        sem_init(&all_courses[i].wait_for_seat, 0, 0);
        sem_init(&all_courses[i].wait_end_tute, 0, 0);

        all_courses[i].removed = false;
    }

    stud st[num_students];
    int st_id = 0;
    for( int i = 0; i < num_students; i++ )
    {
        st[i].id = st_id++;
        st[i].state = not_registered;
        scanf("%f%d%d%d%d", &st[i].calibre, &st[i].pref[0], &st[i].pref[1], &st[i].pref[2], &st[i].time);
        // printf("%f%d%d%d%d\n", st[i].calibre, st[i].pref[0], st[i].pref[1], st[i].pref[2], st[i].time);
    }

    all_labs = (lab *)malloc(num_labs*sizeof(lab));
    assert(all_labs != NULL);

    lab_locks = (sem_t *)malloc( num_labs*sizeof(sem_t) );
    assert(lab_locks != NULL);

    for( int i = 0; i < num_labs; i++)
    {
        all_labs[i].name = (char *)malloc(sizeof(char *));
        scanf("%s%d%d", all_labs[i].name, &all_labs[i].num_tas, &all_labs[i].max_no_times );
        // printf("labs \n\n%s %d %d\n", all_labs[i].name, all_labs[i].num_tas, all_labs[i].max_no_times );

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
    pthread_mutex_lock(&scan_lock);
    done = 1;
    pthread_cond_signal(&scan_sig);
    pthread_mutex_unlock(&scan_lock);

    for( int i = 0; i < num_students; i++ )
    {
        pthread_create(&(st[i].stud_thread_id), NULL, start_stud, &(st[i]) );
    }

    for( int i = 0; i < num_courses; i++)
    {
        pthread_create(&(all_courses[i].course_thr_id), NULL, start_c, &(all_courses[i]) );
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

    return 0;
}

void *start_c(void *args)
{
    course *c = (course *)args;
    pthread_mutex_lock(&scan_lock);
    while( done == 0 )
    {
        pthread_cond_wait(&(scan_sig), &scan_lock);
    }
    pthread_mutex_unlock(&scan_lock);

    while(1)
    {
        int status = 0;
        int cur_ta_id = -1;
        int cur_lab_id = -1;
        int num_taed = -1;
        char *cur_lab_name = NULL;

        // no student is waiting for course
        pthread_mutex_lock(&(c->ppl_lock));
        while(c->ppl_there == 0)
            pthread_cond_wait(&(c->ppl_sig), &(c->ppl_lock));
        pthread_mutex_unlock(&(c->ppl_lock));

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
                        num_taed = all_labs[lab_id].num_taed[j];
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
            int tmp;
            pthread_mutex_lock(&(c->ppl_lock));
            tmp = c->ppl_there;
            c->ppl_there = 0;
            c->removed = true;
            pthread_mutex_unlock(&(c->ppl_lock));

            for( int i = 0; i < tmp; i++)
            {
                sem_post(&(c->wait_for_seat));
            }
            print_E10(c->name);
            return NULL;
        }

        // TA assigned
        else
        {
            print_E11(cur_ta_id, cur_lab_name, c->name, num_taed);

            int cur_seats = ( rand() % c->max_slots ) + 1;
            int num_alloc = cur_seats;
            print_E7(cur_lab_name, cur_seats);

            // allocating seats to students
            pthread_mutex_lock(&(c->ppl_lock));
            if( c->ppl_there < cur_seats )
            {
                num_alloc = c->ppl_there;
            }
            pthread_mutex_unlock(&(c->ppl_lock));

            int i;
            for( i = 0; i < num_alloc; i++ )
            {
                // wake up student threads waiting for seats
                sem_post( &c->wait_for_seat );            
            }

            // tute
            print_E8(c->name, num_alloc, cur_seats);
            sleep(1);

            // end tute
            pthread_mutex_lock(&(c->attend_lock));
            while( c->num_stud < num_alloc )
                pthread_cond_wait( &(c->attend), &(c->attend_lock) );
            pthread_mutex_unlock(&(c->attend_lock));

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
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += s->time;

    // students fills his preferences after 't' seconds
    if( s->state == not_registered )
    {
        sem_timedwait(&imaginary_lock, &ts);
        print_E1(id);
    }
    s->state = waiting;

    int pref_ind = 0;

    while( pref_ind < 3 )
    {
        pthread_mutex_lock(&(all_courses[s->pref[pref_ind]].ppl_lock));
        all_courses[s->pref[pref_ind]].ppl_there++;
        pthread_cond_signal(&(all_courses[s->pref[pref_ind]].ppl_sig));
        pthread_mutex_unlock(&(all_courses[s->pref[pref_ind]].ppl_lock));

        if( all_courses[s->pref[pref_ind]].removed == true )
        {
            goto removed;
        }
         
        int ret = sem_wait(&(all_courses[s->pref[pref_ind]].wait_for_seat));
        if( ret == -1)
        {
            perror("sem_wait error");
        }

        if( all_courses[s->pref[pref_ind]].removed == true )
        {
            goto removed;
        }
        print_E2(id, all_courses[s->pref[pref_ind]].name);
        s->state = attending;

        pthread_mutex_lock( &(all_courses[s->pref[pref_ind]].attend_lock) );
        all_courses[s->pref[pref_ind]].num_stud++;
        pthread_cond_signal( &(all_courses[s->pref[pref_ind]].attend) );
        pthread_mutex_unlock( &(all_courses[s->pref[pref_ind]].attend_lock) );
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

        // if course withdrawn or student withdraws from course    
        removed:
            pref_ind++;
            
            if( pref_ind == 3 )
            {
                print_E6(id);
                return NULL;
            }
            else
            {
                s->state = waiting;
                print_E4(id, pref_ind, all_courses[ s->pref[pref_ind-1] ].name, all_courses[ s->pref[pref_ind] ].name );
            }
        }
        else
        {
            print_E5(id, all_courses[ s->pref[pref_ind] ].name);
            s->state = finished;
            return NULL;
        }
    }

    // student didn't find any course
    print_E6(id);
    return NULL;
}

void init()
{
    // Initializing semaphores to zero
    sem_init(&imaginary_lock, 0, 0);
    // print lock initialised in q1.h
}

void destroy()
{
    // destroy semaphore/mutex after all the threads finished working
    pthread_mutex_destroy(&print_lock);
    sem_destroy(&imaginary_lock);
}

void print_E1( int id )
{
    pthread_mutex_lock(&print_lock);
    printf( GREEN"Student %d has filled in preferences for course registration\n"NORMAL, id);
    pthread_mutex_unlock(&print_lock);
}

void print_E2( int id, char *name )
{
    pthread_mutex_lock(&print_lock);
    printf( BLUE"Student %d has been allocated a seat in course %s\n"NORMAL, id, name);
    pthread_mutex_unlock(&print_lock);
}

void print_E3( int id, char *name )
{
    pthread_mutex_lock(&print_lock);
    printf( RED"Student %d has withdrawn from course %s\n"NORMAL, id, name);
    pthread_mutex_unlock(&print_lock);
}

void print_E4( int id, int priority, char *pref1, char *pref2 )
{
    pthread_mutex_lock(&print_lock);
    printf( CYAN"Student %d has changed current preference from %s (priority %d) to %s (priority %d)\n"NORMAL, id, pref1, priority, pref2, priority+1);
    pthread_mutex_unlock(&print_lock);
}

void print_E5( int id, char *name )
{
    pthread_mutex_lock(&print_lock);
    printf( MAGENTA"Student %d has selected course %s permanently\n"NORMAL, id, name);
    pthread_mutex_unlock(&print_lock);
}

void print_E6( int id )
{
    pthread_mutex_lock(&print_lock);
    printf( WHITE"Student %d could not get any of his preferred courses\n"NORMAL, id);
    pthread_mutex_unlock(&print_lock);
}

void print_E7( char *name, int seats )
{
    pthread_mutex_lock(&print_lock);
    printf( YELLOW"Course %s has been allocated %d seats\n"NORMAL, name, seats);
    pthread_mutex_unlock(&print_lock);
}

void print_E8( char *name, int filled, int seats)
{
    pthread_mutex_lock(&print_lock);
    printf( GREEN"Tutorial has started for Course %s with %d seats filled out of %d\n"NORMAL, name, filled, seats);
    pthread_mutex_unlock(&print_lock);
}

void print_E9( int ta_id, char *l_name, char *name )
{
    pthread_mutex_lock(&print_lock);
    printf( BLUE"TA %d from lab %s has completed the tutorial and left the course %s\n"NORMAL, ta_id, l_name, name);
    pthread_mutex_unlock(&print_lock);
}

void print_E10( char *name )
{
    pthread_mutex_lock(&print_lock);
    printf( MAGENTA"Course %s doesn???t have any TA???s eligible and is removed from course offerings\n"NORMAL, name);
    pthread_mutex_unlock(&print_lock);
}


void print_E11( int ta_id, char *l_name, char *name, int num )
{
    char filler[5];
    if( num == 1)
        strcpy(filler, "st");
    else if( num == 2)
        strcpy(filler, "nd");
    else if( num == 3)
        strcpy(filler, "rd");
    else
        strcpy(filler, "th");
    
    pthread_mutex_lock(&print_lock);
    printf( RED"TA %d from lab %s has been allocated to course %s for %d%s TA ship\n"NORMAL, ta_id, l_name, name, num, filler);
    pthread_mutex_unlock(&print_lock);
}

void print_E12( char *l_name )
{
    pthread_mutex_lock(&print_lock);
    printf( CYAN"Lab %s no longer has students available for TA ship\n"NORMAL, l_name);
    pthread_mutex_unlock(&print_lock);
}

/***
10 3 4
SMAI 0.8 3 2 0 2
NLP 0.95 4 1 0
CV 0.9 2 2 1 2
DSA 0.75 5 3 0 1 2
0.8 0 3 1 1
0.6 3 1 2 3
0.85 2 1 0 1
0.5 1 2 3 2
0.75 0 2 1 3
0.95 1 0 2 2
0.4 3 0 2 3
0.1 0 3 1 2
0.85 1 0 3 1
0.3 0 1 2 1
PRECOG 3 1
CVIT 4 2
RRC 1 3
***/