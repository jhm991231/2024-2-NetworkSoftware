#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

int sendMsg(SOCKET client_socket, char *message, struct sockaddr_in server_addr, int num);

int main()
{
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server_addr, local_addr;
    char server_ip[INET_ADDRSTRLEN];
    int server_port;
    int server_addr_size = sizeof(server_addr);
    char buffer[1024];
    char *message = "Hello from the client";

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

    printf("서버 IP 주소를 입력하세요 (ex. 127.0.0.1): ");
    fgets(server_ip, INET_ADDRSTRLEN, stdin);
    server_ip[strcspn(server_ip, "\n")] = '\0';

    printf("서버 포트 번호를 입력하세요: ");
    scanf("%d", &server_port);
    getchar();

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    int z = inet_pton(AF_INET, server_ip, &server_addr.sin_addr.s_addr);

    if (z <= 0)
    {
        printf("Failed to convert server IP address. Error Code: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    int input;
    do
    {
        printf("1. echo\n");
        printf("2. chat\n");
        printf("3. stat\n");
        printf("4. quit\n");

        scanf("%d", &input);
        getchar();

        if (input < 1 || input > 4)
        {
            printf("Invalid option. Please enter a valid option (1, 2, 3, 4).\n");
        }

        int sendMsgSize = sendMsg(client_socket, message, server_addr, input);

        if (sendMsgSize == SOCKET_ERROR)
        {
            printf("Sendto failed. Error Code: %d\n", WSAGetLastError());
            closesocket(client_socket);
            WSACleanup();
            return 1;
        }
        else
        {
            int recvMsgSize = recvfrom(client_socket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&server_addr, &server_addr_size);
            if (recvMsgSize == SOCKET_ERROR)
            {
                printf("Recvfrom failed. Error Code: %d\n", WSAGetLastError());
                closesocket(client_socket);
                WSACleanup();
                return 1;
            }
            buffer[recvMsgSize] = '\0';
            printf("Message Received: %s\n", buffer);
        }
    } while (1);

    closesocket(client_socket);
    WSACleanup();

    return 0;
}

int sendMsg(SOCKET client_socket, char *message, struct sockaddr_in server_addr, int num)
{
    char newMessage[1024];
    char userInput[100];
    int option;
    int sendMsgSize;

    switch (num)
    {
    case 1: // echo
        newMessage[0] = 0x0;
        newMessage[1] = 0x1;
        strcpy(newMessage + 2, message);

        sendMsgSize = sendto(client_socket, newMessage, 2 + strlen(newMessage + 2), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        return sendMsgSize;
    case 2: // chat
        newMessage[0] = 0x0;
        newMessage[1] = 0x2;
        strcpy(newMessage + 2, message);

        sendMsgSize = sendto(client_socket, newMessage, 2 + strlen(newMessage + 2), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        return sendMsgSize;
    case 3: // stat
        newMessage[0] = 0x0;
        newMessage[1] = 0x3;

        do
        {
            printf("\n1.bytes\n");
            printf("2.number\n");
            printf("3.both\n");

            fgets(userInput, sizeof(userInput), stdin);
            option = atoi(userInput);

            if (option < 1 || option > 3)
            {
                printf("Invalid option. Please enter 1, 2, or 3\n");
            }
        } while (option < 1 || option > 3);

        switch (option)
        {
        case 1: // bytes 선택
            strcpy(newMessage + 2, "bytes");
            break;
        case 2: // number 선택
            strcpy(newMessage + 2, "number");
            break;
        defalut: // 둘 다 선택
            strcpy(newMessage + 2, "both");
            break;
        }
        sendMsgSize = sendto(client_socket, newMessage, 2 + strlen(newMessage + 2), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        return sendMsgSize;
    case 4: // quit
        newMessage[0] = 0x0;
        newMessage[1] = 0x4;

        sendMsgSize = sendto(client_socket, newMessage, 2, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        closesocket(client_socket);
        WSACleanup();
        exit(0);
    default:
        return SOCKET_ERROR;
    }
}