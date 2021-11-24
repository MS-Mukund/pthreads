#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>

// threads
#include <pthread.h>
#include <semaphore.h>

/////////////////////////////
#include <iostream>
#include <assert.h>
#include <tuple>

#include <vector>
#include <queue>  // std::queue
#include <sstream>
using namespace std;
/////////////////////////////

//Regular bold text
#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define ANSI_RESET "\x1b[0m"

typedef long long LL;

#define pb push_back
#define debug(x) cout << #x << " : " << x << endl
#define part cout << "-----------------------------------" << endl;

///////////////////////////////
#define MAX_CLIENTS 44
#define PORT_ARG 8001
#define MAX_SZ 100

const int initial_msg_len = 256;
const LL buff_sz = 1048576;
////////////////////////////////////
string dictionary[ 2*MAX_SZ+5 ] = {""}; 
pthread_mutex_t dict_lock[ 2*MAX_SZ+5 ] = {PTHREAD_MUTEX_INITIALIZER};

typedef struct thread_info {
    pthread_t thread_id;
    int id;
}threads;

void *waiting(void *arg);

queue<int *> client_q;
pthread_mutex_t lock_q = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t signal_q = PTHREAD_COND_INITIALIZER;

pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;
