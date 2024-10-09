#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

typedef struct
{
    char nickname[20];
    SOCKET client_socket;
    int active;
} client_t;

SOCKET server_socket;
int server_port;
client_t clients[MAX_CLIENTS];
int client_count = 0;
int message_count = 0;
HANDLE mutex;

void print_client_info()
{
    WaitForSingleObject(mutex, INFINITE);
    printf("Client Information:\n");
    for (int i = 0; i < client_count; i++)
    {
        if (clients[i].active)
        {
            printf("Nickname: %s, Socket: %d\n", clients[i].nickname, clients[i].client_socket);
        }
    }
    ReleaseMutex(mutex);
}

void print_message_stats()
{
    printf("Total messages received: %d\n", message_count);
}

DWORD WINAPI menu_thread(LPVOID arg)
{
    int choice;
    while (1)
    {
        printf("\nMenu:\n");
        printf("1. Client Information\n");
        printf("2. Chat Statistics\n");
        printf("3. Exit\n");
        printf("Enter your choice:\n");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            print_client_info();
            break;

        case 2:
            print_message_stats();
            break;

        case 3:
            printf("Exiting server...\n");
            closesocket(server_socket);
            WSACleanup();
            ExitProcess(0);
            break;

        default:
            printf("Invalid choice. Please try again.\n");
        }
    }
}

void relay_message(int sender_index, char *message, int message_len)
{
    WaitForSingleObject(mutex, INFINITE);
    for (int i = 0; i < client_count; i++)
    {
        if (i != sender_index && clients[i].active)
        {
            send(clients[i].client_socket, message, message_len, 0);
        }
    }
    ReleaseMutex(mutex);
}

DWORD WINAPI client_handler_thread(LPVOID arg)
{
    int index = *(int *)arg;
    char buffer[BUFFER_SIZE];

    while (1)
    {
        int recv_len = recv(clients[index].client_socket, buffer, BUFFER_SIZE, 0);

        if (recv_len > 0)
        {
            buffer[recv_len] = '\0';
            printf("%s", buffer);
            relay_message(index, buffer, recv_len);
            InterlockedIncrement(&message_count);
        }
        else if (recv_len == 0)
        {
            printf("Client disconnected.\n");
            closesocket(clients[index].client_socket);
            clients[index].active = 0;
            break;
        }
        else
        {
            printf("Recv failed. Error Code: %d\n", WSAGetLastError());
            break;
        }
    }

    return 0;
}

int main()
{
    WSADATA wsa;
    struct sockaddr_in server_addr, client_addr;
    int client_addr_len = sizeof(client_addr);

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(7000);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket, 3) == SOCKET_ERROR)
    {
        printf("Listen failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("Server listening on port 7000...\n");

    mutex = CreateMutex(NULL, FALSE, NULL);
    if (mutex == NULL)
    {
        printf("CreateMutex failed, Error Code: %d\n", GetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    HANDLE menu_thread_handle = CreateThread(NULL, 0, menu_thread, NULL, 0, NULL);
    if (menu_thread_handle == NULL)
    {
        printf("Failed to create menu thread. Error Code: %d\n", GetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    while (1)
    {
        SOCKET client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket == INVALID_SOCKET)
        {
            printf("Accept failed. Error Code: %d\n", WSAGetLastError());
        }

        WaitForSingleObject(mutex, INFINITE);
        if (client_count < MAX_CLIENTS)
        {
            int index = client_count++;
            clients[index].client_socket = client_socket;
            clients[index].active = 1;
            strcpy(clients[index].nickname, "Annonymous");

            HANDLE client_thread_handle = CreateThread(NULL, 0, client_handler_thread, &index, 0, NULL);
            if (client_thread_handle == NULL)
            {
                printf("Failed to create client handler thread. Error Code: %d\n", GetLastError());
                closesocket(client_socket);
                clients[index].active = 0;
            }
        }
        else
        {
            printf("Max clients reached, closing connection.\n");
            closesocket(client_socket);
        }
        ReleaseMutex(mutex);
    }

    WaitForSingleObject(menu_thread_handle, INFINITE);

    CloseHandle(mutex);
    closesocket(server_socket);
    WSACleanup();

    return 0;
}