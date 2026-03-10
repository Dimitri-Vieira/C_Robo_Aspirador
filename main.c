#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
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
    int cap, ini, sz; // cap(capacidade máxima), ini(inicio da lista), sz(registro atual)
} Log;

// Operações do log
void log_init(Log *L, int capacidadeMax){
    L->v = malloc(capacidadeMax * sizeof(Acao));
    L->cap = capacidadeMax;
    L->ini = 0;
    L->sz = 0;
}

void log_push(Log *L, Acao novaAcao){
    int fim = (L->ini + L->sz) % L->cap;
    L->v[fim] = novaAcao;

    if(L->sz < L->ini) {
        L->sz++;

    } else {
        L->ini = (L->ini + 1) % L->cap;
    }
}

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
                printf("Ponto inicial 'S' encontrado. Coordenadas definidas: [%d][%d]\n", M->S.r, M->S.c);
            }

            if(area == '*') {
                M->sujeira_total++;
            }
            
        }
        
        strcpy(M->g[i], linhaParaValidacao); //insere os dados validados
    }
    
    return 0; 
}

void imprimir_mapa(int modoPAP, Mapa *M, Ponto pos){
    if(modoPAP == 1) {
        printf("\n***** Mapa *****\n");
        for(int i = 0; i < M->N; i++) {
            for(int j = 0; j < M->M; j++) {
                if(pos.r == i && pos.c == j) {
                    printf("R");
                } else {
                    printf("%c", M->g[i][j]);
                }
            }
            printf("\n");
        }
    }
}

int dentro(Mapa *M, Ponto pos) {
    if(pos.r >= 0 && pos.r < M->N && pos.c >= 0 && pos.c < M->M) {
        return 1;
    }
    return 0;
}

int eh_sujo(Mapa *M, Ponto pos) {
    if(dentro(M, pos) && M->g[pos.r][pos.c] == '*') {
        return 1;
    }
    return 0;
}

int eh_bloqueio(Mapa *M, Ponto pos) {
    if(!dentro(M, pos) || M->g[pos.r][pos.c] == '#') {
        return 1;
    }
    return 0;
}

void espera_enter(){
    printf("Pressione [ENTER] para continuar...\n");
    flush_stdin();
}

const char* nome_acao(Acao a) {
    switch(a) {
        case LIMPAR: return "LIMPAR";
        case MOVER_N: return "MOVER_N";
        case MOVER_S: return "MOVER_S";
        case MOVER_L: return "MOVER_L";
        case MOVER_O: return "MOVER_O";
        case FICAR: return "FICAR";
    }
    return "DESCONHECIDO";
}


Acao decide_reflex(Mapa *M, Ponto pos, char *motivo) {
    // Regra 1: Celula atual é suja
    if (eh_sujo(M, pos)) {
        strcpy(motivo, "Regra 1: Celula atual suja. Acao: LIMPAR.");
        return LIMPAR;
    }
    
    // Preparando as posições vizinhas
    Ponto n = {pos.r - 1, pos.c}; // Norte
    Ponto s = {pos.r + 1, pos.c}; // Sul
    Ponto l = {pos.r, pos.c + 1}; // Leste
    Ponto o = {pos.r, pos.c - 1}; // Oeste

    // Regra 2: Há sujeira em vizinhos N/S/L/O
    if (eh_sujo(M, n)) { strcpy(motivo, "Regra 2: Vizinho sujo ao Norte."); return MOVER_N; }
    if (eh_sujo(M, s)) { strcpy(motivo, "Regra 2: Vizinho sujo ao Sul."); return MOVER_S; }
    if (eh_sujo(M, l)) { strcpy(motivo, "Regra 2: Vizinho sujo a Leste."); return MOVER_L; }
    if (eh_sujo(M, o)) { strcpy(motivo, "Regra 2: Vizinho sujo a Oeste."); return MOVER_O; }
    
    // Regra 3: Varrer em zig-zag pelas colunas
    if (pos.c % 2 == 0) { 
        // Coluna par: tenta leste, se bloqueado desce, se bloqueado oeste
        if (!eh_bloqueio(M, l)) { strcpy(motivo, "Regra 3 (zig-zag): Coluna par, Leste livre."); return MOVER_L; }
        if (!eh_bloqueio(M, s)) { strcpy(motivo, "Regra 3 (zig-zag): Leste bloqueado, descendo Sul."); return MOVER_S; }
        if (!eh_bloqueio(M, o)) { strcpy(motivo, "Regra 3 (zig-zag): Sem Leste/Sul, indo Oeste."); return MOVER_O; }
    } else { 
        // Coluna ímpar: tenta oeste, senao desce, senao leste
        if (!eh_bloqueio(M, o)) { strcpy(motivo, "Regra 3 (zig-zag): Coluna impar, Oeste livre."); return MOVER_O; }
        if (!eh_bloqueio(M, s)) { strcpy(motivo, "Regra 3 (zig-zag): Oeste bloqueado, descendo Sul."); return MOVER_S; }
        if (!eh_bloqueio(M, l)) { strcpy(motivo, "Regra 3 (zig-zag): Sem Oeste/Sul, indo Leste."); return MOVER_L; }
    }
    
    // Regra 4: Fallback (se travar, tenta alternativo)
    if (!eh_bloqueio(M, n)) { strcpy(motivo, "Fallback: Travado no zig-zag, tentando Norte."); return MOVER_N; }
    if (!eh_bloqueio(M, s)) { strcpy(motivo, "Fallback: Norte bloqueado, tentando Sul."); return MOVER_S; } 
    
    strcpy(motivo, "Fallback: Preso! Nenhuma direcao segura.");
    return FICAR; 
}

