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

        if (sendto(client_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
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
    int server_addr_len = sizeof(server_addr);

    while (1)
    {
        int recv_len = recvfrom(client_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &server_addr_len);
        if (recv_len == SOCKET_ERROR)
        {
            printf("Recvfrom failed. Error Code: %d\n", WSAGetLastError());
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
    char server_ip[INET_ADDRSTRLEN];
    int server_port;
    HANDLE send_thread, recv_thread;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == INVALID_SOCKET)
    {
        printf("Could not create socket. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // printf("서버 IP 주소를 입력해주세요 (ex. 127.0.0.1): ");
    // fgets(server_ip, INET_ADDRSTRLEN, stdin);
    // server_ip[strcspn(server_ip, "\n")] = '\0';

    // printf("서버 포트 번호를 입력해주세요: ");
    // scanf("%d", &server_port);
    // getchar();

    server_addr.sin_family = AF_INET;
    // server_addr.sin_port = htons(server_port);
    server_addr.sin_port = htons(1010);
    // int z = inet_pton(AF_INET, server_ip, &server_addr.sin_addr.s_addr);
    int z = inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr.s_addr);

    if (z <= 0)
    {
        printf("Failed to convert server IP address. Error Code: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    printf("사용할 닉네임을 입력하세요: ");
    fgets(nickname, BUFFER_SIZE, stdin);
    nickname[strcspn(nickname, "\n")] = '\0';

    char init_message[BUFFER_SIZE];
    snprintf(init_message, BUFFER_SIZE, "[%s] is now connected.\n", nickname);
    if (sendto(client_socket, init_message, strlen(init_message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf("Initial message send failed. Error Code: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    send_thread = CreateThread(NULL, 0, send_message, NULL, 0, NULL);
    if (send_thread == NULL)
    {
        printf("Failed to create send thread. Error Code: %d\n", GetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    recv_thread = CreateThread(NULL, 0, receive_message, NULL, 0, NULL);
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