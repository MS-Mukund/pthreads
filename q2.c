#include "q2.h"
pthread_t goal_thr;

int main()
{
    srand(time(0));

    init();

    int zone_h, zone_a, zone_n;
    scanf("%d%d%d", &zone_h, &zone_a, &zone_n);
    sem_init(&hzone_sem, 0, zone_h);
    sem_init(&azone_sem, 0, zone_a);
    sem_init(&nzone_sem, 0, zone_n);

    scanf("%d", &spec_time);

    int num_grps;
    scanf("%d", &num_grps);

    grp_l = (group *)malloc(num_grps * sizeof(group));
    assert(grp_l != NULL);

    wait_for_grp = (pthread_cond_t *)malloc(num_grps * sizeof(pthread_cond_t));
    assert(wait_for_grp != NULL);
    for( int i = 0; i < num_grps; i++ )
        pthread_cond_init(&wait_for_grp[i], NULL);

    frnd_lock = (pthread_mutex_t *)malloc(num_grps * sizeof(pthread_mutex_t));
    assert(frnd_lock != NULL);
    for( int i = 0; i < num_grps; i++ )
        pthread_mutex_init(&frnd_lock[i], NULL);

    int num_ppl;
    for( int ct = 0; ct < num_grps; ct++)
    {
        scanf("%d", &num_ppl);
        grp_l[ct].rem_friends = num_ppl;
        grp_l[ct].id = ct;
        pthread_mutex_init(&grp_l[ct].lock, NULL);

        grp_l[ct].fan_l = (fans *)malloc(num_ppl * sizeof(fans));
        assert(grp_l[ct].fan_l != NULL);

        for( int ct2 = 0; ct2 < num_ppl; ct2++)
        {
            grp_l[ct].fan_l[ct2].g_id = ct;
            grp_l[ct].fan_l[ct2].state = not_reached;
            grp_l[ct].fan_l[ct2].name = (char *)malloc(sizeof(char *));
            scanf("%s%s%d%d%d", grp_l[ct].fan_l[ct2].name, grp_l[ct].fan_l[ct2].zone, &grp_l[ct].fan_l[ct2].reach_t, &grp_l[ct].fan_l[ct2].patience, &grp_l[ct].fan_l[ct2].num_goals);
            // printf("%s %c %d %d %d\n", grp_l[ct].fan_l[ct2].name, grp_l[ct].fan_l[ct2].zone, grp_l[ct].fan_l[ct2].reach_t, grp_l[ct].fan_l[ct2].patience, grp_l[ct].fan_l[ct2].num_goals);
        }
    }

    int num_goals;
    scanf("%d", &num_goals);

    goals = (g_info *)malloc(num_goals * sizeof(g_info));
    assert(goals != NULL);
    for( int i = 0; i < num_goals; i++)
    {
        scanf("%s%d%f", goals[i].team, &goals[i].time, &goals[i].prob);
    }

    // for( int ct = 0; ct < num_grps; ct++)
    // {
    //     printf("%d\n", grp_l[ct].rem_friends);
        
    //     for( int ct2 = 0; ct2 < grp_l[ct].rem_friends; ct2++)
    //     {
    //         printf("%s %s %d %d %d\n", grp_l[ct].fan_l[ct2].name, grp_l[ct].fan_l[ct2].zone, grp_l[ct].fan_l[ct2].reach_t, grp_l[ct].fan_l[ct2].patience, grp_l[ct].fan_l[ct2].num_goals);
    //     }
    // }

    // printf("%d\n", num_goals);
    // for( int i = 0; i < num_goals ; i++)
    // {
        // printf("%s %d %f\n", goals[i].team, goals[i].time, goals[i].prob);
    // }

    pthread_create(&goal_thr, NULL, goal_scored, (void *)&num_goals);
    for( int ct = 0; ct < num_grps; ct++)
    {
        for( int ct2 = 0; ct2 < grp_l[ct].rem_friends; ct2++)
        {
            pthread_create(&grp_l[ct].fan_l[ct2].fan_t_id, NULL, fan_func, &grp_l[ct].fan_l[ct2]);
        }
    }
    // printf("created threads\n");
    for( int ct = 0; ct < num_grps; ct++)
    {
        for( int ct2 = 0; ct2 < num_ppl; ct2++)
        {
            // printf("ok\n");
            pthread_join(grp_l[ct].fan_l[ct2].fan_t_id, NULL);
            // printf("what\n");
        }
        // printf("here22\n");
    }  
    // printf( "done with students\n");
    pthread_join(goal_thr, NULL);
    // printf( "all done\n");
    destroy();

    return 0;
}

