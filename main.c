#include <stdio.h>
#include <winsock2.h>
#include <pthread.h>

void *teste_thread(void *arg) {
    printf("Thread funcionando!\n");
    return NULL;
}

int main(void) {
    WSADATA wsa;
    pthread_t t;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) == 0)
        printf("Winsock funcionando!\n");

    pthread_create(&t, NULL, teste_thread, NULL);
    pthread_join(t, NULL);

    WSACleanup();
    system("pause");
    return 0;
}
