#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024

SOCKET client_socket;
struct sockaddr_in server_addr;
char nickname[20];

DWORD WINAPI send_message(LPVOID arg)
{
    char message[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    while (1)
    {
        fgets(message, BUFFER_SIZE, stdin);
        snprintf(buffer, BUFFER_SIZE, "[%s] %s", nickname, message);

        if (send(client_socket, buffer, strlen(buffer), 0) == SOCKET_ERROR)
        {
            printf("Message send failed. Error Code: %d\n", WSAGetLastError());
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}

DWORD WINAPI receive_message(LPVOID arg)
{
    char buffer[BUFFER_SIZE];
    while (1)
    {
        int recv_len = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (recv_len == SOCKET_ERROR)
        {
            printf("recv failed. Error Code: %d\n", WSAGetLastError());
            closesocket(client_socket);
            WSACleanup();
            return 1;
        }
        else
        {
            buffer[recv_len] = '\0';
            printf("%s", buffer);
        }
    }
    return 0;
}

int main()
{
    WSADATA wsa;
    HANDLE send_thread, recv_thread;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET)
    {
        printf("Could not create socket. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(7000);
    int z = inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr.s_addr);

    if (z <= 0)
    {
        printf("Failed to convert server IP address. Error Code: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Connect failed. Error Code: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    printf("사용할 닉네임을 입력하세요: ");
    fgets(nickname, BUFFER_SIZE, stdin);
    nickname[strcspn(nickname, "\n")] = '\0';

    send_thread = CreateThread(NULL, 0, send_message, NULL, 0, NULL);
    if (send_thread == NULL)
    {
        printf("Failed to create send thread. Error Code: %d\n", GetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    recv_thread = CreateThread(NULL, 0, recv_thread, NULL, 0, NULL);
    if (recv_thread == NULL)
    {
        printf("Failed to create recv thread. Error Code: %d\n", GetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    WaitForSingleObject(send_thread, INFINITE);
    WaitForSingleObject(recv_thread, INFINITE);

    closesocket(client_socket);
    WSACleanup();

    return 0;
}