int signal_relevant_zone(int indicator)
{
    assert(indicator != -1);

    if( indicator == 1 ) // home
    {
        sem_post(&hzone_sem);

        pthread_mutex_lock(&hfq_lock);
        pthread_cond_signal(&hfan_q);
        pthread_mutex_unlock(&hfq_lock);

        return 1;
    }
    else if (indicator == 2) // neutral
    {
        sem_post(&nzone_sem);

        pthread_mutex_lock(&nfq_lock);
        pthread_cond_signal(&nfan_q);
        pthread_mutex_unlock(&nfq_lock);

        return 2;
    }
    else if (indicator == 3) // away
    {
        sem_post(&azone_sem);

        pthread_mutex_lock(&afq_lock);
        pthread_cond_signal(&afan_q);
        pthread_mutex_unlock(&afq_lock);

        return 3;
    }
    else
    {
        printf("Error: invalid indicator\n");
        return -1;
    }
}

void *fan_func(void *args)
{
    fans *fan = (fans *)args;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += fan->reach_t;

    sem_timedwait(&imaginary_lock, &ts);

    // reached stadium
    fan->state = ticket_wait;
    print_E1(fan->name);

    int sem_ret = 0, stat = 0;
    int time_ret = 0;
    // wait for ticket
    if( fan->zone[0] == 'A')
    {
        sem_ret = sem_trywait(&azone_sem);   // try for away zone seat
        if( sem_ret == 0)
        {
            print_E2(fan->name, 'A');
            stat = 3;
        }

        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += fan->patience;
        while( stat == 0)
        {
            pthread_mutex_lock(&afq_lock);
            time_ret = pthread_cond_timedwait(&afan_q, &afq_lock, &ts);
            if( errno == ETIMEDOUT)     // waited in q, couldn't get seat
            {
                print_E3(fan->name);
                stat = -1;
                return NULL;
            }
            pthread_mutex_unlock(&afq_lock);

            if( time_ret == 0)      
            {
                sem_trywait(&azone_sem);
                if( sem_ret == 0)
                {
                    print_E2(fan->name, 'A');
                    stat = 3;
                }
            }

            // if stat is still zero, it means some bogus signal came

        }   
    }

    else if( fan->zone[0] == 'H')
    {
        sem_ret = sem_trywait(&hzone_sem);   // try for home zone seat
        if( sem_ret == 0)
        {
            print_E2(fan->name, 'H');
            stat = 1;
        }
        else
        {
            sem_ret = sem_trywait(&nzone_sem);   // try for neutral zone seat
            if( sem_ret == 0)
            {
                print_E2(fan->name, 'N');
                stat = 2;
            }
        }

        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += fan->patience;
        while( stat == 0 )
        {
            pthread_mutex_lock(&hfq_lock);
            time_ret = pthread_cond_timedwait(&hfan_q, &hfq_lock, &ts);
            if( errno == ETIMEDOUT )     // waited in q, couldn't get seat
            {
                print_E3(fan->name);
                stat = -1;
                return NULL;
            }
            pthread_mutex_unlock(&hfq_lock);

            if( time_ret == 0)      
            {
                sem_trywait(&hzone_sem);
                if( sem_ret == 0)
                {
                    print_E2(fan->name, 'H');
                    stat = 1;
                }
                else if( ( sem_ret = sem_trywait(&nzone_sem) ) == 0)
                {
                    print_E2(fan->name, 'N');
                    stat = 2;
                }
            }
        }   
    }

    else if( fan->zone[0] == 'N')
    {
        if( ( sem_ret = sem_trywait(&hzone_sem) ) == 0 )            // try for home zone seat
        {
            print_E2(fan->name, 'H');
            stat = 1;
        }
        else if( ( sem_ret = sem_trywait(&nzone_sem) ) == 0 )       // try for neutral zone seat
        {
            print_E2(fan->name, 'N');
            stat = 2;
        }
        else if( ( sem_ret = sem_trywait(&azone_sem) ) == 0 )       // try for away zone seat
        {
            print_E2(fan->name, 'A');
            stat = 3;
        }

        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += fan->patience;
        while( stat == 0 )
        {
            pthread_mutex_lock(&nfq_lock);
            time_ret = pthread_cond_timedwait(&nfan_q, &nfq_lock, &ts);
            if( errno == ETIMEDOUT)     // waited in q, couldn't get seat
            {
                print_E3(fan->name);
                stat = -1;
                return NULL;
            }
            pthread_mutex_unlock(&nfq_lock);

            if( time_ret == 0)      
            {
                if( (sem_ret = sem_trywait(&hzone_sem) ) == 0)
                {
                    print_E2(fan->name, 'H');
                    stat = 1;
                }
                else if( ( sem_ret = sem_trywait(&nzone_sem) ) == 0)
                {
                    print_E2(fan->name, 'N');
                    stat = 2;
                }
                else if( ( sem_ret = sem_trywait(&azone_sem) ) == 0)
                {
                    print_E2(fan->name, 'A');
                    stat = 3;
                }
            }
        }   
    }

    // got seat
    fan->state = watching;  
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += spec_time;

    if( fan->zone[0] == 'N')
    {
        // he watches full game
        sem_timedwait(&imaginary_lock, &ts);
        print_E4(fan->name);
        signal_relevant_zone(stat);
    }

    else if( fan->zone[0] == 'A')
    {
       // watches until rage limit 
       while( cur_goals_home < fan->num_goals )
       {
           pthread_mutex_lock(&hgoal_lock);
           pthread_cond_timedwait(&home_sig, &hgoal_lock, &ts);
           if( errno == ETIMEDOUT )
           {
               print_E4(fan->name);
               pthread_mutex_unlock(&hgoal_lock);
               break;
           }
           pthread_mutex_unlock(&hgoal_lock);

           if( cur_goals_home >= fan->num_goals )
           {
               break;
           }
       }
       if( cur_goals_home >= fan->num_goals )
            print_E5(fan->name);
       
       fan->state = exit_wait;
       signal_relevant_zone(stat);
    }

    else if( fan->zone[0] == 'H')
    {
       // watches until rage limit 
       while( cur_goals_away <= fan->num_goals )
       {
           pthread_mutex_lock(&agoal_lock);
           pthread_cond_timedwait(&away_sig, &agoal_lock, &ts);
           if( errno == ETIMEDOUT )
           {
               print_E4(fan->name);
               fan->state = exit_wait;
               signal_relevant_zone(stat);
               pthread_mutex_unlock(&agoal_lock);
               break;
           }
           pthread_mutex_unlock(&agoal_lock);

           if( cur_goals_away > fan->num_goals )
           {
               print_E5(fan->name);
               fan->state = exit_wait;
               signal_relevant_zone(stat);
               break;
           }
       }
    }
    // printf("done watching\n");

    // he's done watching, waiting for friends
    print_E6(fan->name);
    int tmp;
    pthread_mutex_lock(&grp_l[fan->g_id].lock);
    tmp = (--grp_l[fan->g_id].rem_friends );
    pthread_mutex_unlock(&grp_l[fan->g_id].lock);
    if( tmp > 0)
    {
        pthread_mutex_lock(&frnd_lock[fan->g_id]);
        pthread_cond_wait(&wait_for_grp[fan->g_id], &frnd_lock[fan->g_id]);
        pthread_mutex_unlock(&frnd_lock[fan->g_id]);
    }
    else    // everyone in the grp exits
    {
        pthread_mutex_lock(&frnd_lock[fan->g_id]);
        pthread_cond_broadcast(&wait_for_grp[fan->g_id]);
        pthread_mutex_unlock(&frnd_lock[fan->g_id]);

        print_E9(fan->g_id+1);
    }

    fan->state = dinner;
    // printf("dinner\n");
    pthread_exit(NULL);
}

