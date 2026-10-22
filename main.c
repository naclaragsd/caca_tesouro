/*
 * Caca ao Tesouro 1x1 - Etapas 1, 2 e 3: conexao + mapa + partida pela rede
 * Baseado no server.c / client.c dos slides do professor
 * (Cap 02 - Parte 2 - Comunicacao com Sockets em C)
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <winsock2.h>
#include "mapa.h"

#define PORTA   51171   /* mesma porta usada nos exemplos do professor */
#define TAM_MSG 64      /* toda mensagem do jogo tem 64 bytes */

int recebe_tudo(SOCKET s, char *buffer, int tam);   /* esta mais abaixo no arquivo */

/* Limpa a tela e mostra os dois mapas, o placar, as ultimas mensagens
   trocadas pela rede e o aviso do que aconteceu na ultima jogada. */
void desenha_tela(char **meu_mapa, char **visto, int eu, int meus_achados, int achados_dele,
                  const char *enviado, const char *recebido, const char *aviso) {
    system("cls");
    printf("=========== CACA AO TESOURO 1x1 ===========\n");
    printf(" Voce e o jogador %d\n", eu);
    desenha_mapa(meu_mapa, "Seu mapa (o adversario procura aqui)");
    desenha_mapa(visto,    "Mapa do adversario (o que voce descobriu)");
    printf("\n Placar: voce %d x %d adversario  (de %d tesouros)\n",
           meus_achados, achados_dele, NUM_TESOUROS);
    printf(" -> ultima enviada:  %s\n", enviado);
    printf(" <- ultima recebida: %s\n", recebido);
    if (aviso[0] != '\0')
        printf("\n >> %s\n", aviso);
    printf("\n");
}

/* A partida pela rede.
   Notacao das mensagens (64 bytes, o 1o caractere e quem envia):
     "1C4"  -> jogador 1 escolheu a casa C4
     "2RT"  -> jogador 2 responde: tesouro
     "2RA3" -> jogador 2 responde: agua, tesouro mais perto a 3 casas
     "2RX"  -> jogador 2 responde: armadilha
   A variavel 'vez' existe nos dois processos. Os dois aplicam a mesma regra
   depois de cada jogada, entao ela fica igual nos dois sem precisar de
   mensagem extra: so as mensagens de jogada/resposta controlam o jogo. */
