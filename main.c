#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    int r, c; // guarda linha (r) e coluna (c) do robô
} Ponto;

typedef enum { LIMPAR, MOVER_N, MOVER_S, MOVER_L, MOVER_O, FICAR } Acao; // Decisões do agente

typedef struct { 
    Acao *v; 
    int cap, ini, sz; // cap(capacidade fixa), 
} Log;

// Operações do log
void log_init(){}
void log_push(){}

typedef struct { 
    int N, M, T; // N e M (dimensões), T(limite de passos)
    char **g; // matriz de caracteres com valores: . limpo, * sujo, # obstáculo.
    Ponto S; //  é a posição inicial lida do mapa
    int sujeira_total;
} Mapa;


int main () {

    return 0;
}