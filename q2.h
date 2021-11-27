#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/types.h>

// Some colors to judge different outputs when running.
#define NRM "\x1B[1;0m"
#define RED "\x1B[1;31m"
#define GRN "\x1B[1;32m"
#define YEL "\x1B[1;33m"
#define BLU "\x1B[1;34m"
#define MAG "\x1B[1;35m"
#define CYN "\x1B[1;36m"
#define WHT "\x1B[1;37m"

// Global variables

// lock to print to console
pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  scan_sig = PTHREAD_COND_INITIALIZER;
pthread_mutex_t scan_lock = PTHREAD_MUTEX_INITIALIZER;
int done = 0;

// semaphores
sem_t imaginary_lock;


enum fan_state { not_reached, ticket_wait, watching, exit_wait, dinner };
typedef struct spectators {
    int g_id;       // group id
    int patience;   // time which he waits for ticket 
    int num_goals;  // number of goals after which he's enraged
    int reach_t;    // time of reaching stadium
    pthread_t fan_t_id;   
    char *name;    // name of fan
    char  zone[2];      // home/away/neutral
    enum fan_state state;
} fans;

typedef struct friend_circle {
    int id;
    int rem_friends;
    pthread_mutex_t lock;
    fans *fan_l;
} group;

pthread_cond_t *wait_for_grp;
pthread_mutex_t *frnd_lock;
group *grp_l;

int spec_time;
int cur_goals_home = 0;
int cur_goals_away = 0;
typedef struct goal_info {
    int time;
    float prob;
    char team[2];
} g_info;

// signals when home or away teams score goals.
pthread_cond_t home_sig = PTHREAD_COND_INITIALIZER;
pthread_cond_t away_sig = PTHREAD_COND_INITIALIZER; 

pthread_mutex_t hgoal_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t agoal_lock = PTHREAD_MUTEX_INITIALIZER;
///

g_info *goals;
pthread_cond_t hfan_q = PTHREAD_COND_INITIALIZER;
pthread_cond_t afan_q = PTHREAD_COND_INITIALIZER;
pthread_cond_t nfan_q = PTHREAD_COND_INITIALIZER;

pthread_mutex_t hfq_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t afq_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t nfq_lock = PTHREAD_MUTEX_INITIALIZER;

sem_t hzone_sem;
sem_t azone_sem;
sem_t nzone_sem;

//////////////
void init();
void destroy();
void *fan_func(void *args);
void *goal_scored(void *args);

void print_E1( char *name );
void print_E2( char *name, char zone );
void print_E3( char *name );
void print_E4( char *name );
void print_E5( char *name );
void print_E6( char *name );
void print_E7( char name, int num );
void print_E8( char name, int num );
void print_E9( int grp_no );
