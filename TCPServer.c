#include <winsock2.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib") // Winsock 라이브러리 연결

int main()
{
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int client_addr_size = sizeof(client_addr);
    char buffer[1024];

    // Winsock 초기화
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // 소켓 생성
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET)
    {
        printf("Could not create socket. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(8080);

    // 소켓 바인딩
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // 연결 대기
    listen(server_socket, 3);
    printf("Waiting for incoming connections...\n");

    // 클라이언트 연결 수락
    client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
    if (client_socket == INVALID_SOCKET)
    {
        printf("Accept failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
    printf("Connection accepted.\n");

    // 클라이언트로부터 메시지 수신
    int recv_size = recv(client_socket, buffer, sizeof(buffer), 0);
    if (recv_size == SOCKET_ERROR)
    {
        printf("Recv failed. Error Code: %d\n", WSAGetLastError());
    }
    else
    {
        buffer[recv_size] = '\0'; // 수신된 데이터는 문자열로 처리
        printf("Received message: %s\n", buffer);

        // 클라이언트에게 응답 전송
        const char *message = "Hello from the server!";
        send(client_socket, message, strlen(message), 0);
    }

    // 소켓 닫기
    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();

    return 0;
}
