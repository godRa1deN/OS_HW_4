#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h> 

#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./client <server_ip> <server_port>\n");
        exit(1);
    }

    struct sockaddr_in server_address;
    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);

    struct sockaddr_in state_address;
    int state_socket, recv_len;
    char buffer[MAX_BUFFER_SIZE];

    state_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (state_socket == -1) {
        perror("Ошибка создания сокета");
        exit(1);
    }

    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(server_ip);
    server_address.sin_port = htons(server_port);

    // Отправка сообщения серверу
    const char* message = "State программа подключена, ожидание подключения клиентов\n";
    if (sendto(state_socket, message, strlen(message), 0, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Ошибка отправки данных");
        close(state_socket);
        exit(1);
    }

    printf("Сокет состояния запущен и ожидает подключений...\n");

    while (1) {
        // Ожидание состояния сервера от сервера
        memset(buffer, 0, MAX_BUFFER_SIZE);
        recv_len = recvfrom(state_socket, buffer, MAX_BUFFER_SIZE, 0, NULL, NULL);
        if (recv_len == -1) {
            perror("Ошибка приема состояния");
            close(state_socket);
            exit(1);
        }
        if (strcmp(buffer, "SERVER_EXIT") == 0) {
            printf("Сервер завершил работу. State программа завершается.\n");
            break;
        }
        buffer[recv_len] = '\0';
        printf("Получено состояние: %s\n", buffer);
    }

    close(state_socket);

    return 0;
}