#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONTEUDO 200

struct node {
    int id;
    struct node *next;
};

struct ator {
    int id;
    char nome[100]; 
    struct node *filmes; 
};

struct filme {
    int id;
    char titulo[100]; 
    struct node *neighbors; 
};

char **ler_linhas(char *arquivo, int *num_linhas) {
    FILE *file = fopen(arquivo, "r");
    if (file == NULL) {
        printf("Erro ao abrir arquivo '%s'\n", arquivo);
        exit(EXIT_FAILURE);
    }
    int tamanho_atual = 1;
    char **linhas = (char **)malloc(tamanho_atual * sizeof(char *));
    if (linhas == NULL) {
        printf("Erro de alocação de memória");
        exit(EXIT_FAILURE);
    }
    char linha[CONTEUDO];
    *num_linhas = 0;
    while (fgets(linha, sizeof(linha), file) != NULL) {
        if (*num_linhas >= tamanho_atual) {
            tamanho_atual *= 2;
            linhas = (char **)realloc(linhas, tamanho_atual * sizeof(char *));
            if (linhas == NULL) {
                printf("Erro de realocação de memória");
                exit(EXIT_FAILURE);
            }
        }
        linhas[*num_linhas] = (char *)malloc((strlen(linha) + 1) * sizeof(char));
        if (linhas[*num_linhas] == NULL) {
            printf("Erro de alocação de memória");
            exit(EXIT_FAILURE);
        }
        strcpy(linhas[*num_linhas], linha);
        (*num_linhas)++;
    }
    fclose(file);
    return linhas;
}

int converter_id(char *id) {
    return atoi(id + 2);
}

int ler_atores(struct ator **atores, char **linhas, int num_linhas) {
    int num_atores = 0;
    *atores = malloc(num_linhas * sizeof(struct ator));
    if (*atores == NULL) {
        printf("Erro de alocação de memória");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < num_linhas; i++) {
        struct ator novoAtor;
        novoAtor.filmes = NULL;
        char *token = strtok(linhas[i], "\t");
        int num_tabs = 0;
        while (token != NULL) {
            switch (num_tabs) {
                case 0:
                    novoAtor.id = converter_id(token); 
                    break;
                case 1:
                    strncpy(novoAtor.nome, token, sizeof(novoAtor.nome) - 1);
                    novoAtor.nome[sizeof(novoAtor.nome) - 1] = '\0';
                    break;
                case 5: ;    
                    {
                        char *filme_token = strtok(token, ",");
                        while (filme_token != NULL) {
                            struct node *new_node = (struct node *)malloc(sizeof(struct node));
                            if (new_node == NULL) {
                                printf("Erro de alocação de memória");
                                exit(EXIT_FAILURE);
                            }
                            new_node->id = converter_id(filme_token); 
                            new_node->next = novoAtor.filmes;
                            novoAtor.filmes = new_node;
                            filme_token = strtok(NULL, ",");
                        }
                    }
                    break;
            }
            token = strtok(NULL, "\t");
            num_tabs++;
        }
        (*atores)[num_atores++] = novoAtor;
    }
    return num_atores;
}

int ler_filmes(struct filme **filmes, char **linhas, int num_linhas) {
    int num_filmes = 0;
    *filmes = (struct filme *)malloc(num_linhas * sizeof(struct filme));
    if (*filmes == NULL) {
        printf("Erro de alocação de memória");
        exit(EXIT_FAILURE);
    }
    for (int i = 1; i < num_linhas; i++) {
        struct filme novoFilme;
        novoFilme.neighbors = NULL;
        char *token = strtok(linhas[i], "\t");
        int num_tabs = 0;
        while (token != NULL) {
            switch (num_tabs) {
                case 0: // ID do filme
                    novoFilme.id = converter_id(token);
                    break;
                case 2: // Título do filme
                    strncpy(novoFilme.titulo, token, sizeof(novoFilme.titulo) - 1);
                    novoFilme.titulo[sizeof(novoFilme.titulo) - 1] = '\0';
                    break;
                default:
                    break;
            }
            num_tabs++;
            token = strtok(NULL, "\t");
        }
        (*filmes)[num_filmes++] = novoFilme;
    }
    return num_filmes;
}