void joga_partida(SOCKET conexao, int eu) {
    int outro = (eu == 1) ? 2 : 1;
    int vez = 1;                        /* o jogador 1 (servidor) comeca */
    int pula_vez[3] = {0, 0, 0};        /* pula_vez[j] = 1: jogador j caiu em armadilha */
    int meus_achados = 0, achados_dele = 0;
    int linha, coluna, quem_jogou;
    char resultado;
    char casa[16];
    char msg[TAM_MSG];
    char enviado[TAM_MSG]  = "-";
    char recebido[TAM_MSG] = "-";
    char aviso[100] = "";
    char **meu_mapa = cria_mapa(AGUA);
    char **visto    = cria_mapa(DESCONHECIDO);

    if (meu_mapa == NULL || visto == NULL) {
        printf("Erro: memoria insuficiente para criar o mapa\n");
        libera_mapa(meu_mapa);
        libera_mapa(visto);
        return;
    }
    sorteia_mapa(meu_mapa);

    while (meus_achados < NUM_TESOUROS && achados_dele < NUM_TESOUROS) {
        desenha_tela(meu_mapa, visto, eu, meus_achados, achados_dele, enviado, recebido, aviso);

        if (vez == eu) {
            /* ---------- MINHA VEZ: escolho uma casa e pergunto ---------- */
            printf(" SUA VEZ! Digite uma casa (ex: C4): ");
            scanf("%15s", casa);

            if (!converte_casa(casa, &linha, &coluna)) {
                strcpy(aviso, "Casa invalida! Use letra A-F e numero 1-6.");
                continue;
            }
            if (visto[linha][coluna] != DESCONHECIDO) {
                strcpy(aviso, "Voce ja escolheu essa casa! Escolha outra.");
                continue;
            }

            memset(msg, 0, TAM_MSG);
            sprintf(msg, "%d%c%c", eu, 'A' + coluna, '1' + linha);   /* ex: "1C4" */
            if (send(conexao, msg, TAM_MSG, 0) == SOCKET_ERROR) {
                printf("Erro ao enviar dados: %d\n", WSAGetLastError());
                break;
            }
            strcpy(enviado, msg);

            printf(" Esperando a resposta...\n");
            if (recebe_tudo(conexao, msg, TAM_MSG) != TAM_MSG) {
                printf("O outro jogador desconectou.\n");
                break;
            }
            msg[TAM_MSG - 1] = '\0';
            strcpy(recebido, msg);

            if (msg[1] != 'R') {
                printf("Mensagem fora da notacao: %s\n", msg);
                break;
            }
            resultado = msg[2];
            visto[linha][coluna] = resultado;

            if (resultado == TESOURO) {
                meus_achados++;
                sprintf(aviso, "TESOURO em %c%c!", 'A' + coluna, '1' + linha);
            } else if (resultado == ARMADILHA) {
                pula_vez[eu] = 1;
                sprintf(aviso, "ARMADILHA em %c%c! Voce perde a proxima vez.", 'A' + coluna, '1' + linha);
            } else {
                sprintf(aviso, "Agua em %c%c... tesouro mais perto a %d casa(s).",
                        'A' + coluna, '1' + linha, atoi(msg + 3));
            }
            quem_jogou = eu;
        } else {
            /* ---------- VEZ DO OUTRO: espero a pergunta e respondo ---------- */
            printf(" Vez do jogador %d. Esperando a jogada...\n", outro);
            if (recebe_tudo(conexao, msg, TAM_MSG) != TAM_MSG) {
                printf("O outro jogador desconectou.\n");
                break;
            }
            msg[TAM_MSG - 1] = '\0';
            strcpy(recebido, msg);

            if (!converte_casa(msg + 1, &linha, &coluna)) {   /* msg + 1 pula o numero do jogador */
                printf("Mensagem fora da notacao: %s\n", msg);
                break;
            }

            resultado = consulta_casa(meu_mapa, linha, coluna);
            memset(msg, 0, TAM_MSG);
            if (resultado == AGUA)
                sprintf(msg, "%dRA%d", eu, distancia_tesouro(meu_mapa, linha, coluna));
            else
                sprintf(msg, "%dR%c", eu, resultado);

            if (send(conexao, msg, TAM_MSG, 0) == SOCKET_ERROR) {
                printf("Erro ao enviar dados: %d\n", WSAGetLastError());
                break;
            }
            strcpy(enviado, msg);

            if (resultado == TESOURO) {
                achados_dele++;
                sprintf(aviso, "O adversario achou um tesouro seu em %c%c!", 'A' + coluna, '1' + linha);
            } else if (resultado == ARMADILHA) {
                pula_vez[outro] = 1;
                sprintf(aviso, "O adversario caiu na armadilha em %c%c! Ele perde a proxima vez.",
                        'A' + coluna, '1' + linha);
            } else {
                sprintf(aviso, "O adversario errou: agua em %c%c.", 'A' + coluna, '1' + linha);
            }
            quem_jogou = outro;
        }

        /* ---------- PASSA A VEZ (mesma regra nos dois processos) ---------- */
        vez = (quem_jogou == 1) ? 2 : 1;
        /* Quem caiu na armadilha perde esta vez, e ela passa para o outro.
           E um 'while' (e nao um 'if') porque o outro tambem pode estar
           punido: se os dois cairam em armadilha, as duas punicoes se cancelam. */
        while (pula_vez[vez]) {
            pula_vez[vez] = 0;
            vez = (vez == 1) ? 2 : 1;
        }
    }

    /* ---------- FIM DE JOGO ---------- */
    desenha_tela(meu_mapa, visto, eu, meus_achados, achados_dele, enviado, recebido, aviso);
    if (meus_achados == NUM_TESOUROS)
        printf(" *** VOCE VENCEU! Achou todos os tesouros. ***\n\n");
    else if (achados_dele == NUM_TESOUROS)
        printf(" Voce perdeu... o adversario achou todos os seus tesouros.\n\n");

    libera_mapa(meu_mapa);    /* devolve a memoria alocada com malloc */
    libera_mapa(visto);
}

