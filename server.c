#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <signal.h>
#define max_clients 1000

int main(int argc, char *argv[])
{
    char *end = NULL;
    
    if (argc == 1) exit(1);
    long my_port = strtol(argv[1],&end,10);
    
    if (end == argv[1] || *end != '\0') exit(1);
    else if (errno == ERANGE || my_port > 65535 || my_port < 1 ) exit(1);

    struct sigaction myaction;
    myaction.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &myaction, NULL);


    int clients[max_clients];
    int select_max;
    for (int i = 0; i < max_clients; i++)   
    {   
        clients[i] = 0;   
    } 

    struct sockaddr_in server;

    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1) {
        perror("server: socket");
        exit(1);
    }

    memset(&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(my_port);  
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(lfd, (struct sockaddr *) &server, sizeof(struct sockaddr_in)) == -1) {
    perror("server: bind");
    close(lfd);
    exit(1);

    }
    if (listen(lfd, 5) < 0) {  //5 is number of partial connections listen can hold on to
    perror("listen");
    exit(1);
    }

    char post[75] = "\n";
    fd_set readfds;

    for (;;){

        FD_ZERO(&readfds);
        FD_SET(lfd, &readfds);
        select_max = lfd;

        for (size_t i = 0; i < max_clients; i++)
        {
            if (clients[i]>0)
            {
                FD_SET(clients[i], &readfds);
                if(clients[i] > select_max) select_max = clients[i];
            }
            
        }
        
        if(select(select_max+1, &readfds, NULL, NULL, NULL) == -1){
            perror("select");
            //exit(1);
            continue;
        }; //CHEDK ERRORS



        if (FD_ISSET(lfd, &readfds))
        {
            struct sockaddr_in client;
            memset(&client, 0, sizeof(client));
            socklen_t client_len = sizeof(client);
            client.sin_family = AF_INET;
            int cfd = accept(lfd, (struct sockaddr *)&client, &client_len); //CHECK ERRORS  
            if(cfd == -1){
                perror("accept");
                //exit(1);
                continue; // I THINK
            }
            
            for (size_t i = 0; i < max_clients; i++)
            {
                if (clients[i]==0) 
                {
                    printf("client connected\n");                           //TESTING
                    clients[i] = cfd;
                    break;
                }
               
            }
        }

        char reading[500];
        int readnum;
        int setclient=0;
        for (size_t i = 0; i < max_clients; i++)
        {
            if (FD_ISSET(clients[i], &readfds))
            {
                setclient=clients[i];
                readnum = read(clients[i],&reading,76);
                if (readnum == -1)
                {
                    perror ("read");
                    //exit (1);
                    close(clients[i]);
                    clients[i] = 0;
                }
                else if (readnum == 0)
                {
                    close(clients[i]);
                    clients[i] = 0;
                    //This means that the file discriptor is probably clsoed and I should clsoe it asd well
                    //check this by useing the sigpipe check
                }
                else if (readnum > 0)
                {
                    reading[readnum] = '\0';
                    printf("I have read %s from client %d \n", reading, setclient);          //TESTS

                    for (size_t i = 0; i < max_clients; i++)
                    {
                        if (clients[i]!=0 && clients[i]!=setclient) 
                        {
                            printf("I should write to client %d\n", clients[i]);
                            write(clients[i],&reading,readnum);
                        }
                    
                    }
                }
                
            }
        }
    }      
}