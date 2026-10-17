#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// --- CONFIGURACOES GLOBAIS ---
#define MAX_CANDIDATOS 500
#define MAX_NOME 10
#define NUM_REPETICOES 10

// Parametros do Evolutivo
#define TAM_POPULACAO 50
#define NUM_GERACOES 1000
#define PROB_MUTACAO 20
#define PROB_CRUZAMENTO 70

// Parametros dos Hibridos
#define ITERS_HIBRIDO_1 1000  // Trepa-Colinas intenso no melhor
#define ITERS_HIBRIDO_2 50    // Trepa-Colinas leve nos filhos

struct Instancia {
    int num_candidatos;
    int num_a_selecionar;
    float matriz_dist[MAX_CANDIDATOS][MAX_CANDIDATOS];
    char ids[MAX_CANDIDATOS][MAX_NOME];
    int count_ids;
} dados;

typedef struct {
    int genes[MAX_CANDIDATOS];
    float fitness;
} Individuo;

Individuo populacao[TAM_POPULACAO];
Individuo nova_geracao[TAM_POPULACAO];

// --- 1. LEITURA DE DADOS ---
int obter_indice(char *nome) {
    for (int i = 0; i < dados.count_ids; i++) if (strcmp(dados.ids[i], nome) == 0) return i;
    strcpy(dados.ids[dados.count_ids], nome); return dados.count_ids++;
}

void carregar_ficheiro(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) { printf("ERRO: Nao encontrei '%s'.\n", filename); exit(1); }
    if (fscanf(f, "%d %d", &dados.num_candidatos, &dados.num_a_selecionar) != 2) exit(1);
    dados.count_ids = 0;
    for(int i=0; i<MAX_CANDIDATOS; i++) for(int j=0; j<MAX_CANDIDATOS; j++) dados.matriz_dist[i][j] = 0.0;
    char n1[MAX_NOME], n2[MAX_NOME]; float d;
    while (fscanf(f, "%s %s %f", n1, n2, &d) == 3) {
        int i = obter_indice(n1); int j = obter_indice(n2);
        dados.matriz_dist[i][j] = dados.matriz_dist[j][i] = d;
    }
    fclose(f);
}

// --- 2. FUNCAO DE AVALIACAO ---
float calcular_custo(int solucao[], int m) {
    float soma = 0.0;
    for (int i = 0; i < m - 1; i++) for (int j = i + 1; j < m; j++) soma += dados.matriz_dist[solucao[i]][solucao[j]];
    return soma / (float)m;
}

// --- 3. FUNCOES AUXILIARES E REPARACAO ---
bool existe(int valor, int *vetor, int tam) {
    for(int i=0; i<tam; i++) if(vetor[i] == valor) return true;
    return false;
}

void gerar_solucao_aleatoria(int *genes) {
    int sel = 0;
    while(sel < dados.num_a_selecionar) {
        int r = rand() % dados.num_candidatos;
        if (!existe(r, genes, sel)) genes[sel++] = r;
    }
}

void reparar_individuo(int *genes) {
    int m = dados.num_a_selecionar;
    bool usados[MAX_CANDIDATOS] = {false};
    for (int i = 0; i < m; i++) {
        if (usados[genes[i]]) genes[i] = -1; else usados[genes[i]] = true;
    }
    for (int i = 0; i < m; i++) {
        if (genes[i] == -1) {
            int novo;
            do { novo = rand() % dados.num_candidatos; } while (usados[novo]);
            genes[i] = novo;
            usados[novo] = true;
        }
    }
}

// --- 4. VIZINHANÇAS (REQUISITO: 2 DIFERENTES) ---
void vizinhanca_swap(int *vizinho, int m) {
    int pos = rand() % m;
    int novo;
    do { novo = rand() % dados.num_candidatos; } while (existe(novo, vizinho, m));
    vizinho[pos] = novo;
}

void vizinhanca_double_swap(int *vizinho, int m) {
    vizinhanca_swap(vizinho, m);
    vizinhanca_swap(vizinho, m);
}

// --- 5. ALGORITMO 1: PESQUISA LOCAL (TREPA-COLINAS) ---
float executar_trepa_colinas(int iteracoes, int tipo_vizinhanca) {
    int atual[MAX_CANDIDATOS], vizinho[MAX_CANDIDATOS];
    gerar_solucao_aleatoria(atual);
    float custo_atual = calcular_custo(atual, dados.num_a_selecionar);

    for (int it = 0; it < iteracoes; it++) {
        for(int i=0; i<dados.num_a_selecionar; i++) vizinho[i] = atual[i];

        if (tipo_vizinhanca == 2) vizinhanca_double_swap(vizinho, dados.num_a_selecionar);
        else vizinhanca_swap(vizinho, dados.num_a_selecionar);

        float custo_vizinho = calcular_custo(vizinho, dados.num_a_selecionar);
        if (custo_vizinho >= custo_atual) {
            for(int i=0; i<dados.num_a_selecionar; i++) atual[i] = vizinho[i];
            custo_atual = custo_vizinho;
        }
    }
    return custo_atual;
}

