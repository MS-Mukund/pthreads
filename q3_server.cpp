#include "q3_server.h"
///////////////////////////////////////////////////
pair<string, int> read_string_from_socket(const int &fd, int bytes)
{
    std::string output;
    output.resize(bytes);

    int bytes_received = read(fd, &output[0], bytes - 1);
    debug(bytes_received);
    if (bytes_received <= 0)
    {
        cerr << "Failed to read data from socket. \n";
    }

    output[bytes_received] = 0;
    output.resize(bytes_received);
    // debug(output);
    return {output, bytes_received};
}

int send_string_on_socket(int fd, const string &s)
{
    // debug(s.length());
    int bytes_sent = write(fd, s.c_str(), s.length());
    if (bytes_sent < 0)
    {
        cerr << "Failed to SEND DATA via socket.\n";
    }

    return bytes_sent;
}

///////////////////////////////
void handle_connection(int *p_client_socket_fd, pthread_t worker_thr_id)
{
    // int client_socket_fd = *((int *)client_socket_fd_ptr);
    //####################################################
    int client_socket_fd = *p_client_socket_fd;
    int received_num, sent_num;

    /* read message from client */
    int ret_val = 1;

    while (true)
    {
        string cmd;
        tie(cmd, received_num) = read_string_from_socket(client_socket_fd, buff_sz);
        ret_val = received_num;
        // debug(ret_val);
        // printf("Read something\n");
        if (ret_val <= 0)
        {
            // perror("Error read()");
            printf("Server could not read msg sent from client\n");
            goto close_client_socket_ceremony;
        }
        cout << "Client sent : " << cmd << endl;
        if (cmd == "exit")
        {
            cout << "Exit pressed by client" << endl;
            goto close_client_socket_ceremony;
        }
        string msg_to_send_back = "Ack: " + cmd;

        ////////////////////////////////////////
        // "If the server write a message on the socket and then close it before the client's read. Will the client be able to read the message?"
        // Yes. The client will get the data that was sent before the FIN packet that closes the socket.

        int sent_to_client = send_string_on_socket(client_socket_fd, msg_to_send_back);
        // debug(sent_to_client);
        if (sent_to_client == -1)
        {
            perror("Error while writing to client. Seems socket has been closed");
            goto close_client_socket_ceremony;
        }

        istringstream args(cmd);
        string key_str;
        args >> key_str;

        int index = stoi(key_str);

        string cmd_name;
        string value;
        args >> cmd_name;
        args >> key_str;
        int key = stoi(key_str);
        int stat = 0;
        if( key < 0 || key > 100 )
        {
            cout << "Invalid key" << endl;
            goto close_client_socket_ceremony;
        }

        if( cmd_name != "delete" || cmd_name != "fetch" )
            args >> value;

        if( cmd_name == "insert" )
        {
            stat = 0;
            pthread_mutex_lock(&dict_lock[key-1]);
            if( dictionary[key-1].empty() )
            {
                dictionary[key-1] = value;
                stat = 1;
            }
            pthread_mutex_unlock(&dict_lock[key-1]);
            
            pthread_mutex_lock(&print_lock);
            if( stat )
                cout << index << ":" << worker_thr_id << ":" << "Insertion successful" << endl;
            else
                cout << "key already exists" << endl;
            pthread_mutex_unlock(&print_lock);
        }

        else if( cmd_name == "concat" )
        {
            stat = 0;
            int key2 = stoi(value);
            if( key2 < 0 || key2 > 100 )
            {
                cout << "Invalid key" << endl;
                goto close_client_socket_ceremony;
            }
            string s1, s2; 
    
            pthread_mutex_lock(&dict_lock[key-1]);   
            pthread_mutex_lock(&dict_lock[key2-1]);
            if( !dictionary[key-1].empty() && !dictionary[key2-1].empty() )
            {
                s1 = dictionary[key-1] + dictionary[key2-1];
                s2 = dictionary[key2-1] + dictionary[key-1];
                dictionary[key-1] = s1;
                dictionary[key2-1] = s2;
                stat = 1;
            }
            pthread_mutex_unlock(&dict_lock[key2-1]);
            pthread_mutex_unlock(&dict_lock[key-1]);
            
            pthread_mutex_lock(&print_lock);
            if( stat )
                cout << index << ":" << worker_thr_id << ":" << s2 << endl;
            else
                cout << "concat failed as at least one of the keys does not exist" << endl;
            pthread_mutex_unlock(&print_lock);
        }

        else if( cmd_name == "update" )
        { 
            stat = 0;          
            pthread_mutex_lock(&dict_lock[key-1]);
            if( !dictionary[key-1].empty() )
            {
                dictionary[key-1] = value;
                stat = 1;
            }
            pthread_mutex_unlock(&dict_lock[key-1]);
            
            pthread_mutex_lock(&print_lock);
            if( stat )
                cout << index << ":" << worker_thr_id << ":" << value << endl;
            else
                cout << "key already exists" << endl;
            pthread_mutex_unlock(&print_lock);
        }

        if( cmd_name == "delete" )
        {
            stat = 0;
            pthread_mutex_lock(&dict_lock[key-1]);
            if( !dictionary[key-1].empty() )      // not empty
            {
                dictionary[key-1] = "";
                stat = 1;
            }
            pthread_mutex_unlock(&dict_lock[key-1]);
            
            pthread_mutex_lock(&print_lock);
            if( stat )
                cout << index << ":" << worker_thr_id << ":" << "Deletion successful" << endl;
            else
                cout << "No such key exists" << endl;
            pthread_mutex_unlock(&print_lock);
        }

        if( cmd_name == "fetch" )
        {
            string val;
            stat = 0;
            pthread_mutex_lock(&dict_lock[key-1]);
            if( !dictionary[key-1].empty() )      // not empty
            {
                val = dictionary[key-1];
                stat = 1;
            }
            pthread_mutex_unlock(&dict_lock[key-1]);
            
            pthread_mutex_lock(&print_lock);
            if( stat )
                cout << index << ":" << worker_thr_id << ":" << val << endl;
            else
                cout << "Key does not exist" << endl;
            pthread_mutex_unlock(&print_lock);
        }
    }

    close_client_socket_ceremony:
    close(client_socket_fd);
    printf(BRED "Disconnected from client" ANSI_RESET "\n");
}