/* Modo de teste do mapa (opcao 3): joga sozinho contra um mapa sorteado,
   sem rede. Serve para testar o mapa antes de juntar com a conexao. */
void testa_mapa(void) {
    char **meu_mapa = cria_mapa(AGUA);          /* mapa secreto, com T, X e agua */
    char **visto    = cria_mapa(DESCONHECIDO);  /* o que o jogador ja descobriu */
    char casa[16];
    char resposta;
    int linha, coluna;
    int achados = 0;

    if (meu_mapa == NULL || visto == NULL) {
        printf("Erro: memoria insuficiente para criar o mapa\n");
        libera_mapa(meu_mapa);
        libera_mapa(visto);
        return;
    }

    sorteia_mapa(meu_mapa);

    while (achados < NUM_TESOUROS) {
        system("cls");
        desenha_mapa(meu_mapa, "Mapa secreto (aparece so no modo teste)");
        desenha_mapa(visto,    "O que voce ja descobriu");
        printf("\nTesouros encontrados: %d de %d\n", achados, NUM_TESOUROS);
        printf("Digite uma casa (ex: C4) ou 0 para sair: ");
        scanf("%15s", casa);

        if (strcmp(casa, "0") == 0) break;

        if (!converte_casa(casa, &linha, &coluna)) {
            printf("Casa invalida! Use letra A-F e numero 1-6.\n");
            system("pause");
            continue;
        }
        if (visto[linha][coluna] != DESCONHECIDO) {
            printf("Voce ja escolheu essa casa!\n");
            system("pause");
            continue;
        }

        resposta = consulta_casa(meu_mapa, linha, coluna);
        visto[linha][coluna] = resposta;

        if (resposta == TESOURO) {
            achados++;
            printf("TESOURO!\n");
        } else if (resposta == ARMADILHA) {
            printf("ARMADILHA! (no jogo, voce perde a proxima vez)\n");
        } else {
            printf("Agua... tesouro mais perto a %d casa(s).\n",
                   distancia_tesouro(meu_mapa, linha, coluna));
        }
        system("pause");
    }

    if (achados == NUM_TESOUROS)
        printf("\nVoce encontrou todos os tesouros!\n");

    libera_mapa(meu_mapa);   /* devolve a memoria alocada com malloc */
    libera_mapa(visto);
}

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

    /* Semente do sorteio: hora atual misturada com o numero do processo.
       So a hora nao basta: se os dois jogadores abrirem no mesmo segundo,
       os dois mapas sairiam iguais. */
    srand((unsigned) time(NULL) ^ (unsigned) GetCurrentProcessId());

    printf("\n1 - Criar partida (servidor)\n");
    printf("2 - Entrar em partida (cliente)\n");
    printf("3 - Testar o mapa (sem rede)\n> ");
    scanf("%d", &opcao);

    if (opcao == 3) {
        testa_mapa();
        WSACleanup();
        system("pause");
        return 0;
    }

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

    /* ===== Partida: o servidor e o jogador 1, o cliente e o jogador 2 ===== */
    joga_partida(conexao, (opcao == 1) ? 1 : 2);

    /* ===== Finaliza socket e biblioteca ===== */
    closesocket(conexao);
    WSACleanup();
    system("pause");
    return 0;
}
