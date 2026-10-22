/*
 * mapa.c - tudo sobre o tabuleiro: criar, sortear, desenhar e consultar
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "mapa.h"

/* Cria um mapa TAM_MAPA x TAM_MAPA na memoria (malloc) com todas as casas
   iguais a 'preenchimento'. Retorna NULL se faltar memoria.
   A matriz e um vetor de linhas: mapa[linha][coluna]. */
char **cria_mapa(char preenchimento) {
    int i, j;
    char **mapa = (char **) malloc(TAM_MAPA * sizeof(char *));
    if (mapa == NULL) return NULL;

    for (i = 0; i < TAM_MAPA; i++) {
        mapa[i] = (char *) malloc(TAM_MAPA * sizeof(char));
        if (mapa[i] == NULL) {
            /* faltou memoria no meio: devolve o que ja foi alocado */
            while (--i >= 0) free(mapa[i]);
            free(mapa);
            return NULL;
        }
        for (j = 0; j < TAM_MAPA; j++)
            mapa[i][j] = preenchimento;
    }
    return mapa;
}

/* Espalha os tesouros e as armadilhas em casas aleatorias (sem repetir casa).
   O srand() precisa ter sido chamado antes, uma vez, no main. */
void sorteia_mapa(char **mapa) {
    int i, linha, coluna;

    for (i = 0; i < NUM_TESOUROS; i++) {
        do {
            linha  = rand() % TAM_MAPA;
            coluna = rand() % TAM_MAPA;
        } while (mapa[linha][coluna] != AGUA);   /* sorteia de novo se a casa ja tem algo */
        mapa[linha][coluna] = TESOURO;
    }

    for (i = 0; i < NUM_ARMADILHAS; i++) {
        do {
            linha  = rand() % TAM_MAPA;
            coluna = rand() % TAM_MAPA;
        } while (mapa[linha][coluna] != AGUA);
        mapa[linha][coluna] = ARMADILHA;
    }
}

/* Devolve a memoria do mapa (free): primeiro cada linha, depois o vetor de linhas */
void libera_mapa(char **mapa) {
    int i;
    if (mapa == NULL) return;
    for (i = 0; i < TAM_MAPA; i++)
        free(mapa[i]);
    free(mapa);
}

/* Escolhe o simbolo que aparece na tela para cada tipo de casa */
static char simbolo(char casa) {
    switch (casa) {
        case AGUA:           return '~';
        case TESOURO:        return 'T';
        case ARMADILHA:      return 'X';
        case TESOURO_ACHADO: return '*';
        default:             return '.';
    }
}

/* Desenha o mapa na tela, com as letras das colunas e os numeros das linhas */
void desenha_mapa(char **mapa, const char *titulo) {
    int i, j;

    printf("\n %s\n", titulo);
    printf("    ");
    for (j = 0; j < TAM_MAPA; j++)
        printf("%c ", 'A' + j);
    printf("\n");

    for (i = 0; i < TAM_MAPA; i++) {
        printf(" %d  ", i + 1);
        for (j = 0; j < TAM_MAPA; j++)
            printf("%c ", simbolo(mapa[i][j]));
        printf("\n");
    }
}

/* Converte o texto digitado (ex: "C4" ou "c4") em linha e coluna da matriz.
   Coluna: letra A-F -> 0-5.  Linha: numero 1-6 -> 0-5.
   Retorna 1 se a casa e valida, 0 se nao for. */
int converte_casa(const char *texto, int *linha, int *coluna) {
    char letra, numero;

    if (texto[0] == '\0' || texto[1] == '\0' || texto[2] != '\0')
        return 0;                                   /* precisa ter exatamente 2 caracteres */

    letra  = (char) toupper((unsigned char) texto[0]);
    numero = texto[1];

    if (letra < 'A' || letra >= 'A' + TAM_MAPA)  return 0;
    if (numero < '1' || numero >= '1' + TAM_MAPA) return 0;

    *coluna = letra - 'A';
    *linha  = numero - '1';
    return 1;
}

/* Diz o que tem na casa: TESOURO, AGUA ou ARMADILHA.
   Se for tesouro, marca como achado, para nao contar duas vezes na dica de distancia. */
char consulta_casa(char **mapa, int linha, int coluna) {
    char casa = mapa[linha][coluna];

    if (casa == TESOURO) {
        mapa[linha][coluna] = TESOURO_ACHADO;
        return TESOURO;
    }
    if (casa == TESOURO_ACHADO)
        return AGUA;            /* esse tesouro ja foi encontrado antes */
    return casa;
}

/* Dica da agua: quantas casas ate o tesouro (ainda nao achado) mais proximo.
   Conta como o rei do xadrez anda: diagonal tambem vale 1 casa.
   Retorna -1 se nao sobrou nenhum tesouro. */
int distancia_tesouro(char **mapa, int linha, int coluna) {
    int i, j, dl, dc, d;
    int menor = -1;

    for (i = 0; i < TAM_MAPA; i++) {
        for (j = 0; j < TAM_MAPA; j++) {
            if (mapa[i][j] != TESOURO) continue;
            dl = abs(i - linha);
            dc = abs(j - coluna);
            d  = (dl > dc) ? dl : dc;
            if (menor == -1 || d < menor)
                menor = d;
        }
    }
    return menor;
}
