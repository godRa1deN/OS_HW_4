#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h> // Добавлен заголовочный файл unistd.h

#define MAX_BUFFER_SIZE 1024

void send_encoded_array(int client_socket, struct sockaddr_in* server_address, int* array, size_t array_length) {
    // Отправка массива на сервер
    ssize_t sent_bytes = sendto(client_socket, (char*)array, array_length * sizeof(int), 0, (struct sockaddr*)server_address, sizeof(*server_address));
    if (sent_bytes == -1) {
        perror("Ошибка при отправке данных");
        close(client_socket);
        exit(1);
    }

    printf("Отправлено %ld байт данных на сервер\n", sent_bytes);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./client <server_ip> <server_port>\n");
        exit(1);
    }

    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);

    struct sockaddr_in server_address;
    int client_socket, recv_len;
    char buffer[MAX_BUFFER_SIZE];

    // Создание UDP сокета
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1) {
        perror("Ошибка создания сокета");
        exit(1);
    }

    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(server_ip);
    server_address.sin_port = htons(server_port);

    // Отправка сообщения серверу
    const char* message = "Клиент подключен";
    if (sendto(client_socket, message, strlen(message), 0, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Ошибка отправки данных");
        close(client_socket);
        exit(1);
    }

    // Получение ответа от сервера
    recv_len = recvfrom(client_socket, buffer, MAX_BUFFER_SIZE, 0, NULL, NULL);
    if (recv_len == -1) {
        perror("Ошибка приема данных");
        close(client_socket);
        exit(1);
    }

    buffer[recv_len] = '\0';
    printf("Получено от сервера: %s\n", buffer);
    

    size_t i, length = strlen(buffer);

    // Кодирование символов
    int int_array[length];
    for (i = 0; i < length; i++) {
        int_array[i] = (int)buffer[i];
        printf("%d ", int_array[i]);
    }
    char client_encoded_state[1500];
    sprintf(client_encoded_state, "Клиент успешно закодировал символы %s\n", buffer);
    if (sendto(client_socket, client_encoded_state, strlen(client_encoded_state), 0, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Ошибка отправки данных");
        close(client_socket);
        exit(1);
    }

    send_encoded_array(client_socket, &server_address, int_array, length);

    close(client_socket);
    return 0;
}