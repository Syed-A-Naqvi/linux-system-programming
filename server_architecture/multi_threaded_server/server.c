#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<pthread.h>

// THREAD FUNCTION
void* manageClient(void* arg)
{
    // getting server-side client socket
    int clientSocket = *(int *)arg;

    // communicating with client
    char RxBuffer[1024] = {};
    char TxBuffer[1024] = {};

    while(1)
    {
        // recieving messages from client
        int rcvd = recv(clientSocket, RxBuffer, sizeof(RxBuffer),0);
        if (rcvd == -1)
        {
            printf("Error recieving message from client.\n");
        }
        else if (rcvd == 0)
        {
            printf("\nClient has been disconnected.\n\n");
            break;
        }
        else
        {
            RxBuffer[strcspn(RxBuffer, "\n")] = '\0';
            printf("Message recieved from client -> \"%s\"\n", RxBuffer);
        }

        // creating acknowledgment message
        sprintf(TxBuffer, "Acknowledging message: \"%s\"\n", RxBuffer);

        // sending message acknowledgement
        int sent = send(clientSocket, TxBuffer, strlen(TxBuffer)+1,0);
        if (sent == -1)
        {
            printf("Error sending message acknowledgement.\n");
        }
        else
        {
            printf("Acknowledgement sent.\n");
        }

        memset(RxBuffer, '\0', sizeof(RxBuffer));
        memset(TxBuffer, '\0', sizeof(TxBuffer));
        
    }

    close(clientSocket);
    pthread_exit(NULL);

}

int main(int argc, char const *argv[])
{
    // CREATING LISTENING SOCKET //
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket != -1)
    {
        printf("Socket created successfully.\n");
    }
    else
    {
        printf("Could not create socket. Terminating...\n");
        return -1;
    }

    // CREATING SOCKET ADDRESS STRUCTURE //
    struct sockaddr_in svrAddress;
    svrAddress.sin_family = AF_INET;
    svrAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    svrAddress.sin_port = htons(50000);
    socklen_t lenAddress = sizeof(svrAddress);

    // BINDING ADDRESS STRUCTURE TO SOCKET //
    int bindResult = bind(serverSocket, (struct sockaddr *)&svrAddress, lenAddress);
    if (bindResult == 0)
    {
        printf("Socket bound successfully to address structure.\n");
    }
    else
    {
        printf("Unable to bind socket to address structure. Terminating...\n");        
        return -1;
    }

    // LISTENING ON SOCKET FOR CONNECTION REQUESTS //
    int listenResult = listen(serverSocket, 2);
    if (listenResult == 0)
    {
        printf("Successfully listening on socket...\n");
    }
    else
    {
        printf("Unable to listen on socket. Terminating...\n");        
        return -1;
    }

    while (1)
    {

        // ACCEPTING INCOMING CONNECTION, CREATING NEW SOCKET AND STORING CLIENT-SIDE SOCKET ADDRESS //
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLen = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
        if (clientSocket == -1)
        {
            printf("Unable to accept connection.\n");        
        }
        else
        {
            printf("\nConnection accepted!\n");
            printf("Client IP: %s\n", inet_ntoa(clientAddress.sin_addr));        
            printf("Client Port Number: %d\n\n", (clientAddress.sin_port));
        }

        pthread_t tid;

        // creating thread to service client connection
        if(pthread_create(&tid, NULL, manageClient, (void *)&clientSocket) != 0)
        {
            printf("Error creating thread for client.\n");
        }

    }
        
    close(serverSocket);

    return 0;
}
