#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib") // Winsock 라이브러리 연결

int main()
{
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server_addr;
    char buffer[1024];
    const char *message = "Hello from the client!";

    // Winsock 초기화
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // 소켓 생성
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET)
    {
        printf("Could not create socket. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    int z = inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr.s_addr); // 서버의 IP 주소
    server_addr.sin_port = htons(8080);

    if (z == 0)
    {
        printf("Failed to convert server IP address. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // 서버에 연결
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf("Connection failed. Error Code: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }
    printf("Connected to server.\n");

    // 서버로 메시지 전송
    send(client_socket, message, strlen(message), 0);

    // 서버로부터 응답 수신
    int recv_size = recv(client_socket, buffer, sizeof(buffer), 0);
    if (recv_size == SOCKET_ERROR)
    {
        printf("Recv failed. Error Code: %d\n", WSAGetLastError());
    }
    else
    {
        buffer[recv_size] = '\0'; // 수신된 데이터는 문자열로 처리
        printf("Received message: %s\n", buffer);
    }

    // 소켓 닫기
    closesocket(client_socket);
    WSACleanup();

    return 0;
}
