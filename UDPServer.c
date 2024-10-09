#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

int totalBytesReceived = 0, totalMessageRecieved = 0;

void handleStatRequest(SOCKET server_socket, struct sockaddr_in client_addr, int addr_len, char *clientRequest, char *response);

void handleStatRequest(SOCKET server_socket, struct sockaddr_in client_addr, int addr_len, char *clientRequest, char *response)
{
    if (strcmp(clientRequest, "bytes") == 0)
    {
        sprintf(response, "Total bytes received: %d", totalBytesReceived);
    }
    else if (strcmp(clientRequest, "number") == 0)
    {
        sprintf(response, "Total message recieved: %d", totalMessageRecieved);
    }
    else
    {
        sprintf(response, "Total messages received: %d, Total bytes received: %d", totalMessageRecieved, totalBytesReceived);
    }

    sendto(server_socket, response, strlen(response), 0, (struct sockaddr *)&client_addr, addr_len);
}

int main()
{
    WSADATA wsa;
    SOCKET server_socket;
    struct sockaddr_in server_addr, client_addr;
    char server_ip[INET_ADDRSTRLEN];
    int server_port;
    char client_ip[INET_ADDRSTRLEN];
    int client_addr_size = sizeof(client_addr);
    int recvMsgSize;
    char buffer[1024];

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == INVALID_SOCKET)
    {
        printf("Could not create socket. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    printf("서버 포트 번호를 입력하세요: ");
    scanf("%d", &server_port);
    getchar();

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(server_port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("Server listening on port %d...\n", server_port);

    while (1)
    {
        recvMsgSize = recvfrom(server_socket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_addr, &client_addr_size);

        inet_ntop(AF_INET, &server_addr.sin_addr, server_ip, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

        if (recvMsgSize == SOCKET_ERROR)
        {
            printf("Recvfrom failed. Error Code: %d\n", WSAGetLastError());
            closesocket(server_socket);
            WSACleanup();
            return 1;
        }
        else
        {
            totalMessageRecieved += 1;
            totalBytesReceived += recvMsgSize;

            buffer[recvMsgSize] = '\0';
            char msgcode[2];
            int sendMsgSize = 0;

            memcpy(msgcode, buffer, 2);

            unsigned short combinedValue = (msgcode[0] << 8) | msgcode[1];

            switch (combinedValue)
            {
            case 0x0001: // echo
                printf("%s:%d가 %s:%d로부터 %d 바이트 메시지 수신: %s\n", server_ip, ntohs(server_addr.sin_port), client_ip, ntohs(client_addr.sin_port), recvMsgSize, buffer + 2);
                sendMsgSize = sendto(server_socket, buffer + 2, strlen(buffer + 2), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                if (sendMsgSize == SOCKET_ERROR)
                {
                    printf("Sendto failed. Error Code: %d\n", WSAGetLastError());
                    closesocket(server_socket);
                    WSACleanup();
                    return 1;
                }

                break;
            case 0x0002: // chat
                const char *message = "Hello from the server!";
                printf("%s:%d가 %s:%d로부터 %d 바이트 메시지 수신: %s\n", server_ip, ntohs(server_addr.sin_port), client_ip, ntohs(client_addr.sin_port), recvMsgSize, message);
                sendMsgSize = sendto(server_socket, message, strlen(message), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                if (sendMsgSize == SOCKET_ERROR)
                {
                    printf("Sendto failed. Error Code: %d\n", WSAGetLastError());
                    closesocket(server_socket);
                    WSACleanup();
                    return 1;
                }
                break;
            case 0x0003: // stat
                char *clientRequest = buffer + 2;
                char response[1024];
                handleStatRequest(server_socket, client_addr, client_addr_size, clientRequest, response);
                printf("%s:%d가 %s:%d로부터 %d 바이트 메시지 수신: %s\n", server_ip, ntohs(server_addr.sin_port), client_ip, ntohs(client_addr.sin_port), recvMsgSize, response);
                break;
            case 0x0004: // quit
                printf("Quit message received.\n");
                closesocket(server_socket);
                WSACleanup();
                return 1;
            default:
                printf("Unknown request received.\n");
                break;
            }
        }
    }

    closesocket(server_socket);
    WSACleanup();

    return 0;
}