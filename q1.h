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

// Some colors to judge different outputs when running.
#define NORMAL "\x1B[1;0m"
#define RED "\x1B[1;31m"
#define GREEN "\x1B[1;32m"
#define YELLOW "\x1B[1;33m"
#define BLUE "\x1B[1;34m"
#define MAGENTA "\x1B[1;35m"
#define CYAN "\x1B[1;36m"
#define WHITE "\x1B[1;37m"

// Global variables

// lock to print to console
pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  scan_sig = PTHREAD_COND_INITIALIZER;
pthread_mutex_t scan_lock = PTHREAD_MUTEX_INITIALIZER;
int done = 0;

// semaphores
sem_t imaginary_lock;
sem_t *lab_locks;

// struct definitions
typedef struct course {
    char *name;         // course name, assumed unique

    float interest;
    int max_slots;
    bool removed;

    int num_labs;   // number of labs from which we can choose;
    int *labs;      // array of lab ids

    sem_t wait_for_seat;
    int ppl_there;
    pthread_mutex_t ppl_lock;
    pthread_cond_t  ppl_sig;

    sem_t wait_end_tute;
    pthread_t course_thr_id;

    pthread_cond_t attend;
    pthread_mutex_t attend_lock;
    int num_stud;
} course;

enum stud_state { not_registered, waiting, attending, finished };
typedef struct student {

    int id; 
    int time;    // time after which he fills preferences
    int pref[3];  // top 3 preferences
    enum stud_state state;  
    float calibre;      
    pthread_t stud_thread_id;

} stud;

typedef struct lab {
    char *name;
    int num_tas;
    int max_no_times;
    
    int *num_taed;      // no of times the TA took tutes
    bool *occupied;      // is the TA occupied? 
} lab;

lab *all_labs;                 // ptr array of all labs
course *all_courses;           // ptr array of all courses

void init();
void destroy();
void *start_stud(void *args);
void *start_c(void *args);

void print_E1( int id );
void print_E2( int id, char *name );
void print_E3( int id, char *name );
void print_E4( int id, int priority, char *pref1, char *pref2 );
void print_E5( int id, char *name );
void print_E6( int id );
void print_E7( char *name, int seats );
void print_E8( char *name, int filled, int seats);
void print_E9( int ta_id, char *l_name, char *name );
void print_E10( char *name );
void print_E11( int ta_id, char *l_name, char *name, int num );
void print_E12( char *l_name );