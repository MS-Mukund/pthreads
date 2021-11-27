#include "q3_client.h"
///////////////////////////////////////
pair<string, int> read_string_from_socket(int fd, int bytes)
{
    std::string output;
    output.resize(bytes);

    int bytes_received = read(fd, &output[0], bytes - 1);
    // debug(bytes_received);
    if (bytes_received <= 0)
    {
        cerr << "Failed to read data from socket. Seems server has closed socket\n";
        // return "
        exit(-1);
    }

    // debug(output);
    output[bytes_received] = 0;
    output.resize(bytes_received);

    return {output, bytes_received};
}

int send_string_on_socket(int fd, string &s)
{
    // cout << "We are sending " << s << endl;
    int bytes_sent = write(fd, s.c_str(), s.length());
    // debug(bytes_sent);
    if (bytes_sent < 0)
    {
        cerr << "Failed to SEND DATA on socket.\n";
        // return "
        exit(-1);
    }

    return bytes_sent;
}

int get_socket_fd(struct sockaddr_in *ptr)
{
    struct sockaddr_in server_obj = *ptr;

    // socket() creates an endpoint for communication and returns a file
    //        descriptor that refers to that endpoint.  The file descriptor
    //        returned by a successful call will be the lowest-numbered file
    //        descriptor not currently open for the process.
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        perror("Error in socket creation for CLIENT");
        exit(-1);
    }
    /////////////////////////////////////////////////////////////////////////////////////
    int port_num = SERVER_PORT;

    memset(&server_obj, 0, sizeof(server_obj)); // Zero out structure
    server_obj.sin_family = AF_INET;
    server_obj.sin_port = htons(port_num); //convert to big-endian order

    // Converts an IP address in numbers-and-dots notation into either a
    // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
    //https://stackoverflow.com/a/20778887/6427607

    /////////////////////////////////////////////////////////////////////////////////////////
    /* connect to server */

    if (connect(socket_fd, (struct sockaddr *)&server_obj, sizeof(server_obj)) < 0)
    {
        perror("Problem in connecting to the server");
        exit(-1);
    }

    //part;
    // printf(BGRN "Connected to server\n" ANSI_RESET);
    // part;
    return socket_fd;
}
////////////////////////////////////////////////////////

void begin_process(string &strs)
{
    struct sockaddr_in server_obj;
    int socket_fd = get_socket_fd(&server_obj);

    // pthread_mutex_lock(&print_lock);
    // cout << "Connection to server successful" << endl;
    // pthread_mutex_unlock(&print_lock);

    send_string_on_socket(socket_fd, strs);
    // cout << "here\n";
    int num_bytes_read;
    string output_msg;
    tie(output_msg, num_bytes_read) = read_string_from_socket(socket_fd, buff_sz);
    istringstream iss(output_msg);
    string ack;
    iss >> ack;
    if( ack != "Ack:")
    {
        cout << "error: server has not acknowledged msg" << endl;
        return;
    }

    pthread_mutex_lock(&print_lock);
    while( iss >> ack )
        cout << ack << " ";
    cout << endl;
    pthread_mutex_unlock(&print_lock);
}   

void *thread_func(void *args)
{
    c_info *cli = (c_info *)args;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += cli->time;

    sem_timedwait(&imaginary_sem, &ts);     //acts as a timer

    // cout << "then " << cli->msg << endl;
    begin_process(cli->msg);

    return NULL;
}

int main(int argc, char *argv[])
{
    int m;
    cin >> m;

    client_list = new c_info[m];
    sem_init(&imaginary_sem, 0, 0);

    for (int i = 0; i < m; i++)
    {
        int time;
        string cmd, key;
        cin >> time >> cmd >> key;

        string key2;
        if( cmd != "delete" && cmd != "fetch" )
        {
            cin >> key2;
            client_list[i].msg = to_string(i) + " " + cmd + " " + key + " " + key2;
        }

        else
        {
            client_list[i].msg = to_string(i) + " " + cmd + " " + key;
        }
        client_list[i].time = time;
    }

    for( int i = 0; i < m; i++)
    {
        pthread_create(&client_list[i].thread_id, NULL, thread_func, (void *)&client_list[i]);
    }
    
    for( int i = 0; i < m; i++)
    {
        pthread_join(client_list[i].thread_id, NULL);
    }
    // cout << "finished" << endl;
    
    return 0;
}