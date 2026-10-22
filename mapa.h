/*
 * mapa.h - "cardapio" das funcoes do mapa
 * Quem quiser usar o mapa (como o main.c) da #include "mapa.h"
 */
#ifndef MAPA_H
#define MAPA_H

#define TAM_MAPA        6   /* mapa 6x6: colunas A-F, linhas 1-6 */
#define NUM_TESOUROS    5
#define NUM_ARMADILHAS  4

/* O que pode ter em cada casa (mesmas letras da notacao das mensagens) */
#define AGUA            'A'
#define TESOURO         'T'
#define ARMADILHA       'X'
#define TESOURO_ACHADO  't'   /* tesouro que o adversario ja encontrou */
#define DESCONHECIDO    '.'   /* casa que ainda nao foi descoberta */

char **cria_mapa(char preenchimento);
void   sorteia_mapa(char **mapa);
void   libera_mapa(char **mapa);
void   desenha_mapa(char **mapa, const char *titulo);
int    converte_casa(const char *texto, int *linha, int *coluna);
char   consulta_casa(char **mapa, int linha, int coluna);
int    distancia_tesouro(char **mapa, int linha, int coluna);

#endif
