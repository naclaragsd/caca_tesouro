/*
 * Caca ao Tesouro 1x1 - Etapa 1: conexao entre os dois jogadores
 * Baseado no server.c / client.c dos slides do professor
 * (Cap 02 - Parte 2 - Comunicacao com Sockets em C)
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>

#define PORTA   51171   /* mesma porta usada nos exemplos do professor */
#define TAM_MSG 64      /* toda mensagem do jogo tem 64 bytes */

/* O recv pode entregar a mensagem em pedacos.
   Esta funcao chama recv ate juntar exatamente 'tam' bytes.
   Retorna tam se deu certo, 0 se o outro desconectou, SOCKET_ERROR se deu erro. */
int recebe_tudo(SOCKET s, char *buffer, int tam) {
    int total = 0;
    while (total < tam) {
        int n = recv(s, buffer + total, tam - total, 0);
        if (n == 0 || n == SOCKET_ERROR) return n;
        total += n;
    }
    return total;
}

int main() {
    WSADATA winsocketsDados;
    SOCKET conexao;            /* socket usado para conversar com o outro jogador */
    char mensagem[TAM_MSG];
    int opcao;
    int temp;

    /* ===== Inicializa biblioteca (igual nos dois) ===== */
    temp = WSAStartup(MAKEWORD(2, 2), &winsocketsDados);
    if (temp != 0) {
        printf("WSAStartup falhou: %d\n", temp);
        return 1;
    } else {
        printf("WSAStartup carregado com sucesso\n");
    }

    printf("\n1 - Criar partida (servidor)\n");
    printf("2 - Entrar em partida (cliente)\n> ");
    scanf("%d", &opcao);

    if (opcao == 1) {
        /* ================= SERVIDOR (server.c) ================= */

        /* Cria socket */
        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            printf("Erro ao criar o socket: %d\n", WSAGetLastError());
            WSACleanup();
            return 1;
        } else {
            printf("Socket criado com sucesso\n");
        }

        /* Associa o socket a este PC, na PORTA */
        struct sockaddr_in server;
        memset(&server, 0, sizeof(server));
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;   /* qualquer IP deste computador */
        server.sin_port = htons(PORTA);

        if (bind(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
            printf("Erro ao associar o socket: %d\n", WSAGetLastError());
            closesocket(sock);
            WSACleanup();
            return 1;
        } else {
            printf("Bind realizado com sucesso\n");
        }

        /* Modo escuta */
        if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
            printf("Erro ao colocar o socket em estado de escuta: %d\n", WSAGetLastError());
            closesocket(sock);
            WSACleanup();
            return 1;
        } else {
            printf("Listen realizado com sucesso\n");
        }

        printf("Esperando o outro jogador na porta %d...\n", PORTA);

        /* Aceita conexao: o programa fica parado aqui ate o cliente conectar */
        SOCKET clientSocket;
        struct sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        clientSocket = accept(sock, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            printf("Erro ao aceitar a conexao: %d\n", WSAGetLastError());
            closesocket(sock);
            WSACleanup();
            return 1;
        } else {
            printf("Conexao aceita com sucesso\n");
        }

        closesocket(sock);         /* jogo 1x1: nao precisa esperar mais ninguem */
        conexao = clientSocket;
    } else {
        /* ================= CLIENTE (client.c) ================= */

        /* Cria socket */
        SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            printf("Erro ao criar o socket: %d\n", WSAGetLastError());
            WSACleanup();
            return 1;
        } else {
            printf("Socket criado com sucesso\n");
        }

        /* No exemplo do professor o IP era fixo (127.0.0.1).
           Aqui o jogador digita, para funcionar tambem entre dois computadores. */
        char ip[32];
        printf("IP do servidor (127.0.0.1 se for no mesmo PC): ");
        scanf("%31s", ip);

        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr(ip);
        serverAddr.sin_port = htons(PORTA);

        /* Conecta ao servidor */
        if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            printf("Erro ao conectar ao servidor: %d\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        } else {
            printf("Conectado ao servidor\n");
        }

        conexao = clientSocket;
    }

    /* ===== Troca de mensagens (igual nos dois) ===== */
    memset(mensagem, 0, TAM_MSG);
    if (opcao == 1) strcpy(mensagem, "OLA do servidor");
    else            strcpy(mensagem, "OLA do cliente");

    if (send(conexao, mensagem, TAM_MSG, 0) == SOCKET_ERROR) {
        printf("Erro ao enviar dados: %d\n", WSAGetLastError());
    } else {
        printf("-> enviado:  %s\n", mensagem);
    }

    if (recebe_tudo(conexao, mensagem, TAM_MSG) == TAM_MSG) {
        mensagem[TAM_MSG - 1] = '\0';     /* garante o fim da string */
        printf("<- recebido: %s\n", mensagem);
    } else {
        printf("O outro jogador desconectou.\n");
    }

    /* ===== Finaliza socket e biblioteca ===== */
    closesocket(conexao);
    WSACleanup();
    system("pause");
    return 0;
}
