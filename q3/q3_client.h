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

/////////////////////////////
#include <pthread.h>
#include <iostream>
#include <semaphore.h>
#include <assert.h>
#include <queue>
#include <vector>
#include <tuple>
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
const LL MOD = 1000000007;
#define part cout << "-----------------------------------" << endl;
#define pb push_back
#define debug(x) cout << #x << " : " << x << endl

///////////////////////////////
#define SERVER_PORT 8001
////////////////////////////////////

const LL buff_sz = 1048576;
//////////////////////////////////////////////////
typedef struct client_info {
    pthread_t thread_id;
    string msg;
    int time;
} c_info;

c_info *client_list;    // points to list of all client threads
sem_t imaginary_sem;
pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;