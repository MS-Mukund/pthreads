#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <assert.h>

// Global variables
// keeps track of number of ticks passed since start of program
int ticks = 0;

// lock to print to console
pthread_mutex_t print_lock;

// Some colors to judge different outputs when running.
#define NRM "\x1B[1;0m"
#define RED "\x1B[1;31m"
#define GRN "\x1B[1;32m"
#define YEL "\x1B[1;33m"
#define BLU "\x1B[1;34m"
#define MAG "\x1B[1;35m"
#define CYN "\x1B[1;36m"
#define WHT "\x1B[1;37m"

// struct definitions
enum direction { east = 1, west = 0 };

typedef struct Baboon
{
    int id;                     // Unique id for each baboon
    enum direction dir;         // Direction from which baboon came
    pthread_t baboon_thread_id; // unique identifier of the thread of a single baboon
} Baboon;

typedef struct course {
    char *name;         // course name, assumed unique

    float interest;
    int max_slots;

    int num_labs;   // number of labs from which we can choose;
    int *labs;      // array of lab ids
    pthread_t course_thr_id;
} course;

enum stud_state { not_registered, waiting, attending, finished, exited };
typedef struct student {
    int id; 
    int time;    // time after which he fills preferences
    int pref[3];  // top 3 preferences
    enum stud_state state;  
    float calibre;      
    pthread_t stud_thread_id;
} stud;

int waiting[2] = {0, 0};        // Number of baboons waiting from each direction
int travelling[2] = {0, 0};     // Number of baboons crossing the rope (only one of the direction will have positive value at a time)
sem_t sem[2];
pthread_mutex_t mutex;          // can also be a binary semaphore. Used to lock critical sections

void init();
void destroy();
void *start_stud(void *args);
void *start_c(void *args);