// FUNCAO DE SUPORTE AOS HIBRIDOS
// Esta funcao É o algoritmo Trepa-Colinas aplicado a um individuo especifico
void aplicar_trepa_colinas_interno(int *genes, int iteracoes) {
    int m = dados.num_a_selecionar;
    int vizinho[MAX_CANDIDATOS];
    float custo = calcular_custo(genes, m);

    for(int k=0; k<iteracoes; k++) {
        for(int i=0; i<m; i++) vizinho[i] = genes[i];
        vizinhanca_swap(vizinho, m); // Hibridos usam Swap (mais rapido)
        float nv_custo = calcular_custo(vizinho, m);
        if(nv_custo > custo) {
            for(int i=0; i<m; i++) genes[i] = vizinho[i];
            custo = nv_custo;
        }
    }
}

// --- 6. OPERADORES DO ALGORITMO 2 ---
int selecao_torneio() {
    int best = rand() % TAM_POPULACAO;
    for (int i = 0; i < 2; i++) {
        int rival = rand() % TAM_POPULACAO;
        if (populacao[rival].fitness > populacao[best].fitness) best = rival;
    }
    return best;
}

int selecao_roleta() {
    float soma_fitness = 0.0;
    for(int i=0; i<TAM_POPULACAO; i++) soma_fitness += populacao[i].fitness;
    float ponto = ((float)rand() / RAND_MAX) * soma_fitness;
    float acumulado = 0.0;
    for(int i=0; i<TAM_POPULACAO; i++) {
        acumulado += populacao[i].fitness;
        if (acumulado >= ponto) return i;
    }
    return TAM_POPULACAO - 1;
}

void crossover_um_ponto(Individuo p1, Individuo p2, Individuo *f1, Individuo *f2) {
    int corte = rand() % dados.num_a_selecionar;
    for(int i=0; i<dados.num_a_selecionar; i++) {
        if(i < corte) { f1->genes[i] = p1.genes[i]; f2->genes[i] = p2.genes[i]; }
        else { f1->genes[i] = p2.genes[i]; f2->genes[i] = p1.genes[i]; }
    }
}

void crossover_uniforme(Individuo p1, Individuo p2, Individuo *f1, Individuo *f2) {
    for(int i=0; i<dados.num_a_selecionar; i++) {
        if (rand() % 2 == 0) { f1->genes[i] = p1.genes[i]; f2->genes[i] = p2.genes[i]; }
        else { f1->genes[i] = p2.genes[i]; f2->genes[i] = p1.genes[i]; }
    }
}

// --- 7. ALGORITMO 2 (EVOLUTIVO) + HIBRIDOS ---
float executar_evolutivo(int tipo_alg, int op_sel, int op_cross, int op_mut) {
    // Inicializacao
    for (int i = 0; i < TAM_POPULACAO; i++) {
        gerar_solucao_aleatoria(populacao[i].genes);
        populacao[i].fitness = calcular_custo(populacao[i].genes, dados.num_a_selecionar);
    }
    Individuo melhor = populacao[0];

    for (int g = 0; g < NUM_GERACOES; g++) {
        nova_geracao[0] = melhor; // Elitismo
        int idx = 1;
        while(idx < TAM_POPULACAO) {
            // SELECAO
            int i1 = (op_sel == 1) ? selecao_torneio() : selecao_roleta();
            int i2 = (op_sel == 1) ? selecao_torneio() : selecao_roleta();
            Individuo p1 = populacao[i1], p2 = populacao[i2];
            Individuo f1, f2;

            // CROSSOVER
            if ((rand() % 100) < PROB_CRUZAMENTO) {
                if(op_cross == 1) crossover_um_ponto(p1, p2, &f1, &f2);
                else crossover_uniforme(p1, p2, &f1, &f2);
                reparar_individuo(f1.genes); reparar_individuo(f2.genes);
            } else { f1 = p1; f2 = p2; }

            // MUTACAO
            if ((rand() % 100) < PROB_MUTACAO) {
                if(op_mut == 1) vizinhanca_swap(f1.genes, dados.num_a_selecionar);
                else vizinhanca_double_swap(f1.genes, dados.num_a_selecionar);
                reparar_individuo(f1.genes);
            }
            if ((rand() % 100) < PROB_MUTACAO) {
                if(op_mut == 1) vizinhanca_swap(f2.genes, dados.num_a_selecionar);
                else vizinhanca_double_swap(f2.genes, dados.num_a_selecionar);
                reparar_individuo(f2.genes);
            }

            // >>> HIBRIDO 2: Aplica Algoritmo 1 (PL) em todos os filhos <<<
            if (tipo_alg == 2) {
                aplicar_trepa_colinas_interno(f1.genes, ITERS_HIBRIDO_2);
                aplicar_trepa_colinas_interno(f2.genes, ITERS_HIBRIDO_2);
            }

            f1.fitness = calcular_custo(f1.genes, dados.num_a_selecionar);
            f2.fitness = calcular_custo(f2.genes, dados.num_a_selecionar);
            if(idx < TAM_POPULACAO) nova_geracao[idx++] = f1;
            if(idx < TAM_POPULACAO) nova_geracao[idx++] = f2;
        }

        // Atualiza Populacao
        int id_melhor = 0;
        for(int i=0; i<TAM_POPULACAO; i++) {
            populacao[i] = nova_geracao[i];
            if(populacao[i].fitness > populacao[id_melhor].fitness) id_melhor = i;
        }

        // >>> HIBRIDO 1: Aplica Algoritmo 1 (PL) so no melhor a cada 10 geracoes <<<
        if (tipo_alg == 1 && g % 10 == 0) {
             aplicar_trepa_colinas_interno(populacao[id_melhor].genes, ITERS_HIBRIDO_1);
             populacao[id_melhor].fitness = calcular_custo(populacao[id_melhor].genes, dados.num_a_selecionar);
        }

        if (populacao[id_melhor].fitness > melhor.fitness) melhor = populacao[id_melhor];
    }
    return melhor.fitness;
}

