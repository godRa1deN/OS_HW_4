#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h> // Добавлен заголовочный файл unistd.h

#define MAX_BUFFER_SIZE 1024
#define MAX_CLIENTS 10 

void receive_encoded_array(int server_socket, struct sockaddr_in* client_address, socklen_t* client_address_len, int* array, size_t array_length) {
    ssize_t recv_len = recvfrom(server_socket, array, MAX_BUFFER_SIZE, 0,
                                (struct sockaddr*)client_address, client_address_len);
    if (recv_len == -1) {
        perror("Ошибка приема данных");
        close(server_socket);
        exit(1);
    }

    for (size_t i = 0; i < array_length; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len;
    int server_socket, recv_len;
    char buffer[MAX_BUFFER_SIZE];

    if (argc != 3) {
        printf("Usage: ./server <server_port> <num_clients>\n");
        exit(1);
    }

    int server_port = atoi(argv[1]);
    int num_clients = atoi(argv[2]);

    if (num_clients <= 0 || num_clients > MAX_CLIENTS) {
        fprintf(stderr, "Количество клиентов должно быть положительным и не превышать %d\n", MAX_CLIENTS);
        exit(EXIT_FAILURE);
    }

    // Ввод строки от сервера
    printf("Введите строку: ");
    char input[1024];
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = '\0'; // Удаление символа новой строки
    size_t length = strlen(input);
    if (length < num_clients) {
        printf("Кол-во клиентов не может быть больше кол-ва символов\n");
        exit(EXIT_FAILURE);
    }


    // Создание UDP сокета
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == -1) {
        perror("Ошибка создания сокета");
        exit(1);
    }

    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(server_port);

    // Привязка сокета к адресу и порту сервера
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Ошибка привязки сокета");
        close(server_socket);
        exit(1);
    }

    printf("Сервер запущен и ожидает подключений...\n");

    int connected_clients = 0; // Счетчик подключенных клиентов

    int encoded_array[length]; // Итоговый массив

    while (connected_clients < num_clients) {
        memset(buffer, 0, MAX_BUFFER_SIZE);
        client_address_len = sizeof(client_address);

        // Получение данных и адреса клиента
        recv_len = recvfrom(server_socket, buffer, MAX_BUFFER_SIZE, 0,
                            (struct sockaddr*)&client_address, &client_address_len);
        if (recv_len == -1) {
            perror("Ошибка приема данных");
            close(server_socket);
            exit(1);
        }

        printf("Подключено клиентов: %d\n", connected_clients + 1);

        printf("Получено от клиента %s:%d: %s\n", 
               inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), buffer);

        // Формирование строки для клиента
        int j, k;
        char client_string[length / num_clients + 1];
        k = 0;

        for (j = connected_clients; j < length; j += num_clients) {
            client_string[k++] = input[j];
        }
        client_string[k] = '\0';

        // Отправка строки клиенту
        if (sendto(server_socket, client_string, strlen(buffer), 0,
                   (struct sockaddr*)&client_address, client_address_len) == -1) {
            perror("Ошибка отправки данных");
            close(server_socket);
            exit(1);
        }

        int received_array[k];
        receive_encoded_array(server_socket, &client_address, &client_address_len, received_array, k);

        int count = 0;
        for (int j = connected_clients; j < length; j += num_clients) {
            encoded_array[j] = received_array[count++];
        }
        connected_clients++;
    }

    printf("Закодированный массив: ");
    for (int i = 0; i < length; i++) {
        printf("%d ", encoded_array[i]);
    }

    close(server_socket);

    return 0;
}