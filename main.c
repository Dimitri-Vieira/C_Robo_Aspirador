#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//implementar a MACRO do MALLOC/FREE

void flush_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

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

int linha_valida(FILE *arquivoMapa, Mapa *M){

    printf("validando dados da composicao do mapa...\n");

    char linhaParaValidacao[1024];
    for(int i = 0; i < M->N; i++){

        // se for a primeira linha do arquivo, pula esse primeiro loop
        if (i == 0){
            fgets(linhaParaValidacao, 1024, arquivoMapa);
        }

        // le a linha
        fgets(linhaParaValidacao, 1024, arquivoMapa);
        
        // retira o "\n" e troca por "\0"
        linhaParaValidacao[strcspn(linhaParaValidacao, "\n")] = '\0';
        
        // se o total de colunas forem maior que o previsto
        int quantidadeChar = strlen(linhaParaValidacao);
        if(quantidadeChar != M->M) {
            printf("Erro: tamanho de coluna em uma das linhas do mapa nao corresponde ao tamanho informado no cabecalho do arquivo de mapa. Verifique o arquivo e corrija-o.");
            return 1;
        }

        // verifica se tem somente caracteres válidos
        for(int j = 0; j < M->M; j++) {

            char area = linhaParaValidacao[j];

            if(area != '.' && area != '*' && area != '#' && area != 'S') {
                printf("Erro: caracter invalido em uma das linhas do mapa. Verifique o arquivo e corrija-o.");
                return 1;
            }

            if (area == 'S'){
                M->S.r = i;
                M->S.c = j;
                linhaParaValidacao[j] = '.'; //substitui o "S" por um "."
            }

            if(area == '*') {
                M->sujeira_total++;
            }
            
        }

        strcpy(M->g[i], linhaParaValidacao); //insere os dados validados
    }

    return 0; 
}

int main () {
    
    printf("Para usar um mapa pre-definido digite: 1\nPara usar um mapa personalizado digite: 2\nOpcao: ");
    int opcaoMapa;
    scanf("%i", &opcaoMapa);

    Mapa M;
    M.S.c, M.S.r = -1;
    FILE *arquivoMapa;
    char nomeArquivoMapa[30] = "";

    // escolhendo tipo de mapa
    if (opcaoMapa == 1) {
        // escolhendo nivel do mapa pre-definido
        int nivelMapa;
        printf("Escolha o nivel do mapa digitando: 1-facil | 2-medio | 3-dificil\nNivel do mapa: ");
        scanf("%d", &nivelMapa);
        flush_stdin();
        
        switch(nivelMapa) {
            case 1:
                strcpy(nomeArquivoMapa, "../mapa_facil.txt");
                break;

            case 2:
                strcpy(nomeArquivoMapa, "../mapa_medio.txt");
                break;

            case 3:
                strcpy(nomeArquivoMapa, "../mapa_dificil.txt");
                break;

            default:
                printf("Erro: valor de nivel invalido. Digite 1, 2 ou 3.");
                return 1;
                break;
        }

        // recebe os parâmetros do mapa e validando valores dos parametros
        arquivoMapa = fopen(nomeArquivoMapa, "r");
        if(arquivoMapa == NULL) {
            printf("Erro: não foi possível abrir o arquivo para leitura.\n");
            return 1;
        }

        fscanf(arquivoMapa, "%d %d %d", &M.N, &M.M, &M.T);
        printf("%d %d %d\n", M.N, M.M, M.T);
        rewind(arquivoMapa);
        fclose(arquivoMapa);

        if(M.N < 0 || M.M < 0 || M.T < 0) {
            printf("Erro: valor de N, M ou T inválidos. Verifique o arquivo de mapa predefinido escolhido e veja se a estrutura está correta. Os valorem devem ser: N e M > 0 e T >= 0\n");
            return 1;
        }

    }
    else {
        strcpy(nomeArquivoMapa, "../mapa_personalizado.txt");

        // abre o arquivo em modo w para apagar o último layout do mapa
        arquivoMapa = fopen(nomeArquivoMapa, "w");
        if(arquivoMapa == NULL) {
            printf("Erro: não foi possível abrir o arquivo para escrever nele.\n");
            return 1;
        }
        fclose(arquivoMapa);

        // recebe os parâmetros do mapa e os coloca no respectivo arquivo
        do {
            printf("Digite linhas(N) colunas(M) limite de passos(T): ");
            scanf("%d %d %d", &M.N, &M.M, &M.T);

            if(M.N < 0 || M.M < 0 || M.T < 0) {
                printf("Erro: valor de N, M ou T inválidos. Insira o valor correto onde N e M > 0 e T >= 0.\n");
            }

            flush_stdin();

        } while(M.N < 0 || M.M < 0 || M.T < 0);
        
        
        arquivoMapa = fopen(nomeArquivoMapa, "a");
        if(arquivoMapa == NULL) {
            printf("Erro: não foi possível abrir o arquivo para escrever nele.\n");
            return 1;
        }

        fprintf(arquivoMapa, "%i %i %i", M.N, M.M, M.T);
        fclose(arquivoMapa);

        // recebe e preenche o arquivo de mapa personalizado com o novo layout
        printf("Crie o mapa digitando a composicao de cada uma das linhas. Nao esqueca que a quantidade de caracteres digitados por linha deve ser exatamente igual a M(quantidade de colunas).\n");
        printf("Caracteres aceitos: . (limpo), * (sujo), # (obstaculo), S (posicao inicial do robo)\n");

        char layoutMapa[M.M];

        for(int i = 0; i < M.N; i++) {
            printf("Linha %i: ", i);
            fgets(layoutMapa, M.M + 1, stdin);
            flush_stdin();

            arquivoMapa = fopen(nomeArquivoMapa, "a");
            if(arquivoMapa == NULL) {
                printf("Erro: não foi possível abrir o arquivo para escrever nele.\n");
                return 1;
            }

            fprintf(arquivoMapa, "\n%s", layoutMapa);
            fclose(arquivoMapa);
        }
    }

    // alocando memoria dinamica para as linhas da matriz
    M.g = malloc(M.N * sizeof(char *));
    
    // alocando memoria dinamica para as colunas da matriz 
    for(int k = 0; k < M.M; k++) {
        M.g[k] = malloc((M.M + 1) * sizeof(char));
    }

    arquivoMapa = fopen(nomeArquivoMapa, "r");
    if (arquivoMapa == NULL) {
        printf("Erro: não foi possível abrir o arquivo para leitura.\n");
        return 1;
    }
    rewind(arquivoMapa);

    linha_valida(arquivoMapa, &M);

    fclose(arquivoMapa);

    // defininfo ponto inicial caso ainda não haja
    printf("Verificando ponto inicial do robo...\n");

    if(M.S.r == -1 || M.S.c == -1) {

        for(int i = 0; i < M.N; i++) {
            for(int j = 0; j < M.M; j++) {
                
                if(M.g[i][j] == '.') {
                    M.S.r = i;
                    M.S.c = j;
                    printf("Ponto inicial definido.%d %d\n", M.S.r, M.S.c);
                    break;
                }
            }   
        }
    }

    return 0;
} 