void *waiting(void *arg)
{
    threads *t = (threads *)arg;
    while (true)
    {
        int *tmp = NULL;
        pthread_mutex_lock(&lock_q);
        if( client_q.empty() )
        {
            pthread_cond_wait(&signal_q, &lock_q);
        }
        tmp = client_q.front();
        client_q.pop();
        pthread_mutex_unlock(&lock_q);
        
        handle_connection(tmp, t->thread_id);
    }
}

int main(int argc, char *argv[])
{
    if( argc != 2 )
    {
        printf("Usage: %s <number of worker threads in the thread pool>\n", argv[0]);
        return 1;
    }
    int num_threads = atoi(argv[1]);
    if( num_threads <= 0 )
    {
        printf("Number of threads must be positive\n");
        return 2;
    }

    threads worker[num_threads];
    int cur_thread_id = 0;

    for( int i = 0; i < num_threads; i++ )
    {
        worker[i].id = cur_thread_id++;
        pthread_create(&(worker[i].thread_id), NULL, waiting, (void *)&worker[i]);
    }


    int wel_socket_fd, client_socket_fd, port_number;
    socklen_t clilen;

    struct sockaddr_in serv_addr_obj, client_addr_obj;
    /////////////////////////////////////////////////////////////////////////
    /* create socket */
    /*
    The server program must have a special door—more precisely,
    a special socket—that welcomes some initial contact 
    from a client process running on an arbitrary host
    */
    //get welcoming socket
    //get ip,port
    /////////////////////////
    wel_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (wel_socket_fd < 0)
    {
        perror("ERROR creating welcoming socket");
        exit(-1);
    }

    //////////////////////////////////////////////////////////////////////
    /* IP address can be anything (INADDR_ANY) */
    bzero((char *)&serv_addr_obj, sizeof(serv_addr_obj));
    port_number = PORT_ARG;
    serv_addr_obj.sin_family = AF_INET;
    // On the server side I understand that INADDR_ANY will bind the port to all available interfaces,
    serv_addr_obj.sin_addr.s_addr = INADDR_ANY;
    serv_addr_obj.sin_port = htons(port_number); //process specifies port

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* bind socket to this port number on this machine */
    /*When a socket is created with socket(2), it exists in a name space
       (address family) but has no address assigned to it.  bind() assigns
       the address specified by addr to the socket referred to by the file
       descriptor wel_sock_fd.  addrlen specifies the size, in bytes, of the
       address structure pointed to by addr.  */

    //CHECK WHY THE CASTING IS REQUIRED
    if (bind(wel_socket_fd, (struct sockaddr *)&serv_addr_obj, sizeof(serv_addr_obj)) < 0)
    {
        perror("Error on bind on welcome socket: ");
        exit(-1);
    }
    //////////////////////////////////////////////////////////////////////////////////////

    /* listen for incoming connection requests */

    listen(wel_socket_fd, MAX_CLIENTS);
    cout << "Server has started listening on the LISTEN PORT" << endl;
    clilen = sizeof(client_addr_obj);



    while (1)
    {
        /* accept a new request, create a client_socket_fd */
        /*
        During the three-way handshake, the client process knocks on the welcoming door
of the server process. When the server “hears” the knocking, it creates a new door—
more precisely, a new socket that is dedicated to that particular client. 
        */
        //accept is a blocking call
        printf("Waiting for a new client to request for a connection\n");
        client_socket_fd = accept(wel_socket_fd, (struct sockaddr *)&client_addr_obj, &clilen);
        if (client_socket_fd < 0)
        {
            perror("ERROR while accept() system call occurred in SERVER");
            exit(-1);
        }
        printf(BGRN "New client connected from port number %d and IP %s \n" ANSI_RESET, ntohs(client_addr_obj.sin_port), inet_ntoa(client_addr_obj.sin_addr));
        int *client_fd = new int;
        *client_fd = client_socket_fd;

        pthread_mutex_lock(&lock_q);
        client_q.push(client_fd);
        pthread_cond_signal(&signal_q);
        pthread_mutex_unlock(&lock_q);
    }

    close(wel_socket_fd);
    return 0;
}