// Executa a ação e atualiza os placares
int aplicar_acao(Mapa *M, Ponto *pos, Acao a, int *limpezas, int *bloqueios) {
    if (a == LIMPAR) {
        if (M->g[pos->r][pos->c] == '*') {
            M->g[pos->r][pos->c] = '.';
            M->sujeira_total--;
            (*limpezas)++;
        }
        return 1; // 1 = ok
    }
    
    if (a == FICAR) return 1;
    
    Ponto dest = *pos;
    if (a == MOVER_N) dest.r--;
    else if (a == MOVER_S) dest.r++;
    else if (a == MOVER_L) dest.c++;
    else if (a == MOVER_O) dest.c--;
    
    if (eh_bloqueio(M, dest)) {
        (*bloqueios)++;
        return 0; // 0 = bloqueado
    }
    
    *pos = dest; // Atualiza posicao
    return 1;
}

int main () {
    
    printf("Para usar um mapa pre-definido digite: 1\nPara usar um mapa personalizado digite: 2\nOpcao: ");
    int opcaoMapa;
    scanf("%i", &opcaoMapa);

    Mapa M;
    M.sujeira_total = 0;
    M.S.c = -1; 
    M.S.r = -1;
    Ponto pos;
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

    
    if(M.S.r == -1 || M.S.c == -1) {
        // defininfo ponto inicial caso ainda não haja
        printf("Verificando ponto inicial do robo...\n");
        for(int i = 0; i < M.N; i++) {
            for(int j = 0; j < M.M; j++) {
                
                if(M.g[i][j] == '.') {
                    M.S.r = i;
                    M.S.c = j;
                    printf("Ponto inicial nao encontrado. Definindo ponto inicial com base no primeiro '.' que aparece. Coordenadas:%d %d\n", M.S.r, M.S.c);
                    break;
                }
            }   
        }
    } 
    // fallback para definicao do ponto inicial
    if(M.S.r == -1 || M.S.c == -1) {
        M.S.r = 0;
        M.S.c = 0;
        printf("Ponto inicial nao encontrado e o mapa nao possui NENHUMA area limpa. Ponto inicial definido como [%d][%d].(FALLBACK FORCADO)\n", M.S.r, M.S.c);
    }

    // definindo posicionamento atual do robo
    pos.r = M.S.r;
    pos.c = M.S.c;

    // modo passo-a-passo
    int modoPAP;
    printf("Deseja modo passo-a-passo? Digite 1(sim) ou 0(nao): ");
    scanf("%d", &modoPAP);
    flush_stdin();

    imprimir_mapa(modoPAP, &M, pos); // Imprime a posição de nascimento do robô

    // Inicializando Buffer Circular para as últimas 64 ações
    Log historico;
    log_init(&historico, 64); 

    // Variáveis para as Métricas Finais
    int limpezas = 0;
    int bloqueios = 0;
    int passos_executados = 0;
    int sujeira_inicial = M.sujeira_total;
    clock_t inicio_tempo = clock();
    char motivo[200];

    // LAÇO PRINCIPAL DE SIMULAÇÃO
    for (int t = 0; t < M.T; t++) {
        
        if (M.sujeira_total == 0) {
            printf("\nToda a sujeira foi limpa.\n");
            break;
        }

        // 1. Decide a ação
        Acao acao_escolhida = decide_reflex(&M, pos, motivo);
        
        // 2. Executa
        int status_ok = aplicar_acao(&M, &pos, acao_escolhida, &limpezas, &bloqueios);
        
        // 3. Registra
        log_push(&historico, acao_escolhida);
        passos_executados++;

        // 4. Interface Passo-a-Passo
        if (modoPAP == 1) {
            printf("\n--- Passo %d ---\n", passos_executados); 
            printf("Acao: %s\n", nome_acao(acao_escolhida)); 
            printf("Motivo: %s\n", motivo); 
            printf("Status: %s\n", status_ok ? "OK" : "BLOQUEADO"); 
            
            imprimir_mapa(modoPAP, &M, pos); 
            
            // Exibindo métricas parciais
            printf("Sujeira restante: %d | Limpezas: %d | Bloqueios: %d\n", 
                   M.sujeira_total, limpezas, bloqueios);
            
            espera_enter();
        }
    }

    // Calculando o tempo total e porcentagem
    clock_t fim_tempo = clock();
    double tempo_cpu = ((double)(fim_tempo - inicio_tempo)) / CLOCKS_PER_SEC;
    
    float porcentagem_removida = 0;
    if (sujeira_inicial > 0) {
        porcentagem_removida = ((sujeira_inicial - M.sujeira_total) / (float)sujeira_inicial) * 100.0;
    }

    // TELA FINAL COM RESULTADOS E MÉTRICAS
    printf("\n========== RESULTADOS FINAIS ==========\n");
    printf("Passos Executados: %d de %d permitidos\n", passos_executados, M.T); 
    printf("Limpezas realizadas: %d\n", limpezas); 
    printf("Tentativas bloqueadas: %d\n", bloqueios); 
    printf("Sujeira inicial: %d | Sujeira restante: %d\n", sujeira_inicial, M.sujeira_total); 
    printf("Porcentagem limpa: %.2f%%\n", porcentagem_removida); 
    printf("Tempo de CPU: %f segundos\n", tempo_cpu); 

    // liberando a memória
    free(historico.v);
    for(int k = 0; k < M.M; k++) {
        free(M.g[k]); // Libera cada coluna
    }
    free(M.g); // Libera o array principal de linhas

    return 0;
}