void *goal_scored(void *args)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += goals[0].time;
    int ptr = 0;
    int prev = 0;
    
    int num_goals = *(int *)args;
    // wait till next goal is scored
    while( ptr < num_goals )
    {
        sem_timedwait(&imaginary_lock, &ts);
        int cur_gls = 1 + cur_goals_home;
        if( (rand() % 100) <= goals[ptr].prob * 100 )
        {
            // goal scored
            if( goals[ptr].team[0] == 'A' )
            {
                pthread_mutex_lock(&agoal_lock);
                cur_gls = ++cur_goals_away;
                pthread_cond_broadcast(&away_sig);
                pthread_mutex_unlock(&agoal_lock);
            }
            else
            {
                pthread_mutex_lock(&hgoal_lock);
                cur_gls = ++cur_goals_home;
                pthread_cond_broadcast(&home_sig);
                pthread_mutex_unlock(&hgoal_lock);
            }
            print_E7(goals[ptr].team[0], cur_gls);
        }
        else
        {
            if( goals[ptr].team[0] == 'A' )
            {
                cur_gls = cur_goals_away + 1;
            }
            // no goal
            print_E8(goals[ptr].team[0], cur_gls);
        }
        ptr++;
        prev = goals[ptr-1].time;
        ts.tv_sec += goals[ptr].time - prev;
    }
    // printf("all goals scored %d\n", num_goals);
    return NULL;
}