void adicionar_aresta(struct filme *filmes, int num_filmes, int id1, int id2) {
    for (int i = 0; i < num_filmes; i++) {
        if (filmes[i].id == id1) {
            struct node *current = filmes[i].neighbors;
            int existe = 0;
            while (current != NULL) {
                if (current->id == id2) {
                    existe = 1;
                    break;
                }
                current = current->next;
            }
            if (!existe) {
                struct node *new_node = (struct node *)malloc(sizeof(struct node));
                if (new_node == NULL) {
                    printf("Erro de alocação de memória");
                    exit(EXIT_FAILURE);
                }
                new_node->id = id2;
                new_node->next = filmes[i].neighbors;
                filmes[i].neighbors = new_node;
            }
        } else if (filmes[i].id == id2) {
            struct node *current = filmes[i].neighbors;
            int existe = 0;
            while (current != NULL) {
                if (current->id == id1) {
                    existe = 1;
                    break;
                }
                current = current->next;
            }
            if (!existe) {
                struct node *new_node = (struct node *)malloc(sizeof(struct node));
                if (new_node == NULL) {
                    printf("Erro de alocação de memória");
                    exit(EXIT_FAILURE);
                }
                new_node->id = id1;
                new_node->next = filmes[i].neighbors;
                filmes[i].neighbors = new_node;
            }
        }
    }
}

void formar_clique(struct ator *atores, int num_atores, struct filme *filmes, int num_filmes) {
    for (int i = 0; i < num_atores; i++) {
        struct node *current = atores[i].filmes;
        while (current != NULL) {
            struct node *other = current->next;
            while (other != NULL) {
                adicionar_aresta(filmes, num_filmes, current->id, other->id);
                other = other->next;
            }
            current = current->next;
        }
    }
}

struct filme *buscar_filme(struct filme *filmes, int num_filmes, int id) {
    int esquerda = 0;
    int direita = num_filmes - 1;
    while (esquerda <= direita) {
        int meio = esquerda + (direita - esquerda) / 2;

        if (filmes[meio].id == id) {
            return &filmes[meio];
        }
        if (filmes[meio].id < id) {
            esquerda = meio + 1;
        } else {
            direita = meio - 1;
        }
    }

    return NULL;
}

void gerar_arquivo_dot(struct filme *filmes, int num_filmes) {
    FILE *file = fopen("output/input.dot", "w");
    if (file == NULL) {
        printf("Erro ao criar arquivo input.dot");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "graph { concentrate=true\n");

    for (int i = 0; i < num_filmes; i++) {
        struct node *current = filmes[i].neighbors;
        while (current != NULL) {
            if (filmes[i].id < current->id) {
                struct filme *filme_vizinho = buscar_filme(filmes, num_filmes, current->id);
                if (filme_vizinho != NULL) {
                    fprintf(file, "    \"%s\" -- \"%s\";\n", filmes[i].titulo, filme_vizinho->titulo);
                }
            }
            current = current->next;
        }
    }
    fprintf(file, "}\n");
    fclose(file);
}

int main() {
    char arquivo_atores[] = "name.basics.tsv";
    char arquivo_filmes[] = "title.basics.tsv";
    int num_linhas_atores, num_linhas_filmes;

    struct ator *atores;
    struct filme *filmes;

    char **linhas_atores = ler_linhas(arquivo_atores, &num_linhas_atores);
    char **linhas_filmes = ler_linhas(arquivo_filmes, &num_linhas_filmes);

    int num_atores = ler_atores(&atores, linhas_atores, num_linhas_atores);
    int num_filmes = ler_filmes(&filmes, linhas_filmes, num_linhas_filmes);

    formar_clique(atores, num_atores, filmes, num_filmes);
    gerar_arquivo_dot(filmes, num_filmes);

    for (int i = 0; i < num_linhas_atores; i++) {
        free(linhas_atores[i]);
    }
    free(linhas_atores);
    for (int i = 0; i < num_linhas_filmes; i++) {
        free(linhas_filmes[i]);
    }
    free(linhas_filmes);
    for (int i = 0; i < num_atores; i++) {
        struct node *current = atores[i].filmes;
        while (current != NULL) {
            struct node *temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(atores);
    for (int i = 0; i < num_filmes; i++) {
        struct node *current = filmes[i].neighbors;
        while (current != NULL) {
            struct node *temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(filmes);
    return 0;
}