int main() {
    srand(time(NULL));
    int op_alg, op_fich, op_viz = 1;
    int sel=1, cross=1, mut=1;

    // Lista de ficheiros
    char *ficheiros[] = {"tourism_5.txt", "tourism_20.txt", "tourism_50.txt",
                         "tourism_100.txt", "tourism_250.txt", "tourism_500.txt"};

    // --- CICLO PRINCIPAL DO PROGRAMA ---
    while(true) {
        printf("\n========================================\n");
        printf("       MENU DE TESTES - TP2 IA          \n");
        printf("========================================\n");
        printf("Escolha o ficheiro para testar:\n");
        printf(" 0: tourism_5\n 1: tourism_20\n 2: tourism_50\n");
        printf(" 3: tourism_100\n 4: tourism_250\n 5: tourism_500\n");
        printf("-1: SAIR DO PROGRAMA\n");
        printf("----------------------------------------\n");
        printf("Opcao: ");

        if (scanf("%d", &op_fich) != 1) { // Protecao contra letras
            while(getchar() != '\n'); // Limpar buffer
            continue;
        }

        if(op_fich == -1) {
            printf("A encerrar...\n");
            break; // Sai do ciclo e termina o programa
        }

        if(op_fich < 0 || op_fich > 5) {
            printf("Opcao invalida!\n");
            continue;
        }

        // Carrega o ficheiro escolhido
        carregar_ficheiro(ficheiros[op_fich]);
        printf("\n--> Ficheiro '%s' carregado com sucesso.\n", ficheiros[op_fich]);

        printf("\nEscolha o Algoritmo:\n");
        printf("1: Pesquisa Local (Trepa-Colinas)\n");
        printf("2: Evolutivo (Genetico)\n");
        printf("3: Hibrido 1 (Melhora o Melhor - Elitista)\n");
        printf("4: Hibrido 2 (Educa os Filhos - Lamarckiano)\n");
        printf("Opcao: ");
        scanf("%d", &op_alg);

        // Configurações especificas dependendo do algoritmo
        if (op_alg == 1) {
            printf("\nConfiguracao Pesquisa Local:\n");
            printf("Vizinhanca (1=Swap Simples, 2=Double Swap): ");
            scanf("%d", &op_viz);
        } else {
            printf("\nConfiguracao Evolutivo/Hibrido:\n");
            printf("Selecao (1=Torneio, 2=Roleta): "); scanf("%d", &sel);
            printf("Crossover (1=Um Ponto, 2=Uniforme): "); scanf("%d", &cross);
            printf("Mutacao (1=Simples, 2=Dupla): "); scanf("%d", &mut);
        }

        printf("\n--- A EXECUTAR %d REPETICOES ---\n", NUM_REPETICOES);
        printf("Aguarde...\n");

        float soma = 0.0, melhor = 0.0;

        // Loop das 10 repetições
        for (int i = 0; i < NUM_REPETICOES; i++) {
            float r = 0.0;
            if (op_alg == 1) r = executar_trepa_colinas(100000, op_viz);
            else r = executar_evolutivo(op_alg - 2, sel, cross, mut);

            printf("Execucao %2d: %.2f\n", i+1, r);
            soma += r;
            if (r > melhor) melhor = r;
        }

        printf("\n----------------------------------------\n");
        printf(" RESULTADOS FINAIS (%s)\n", ficheiros[op_fich]);
        printf("----------------------------------------\n");
        if(op_alg > 1) printf("Config: Sel=%d | Cross=%d | Mut=%d\n", sel, cross, mut);
        if(op_alg == 1) printf("Config: Vizinhanca=%d\n", op_viz);
        printf(">> MELHOR VALOR: %.2f\n", melhor);
        printf(">> MEDIA VALORES:  %.2f\n", soma/NUM_REPETICOES);
        printf("----------------------------------------\n");

        printf("\nPressione ENTER para voltar ao menu...");
        while(getchar() != '\n'); // Limpa buffer anterior
        getchar(); // Espera pelo Enter real
        printf("\n\n");
    }

    return 0;
}