void init()
{
    // Initializing semaphores to zero
    sem_init(&imaginary_lock, 0, 0);    
}

void destroy()
{
    // destroy semaphore/mutex after all the threads finished working
    pthread_mutex_destroy(&print_lock);
    sem_destroy(&imaginary_lock);
}

void print_E1( char *name )
{
    pthread_mutex_lock(&print_lock);
    printf( "%s has reached the stadium\n", name );
    pthread_mutex_unlock(&print_lock);
}

void print_E2( char *name, char zone )
{
    pthread_mutex_lock(&print_lock);
    printf( "%s has got a seat in zone %c\n", name, zone );
    pthread_mutex_unlock(&print_lock);
}

void print_E3(char *name )
{
    pthread_mutex_lock(&print_lock);
    printf( "%s couldn't get a seat\n", name);
    pthread_mutex_unlock(&print_lock);
}

void print_E4( char *name )
{
    pthread_mutex_lock(&print_lock);
    printf( "%s watched the match for %d seconds and is leaving \n", name, spec_time );
    pthread_mutex_unlock(&print_lock);
}

void print_E5( char *name )
{
    pthread_mutex_lock(&print_lock);
    printf( "%s is leaving due to the bad defensive performance of his team\n", name);
    pthread_mutex_unlock(&print_lock);
}

void print_E6( char *name)
{
    pthread_mutex_lock(&print_lock);
    printf( "%s is waiting for their friends at the exit\n", name);
    pthread_mutex_unlock(&print_lock);
}

void print_E7( char name, int num )
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
    printf( "Team %c have scored their %d%s goal\n", name, num, filler);
    pthread_mutex_unlock(&print_lock);
}

void print_E8( char name, int num)
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
    printf( "Team %c missed the chance to score their %d%s goal\n", name, num, filler);
    pthread_mutex_unlock(&print_lock);
}

void print_E9( int grp_no )
{
    pthread_mutex_lock(&print_lock);
    printf( "Group %d is leaving for dinner\n", grp_no);
    pthread_mutex_unlock(&print_lock);
}

/**
2 1 2
3
2
3
Vibhav N 3 2 -1
Sarthak H 1 3 2
Ayush A 2 1 4
4
Rachit H 1 2 4
Roshan N 2 1 -1
Adarsh A 1 2 1
Pranav N 3 1 -1
5
H 1 1
A 2 0.95
A 3 0.5
H 5 0.85
H 6 0.4 
 */