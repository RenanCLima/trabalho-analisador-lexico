// ======================================= IMPORTS =======================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ======================================= DEFINIÇÕES =======================================
#define MAX_SOURCE_SIZE 10000
#define MAX_TABELA_SIMBOLOS 1000
#define MAX_LEXEMA 256

// --------------------------------------- Enumeração de Tokens ---------------------------------------
typedef enum {
    // Palavras-chave
    KW_PROGRAM, KW_VAR, KW_INTEGER, KW_REAL, KW_BEGIN, KW_END,
    KW_IF, KW_THEN, KW_ELSE, KW_WHILE, KW_DO,
    
    // Operadores
    OP_ASS, OP_EQ, OP_LT, OP_GT, OP_LE, OP_GE, OP_NE,
    OP_AD, OP_MIN, OP_MUL, OP_DIV,
    
    // Símbolos
    SMB_SEM, SMB_COM, SMB_DOT, SMB_COLON, SMB_OPA, SMB_CPA, SMB_OBC, SMB_CBC,
    
    // Literais
    ID, NUM_INT, NUM_REAL,
    
    // Especiais
    END_TOKEN, ERROR_TOKEN
} TipoToken;

// --------------------------------------- Structs ---------------------------------------
typedef struct {
    TipoToken tipo;
    char *lexema;
    int linha;
    int coluna;
} Token;

typedef struct {
    const char *src;
    int i;
    int linha, coluna;
    char caractere;
} Scanner;

typedef struct {
    char lexema[MAX_LEXEMA];
    TipoToken tipo;
} EntradaTS;

typedef struct {
    EntradaTS entradas[MAX_TABELA_SIMBOLOS];
    int tamanho;
} TabelaSimbolos;

// --------------------------------------- Variáveis Globais ---------------------------------------
TabelaSimbolos tabela_simbolos;
Token token_atual;
FILE *arquivo_saida;
int erro_sintatico = 0;
int nivel_producao = 0;

// --------------------------------------- Declarações de Funções ---------------------------------------
// Léxico
char* nome_token(TipoToken t);
void inicializar_tabela_simbolos();
TipoToken buscar_tabela_simbolos(const char *lexema);
void adicionar_identificador_ts(const char *lexema);
void imprimir_tabela_simbolos(FILE *out);
char *str_ndup(const char *s, size_t n);
Token criar_token_texto(TipoToken tipo, const char *ini, size_t n, int lin, int col);
void iniciar(Scanner *sc, const char *texto, int linha_inicial);
void avancar(Scanner *sc);
void pular_espacos(Scanner *sc);
Token token_simples(Scanner *sc, TipoToken tipo);
Token token_erro_msg(Scanner *sc, const char *msg);
Token coletar_identificador(Scanner *sc);
Token coletar_numero(Scanner *sc);
Token coletar_operador(Scanner *sc);
Token coletar_string(Scanner *sc);
Token proximo_token(Scanner *sc);

// Sintático
void casa_token(TipoToken esperado, Scanner *sc);
void proximo_token_sintatico(Scanner *sc);
void erro_sintatico_msg(const char *msg);
void imprimir_producao(const char *regra);
void programa(Scanner *sc);
void bloco(Scanner *sc);
void parte_declaracao_variaveis(Scanner *sc);
void declaracao_variaveis(Scanner *sc);
void lista_identificadores(Scanner *sc);
void tipo(Scanner *sc);
void comando_composto(Scanner *sc);
void comando(Scanner *sc);
void atribuicao(Scanner *sc);
void comando_condicional(Scanner *sc);
void comando_repetitivo(Scanner *sc);
void expressao(Scanner *sc);
void relacao(Scanner *sc);
void expressao_simples(Scanner *sc);
void termo(Scanner *sc);
void fator(Scanner *sc);
void variavel(Scanner *sc);

// ======================================= IMPLEMENTAÇÃO LÉXICO =======================================

void inicializar_tabela_simbolos() {
    tabela_simbolos.tamanho = 0;
    const char* palavras[] = {"program", "var", "integer", "real", "begin", "end", "if", "then", "else", "while", "do"};
    TipoToken tipos[] = {KW_PROGRAM, KW_VAR, KW_INTEGER, KW_REAL, KW_BEGIN, KW_END, KW_IF, KW_THEN, KW_ELSE, KW_WHILE, KW_DO};
    
    for (int i = 0; i < 11; i++) {
        strcpy(tabela_simbolos.entradas[tabela_simbolos.tamanho].lexema, palavras[i]);
        tabela_simbolos.entradas[tabela_simbolos.tamanho].tipo = tipos[i];
        tabela_simbolos.tamanho++;
    }
}

TipoToken buscar_tabela_simbolos(const char *lexema) {
    for (int i = 0; i < tabela_simbolos.tamanho; i++) {
        if (strcasecmp(tabela_simbolos.entradas[i].lexema, lexema) == 0) {
            return tabela_simbolos.entradas[i].tipo;
        }
    }
    return ID;
}

void adicionar_identificador_ts(const char *lexema) {
    for (int i = 0; i < tabela_simbolos.tamanho; i++) {
        if (strcasecmp(tabela_simbolos.entradas[i].lexema, lexema) == 0) return;
    }
    if (tabela_simbolos.tamanho < MAX_TABELA_SIMBOLOS) {
        strcpy(tabela_simbolos.entradas[tabela_simbolos.tamanho].lexema, lexema);
        tabela_simbolos.entradas[tabela_simbolos.tamanho].tipo = ID;
        tabela_simbolos.tamanho++;
    } else {
        fprintf(stderr, "ERRO FATAL: Tabela de simbolos cheia!\n");
        exit(1);
    }
}

void imprimir_tabela_simbolos(FILE *out) {
    fprintf(out, "\n======= TABELA DE SIMBOLOS =======\n");
    for (int i = 0; i < tabela_simbolos.tamanho; i++) {
        fprintf(out, "%s - %s\n", tabela_simbolos.entradas[i].lexema, nome_token(tabela_simbolos.entradas[i].tipo));
    }
    fprintf(out, "==================================\n");
}

char* nome_token(TipoToken t) {
    switch(t) {
        case KW_PROGRAM: return "PROGRAM";
        case KW_VAR: return "VAR";
        case KW_INTEGER: return "INTEGER";
        case KW_REAL: return "REAL";
        case KW_BEGIN: return "BEGIN";
        case KW_END: return "END";
        case KW_IF: return "IF";
        case KW_THEN: return "THEN";
        case KW_ELSE: return "ELSE";
        case KW_WHILE: return "WHILE";
        case KW_DO: return "DO";
        case OP_ASS: return "OP_ASS";
        case OP_EQ: return "OP_EQ";
        case OP_LT: return "OP_LT";
        case OP_GT: return "OP_GT";
        case OP_LE: return "OP_LE";
        case OP_GE: return "OP_GE";
        case OP_NE: return "OP_NE";
        case OP_AD: return "OP_AD";
        case OP_MIN: return "OP_MIN";
        case OP_MUL: return "OP_MUL";
        case OP_DIV: return "OP_DIV";
        case SMB_SEM: return "SMB_SEM";
        case SMB_COM: return "SMB_COM";
        case SMB_DOT: return "SMB_DOT";
        case SMB_COLON: return "SMB_COLON";
        case SMB_OPA: return "SMB_OPA";
        case SMB_CPA: return "SMB_CPA";
        case SMB_OBC: return "SMB_OBC";
        case SMB_CBC: return "SMB_CBC";
        case ID: return "ID";
        case NUM_INT: return "NUM_INT";
        case NUM_REAL: return "NUM_REAL";
        case END_TOKEN: return "EOF";
        case ERROR_TOKEN: return "ERROR";
        default: return "?";
    }
}

char *str_ndup(const char *s, size_t n) {
    char *p = (char*)malloc(n + 1);
    if (!p) { fprintf(stderr, "Memoria insuficiente\n"); exit(1); }
    memcpy(p, s, n); p[n] = '\0';
    return p;
}

Token criar_token_texto(TipoToken tipo, const char *ini, size_t n, int lin, int col) {
    Token t;
    t.tipo = tipo;
    t.lexema = str_ndup(ini, n);
    t.linha = lin;
    t.coluna = col;
    return t;
}

void iniciar(Scanner *sc, const char *texto, int linha_inicial) {
    sc->src = texto ? texto : "";
    sc->linha = linha_inicial;
    sc->coluna = 1;
    sc->i = 0;
    sc->caractere = sc->src[0];
}

void avancar(Scanner *sc) {
    if (sc->caractere == '\0') return;
    if (sc->caractere == '\n') { sc->linha++; sc->coluna = 1; }
    else { sc->coluna++; }
    sc->i++;
    sc->caractere = sc->src[sc->i];
}

void pular_espacos(Scanner *sc) {
    while (isspace((unsigned char)sc->caractere)) avancar(sc);
}

Token token_simples(Scanner *sc, TipoToken tipo) {
    int lin = sc->linha, col = sc->coluna;
    char ch = sc->caractere;
    avancar(sc);
    return criar_token_texto(tipo, &ch, 1, lin, col);
}

Token token_erro_msg(Scanner *sc, const char *msg) {
    return criar_token_texto(ERROR_TOKEN, msg, strlen(msg), sc->linha, sc->coluna);
}

Token coletar_identificador(Scanner *sc) {
    int lin = sc->linha, col = sc->coluna;
    size_t ini = sc->i;
    while (isalnum((unsigned char)sc->caractere)) avancar(sc);
    size_t len = sc->i - ini;
    char lexema_temp[MAX_LEXEMA];
    strncpy(lexema_temp, sc->src + ini, len);
    lexema_temp[len] = '\0';
    for (size_t i = 0; i < len; i++) {
        lexema_temp[i] = (char)tolower((unsigned char)lexema_temp[i]);
    }
    TipoToken tipo = buscar_tabela_simbolos(lexema_temp);
    if (tipo == ID) adicionar_identificador_ts(lexema_temp);
    return criar_token_texto(tipo, sc->src + ini, len, lin, col);
}

Token coletar_numero(Scanner *sc) {
    int lin = sc->linha, col = sc->coluna;
    size_t ini = sc->i;
    int tem_ponto = 0;
    while (isdigit((unsigned char)sc->caractere) || sc->caractere == '.') {
        if (sc->caractere == '.') {
            if (tem_ponto) break;
            tem_ponto = 1;
        }
        avancar(sc);
    }
    size_t len = sc->i - ini;
    TipoToken tipo = tem_ponto ? NUM_REAL : NUM_INT;
    return criar_token_texto(tipo, sc->src + ini, len, lin, col);
}

Token coletar_operador(Scanner *sc) {
    int lin = sc->linha, col = sc->coluna;
    char c = sc->caractere;
    avancar(sc);
    if (c == ':' && sc->caractere == '=') {
        avancar(sc);
        return criar_token_texto(OP_ASS, ":=", 2, lin, col);
    }
    if (c == '<') {
        if (sc->caractere == '=') { avancar(sc); return criar_token_texto(OP_LE, "<=", 2, lin, col); }
        if (sc->caractere == '>') { avancar(sc); return criar_token_texto(OP_NE, "<>", 2, lin, col); }
        return criar_token_texto(OP_LT, "<", 1, lin, col);
    }
    if (c == '>') {
        if (sc->caractere == '=') { avancar(sc); return criar_token_texto(OP_GE, ">=", 2, lin, col); }
        return criar_token_texto(OP_GT, ">", 1, lin, col);
    }
    if (c == '=') return criar_token_texto(OP_EQ, "=", 1, lin, col);
    if (c == ':') return criar_token_texto(SMB_COLON, ":", 1, lin, col);
    return token_erro_msg(sc, "Operador invalido");
}

Token coletar_string(Scanner *sc) {
    int lin = sc->linha, col = sc->coluna;
    size_t ini = sc->i;
    avancar(sc);
    while (sc->caractere != '\0' && sc->caractere != '"' && sc->caractere != '\n') {
        avancar(sc);
    }
    if (sc->caractere == '\n' || sc->caractere == '\0') {
        return token_erro_msg(sc, "String nao-fechada");
    }
    avancar(sc);
    size_t len = sc->i - ini;
    return criar_token_texto(ERROR_TOKEN, sc->src + ini, len, lin, col);
}

Token proximo_token(Scanner *sc) {
    pular_espacos(sc);
    if (sc->caractere == '\0') return criar_token_texto(END_TOKEN, "", 0, sc->linha, sc->coluna);
    if (isalpha((unsigned char)sc->caractere)) return coletar_identificador(sc);
    if (isdigit((unsigned char)sc->caractere)) return coletar_numero(sc);
    if (sc->caractere == '"') return coletar_string(sc);
    if (strchr(":<>=", sc->caractere)) return coletar_operador(sc);

    switch (sc->caractere) {
        case '+': return token_simples(sc, OP_AD);
        case '-': return token_simples(sc, OP_MIN);
        case '*': return token_simples(sc, OP_MUL);
        case '/': return token_simples(sc, OP_DIV);
        case ';': return token_simples(sc, SMB_SEM);
        case ',': return token_simples(sc, SMB_COM);
        case '.': return token_simples(sc, SMB_DOT);
        case '(': return token_simples(sc, SMB_OPA);
        case ')': return token_simples(sc, SMB_CPA);
        case '{': return token_simples(sc, SMB_OBC);
        case '}': return token_simples(sc, SMB_CBC);
        default: {
            char msg[64];
            snprintf(msg, sizeof(msg), "Caractere invalido: '%c'", sc->caractere);
            avancar(sc);
            return token_erro_msg(sc, msg);
        }
    }
}

// ======================================= IMPLEMENTAÇÃO SINTÁTICA =======================================

void imprimir_producao(const char *regra) {
    for (int i = 0; i < nivel_producao; i++) fprintf(arquivo_saida, "  ");
    fprintf(arquivo_saida, "%s\n", regra);
}

void erro_sintatico_msg(const char *msg) {
    fprintf(arquivo_saida, "\nERRO SINTATICO:\n");
    fprintf(arquivo_saida, "%d: %s\n", token_atual.linha, msg);
    fprintf(stderr, "\nERRO SINTATICO:\n");
    fprintf(stderr, "%d: %s\n", token_atual.linha, msg);
    erro_sintatico = 1;
}

void proximo_token_sintatico(Scanner *sc) {
    if (token_atual.lexema) free(token_atual.lexema);
    token_atual = proximo_token(sc);
}

void casa_token(TipoToken esperado, Scanner *sc) {
    if (erro_sintatico) return;
    if (token_atual.tipo == esperado) {
        fprintf(arquivo_saida, "Token casado: <%s, %s> na linha %d\n", 
                nome_token(token_atual.tipo), token_atual.lexema, token_atual.linha);
        proximo_token_sintatico(sc);
    } else if (token_atual.tipo == END_TOKEN) {
        erro_sintatico_msg("fim de arquivo nao esperado.");
    } else {
        char msg[256];
        snprintf(msg, sizeof(msg), "token nao esperado [%s]. Esperado: %s", 
                 token_atual.lexema, nome_token(esperado));
        erro_sintatico_msg(msg);
    }
}

void programa(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<programa> ::= program <identificador> ; <bloco> .");
    nivel_producao++;
    casa_token(KW_PROGRAM, sc);
    casa_token(ID, sc);
    casa_token(SMB_SEM, sc);
    bloco(sc);
    casa_token(SMB_DOT, sc);
    nivel_producao--;
}

void bloco(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<bloco> ::= <parte de declaracoes de variaveis> <comando composto>");
    nivel_producao++;
    parte_declaracao_variaveis(sc);
    comando_composto(sc);
    nivel_producao--;
}

void parte_declaracao_variaveis(Scanner *sc) {
    if (erro_sintatico) return;
    
    // Gramática: <parte de declarações de variáveis> ::= { var <declaração de variáveis> {; <declaração de variáveis>}; }
    if (token_atual.tipo == KW_VAR) {
        imprimir_producao("<parte de declaracoes de variaveis> ::= var <declaração de variáveis> {; <declaração de variáveis>};");
        nivel_producao++;
        
        // Loop para aceitar multiplos blocos 'var' (caso existam, devido as chaves externas da gramatica)
        // Mas geralmente Pascal tem um só. Seguiremos a lógica:
        while (token_atual.tipo == KW_VAR && !erro_sintatico) {
            casa_token(KW_VAR, sc);
            
            // Primeira declaração obrigatória
            declaracao_variaveis(sc);
            casa_token(SMB_SEM, sc); // Ponto e vírgula obrigatório após declaração
            
            // Repetição de outras declarações
            while (token_atual.tipo == ID && !erro_sintatico) {
                declaracao_variaveis(sc);
                casa_token(SMB_SEM, sc); // Ponto e vírgula obrigatório após declaração
            }
        }
        nivel_producao--;
    } else {
        imprimir_producao("<parte de declaracoes de variaveis> ::= <vazio>");
    }
}

void declaracao_variaveis(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<declaracao de variaveis> ::= <lista de identificadores> : <tipo>");
    nivel_producao++;
    lista_identificadores(sc);
    casa_token(SMB_COLON, sc);
    tipo(sc);
    nivel_producao--;
}

void lista_identificadores(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<lista de identificadores> ::= <identificador> { , <identificador> }");
    nivel_producao++;
    casa_token(ID, sc);
    while (token_atual.tipo == SMB_COM && !erro_sintatico) {
        casa_token(SMB_COM, sc);
        casa_token(ID, sc);
    }
    nivel_producao--;
}

void tipo(Scanner *sc) {
    if (erro_sintatico) return;
    if (token_atual.tipo == KW_INTEGER) {
        imprimir_producao("<tipo> ::= integer");
        nivel_producao++;
        casa_token(KW_INTEGER, sc);
        nivel_producao--;
    } else if (token_atual.tipo == KW_REAL) {
        imprimir_producao("<tipo> ::= real");
        nivel_producao++;
        casa_token(KW_REAL, sc);
        nivel_producao--;
    } else {
        erro_sintatico_msg("tipo esperado (integer ou real)");
    }
}

void comando_composto(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<comando composto> ::= begin <comando> ; { <comando> ; } end");
    nivel_producao++;
    casa_token(KW_BEGIN, sc);
    
    // Gramática exige ao menos um comando: <comando>;
    comando(sc);
    casa_token(SMB_SEM, sc);
    
    // Loop para comandos subsequentes
    while (token_atual.tipo != KW_END && token_atual.tipo != END_TOKEN && !erro_sintatico) {
        comando(sc);
        casa_token(SMB_SEM, sc);
    }
    casa_token(KW_END, sc);
    nivel_producao--;
}

void comando(Scanner *sc) {
    if (erro_sintatico) return;
    if (token_atual.tipo == ID) {
        imprimir_producao("<comando> ::= <atribuicao>");
        nivel_producao++; atribuicao(sc); nivel_producao--;
    } else if (token_atual.tipo == KW_BEGIN) {
        imprimir_producao("<comando> ::= <comando composto>");
        nivel_producao++; comando_composto(sc); nivel_producao--;
    } else if (token_atual.tipo == KW_IF) {
        imprimir_producao("<comando> ::= <comando condicional>");
        nivel_producao++; comando_condicional(sc); nivel_producao--;
    } else if (token_atual.tipo == KW_WHILE) {
        imprimir_producao("<comando> ::= <comando repetitivo>");
        nivel_producao++; comando_repetitivo(sc); nivel_producao--;
    } else {
        erro_sintatico_msg("comando esperado");
    }
}

void atribuicao(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<atribuicao> ::= <variavel> := <expressao>");
    nivel_producao++;
    variavel(sc);
    casa_token(OP_ASS, sc);
    expressao(sc);
    nivel_producao--;
}

void comando_condicional(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<comando condicional> ::= if <expressao> then <comando> [ else <comando> ]");
    nivel_producao++;
    casa_token(KW_IF, sc);
    expressao(sc);
    casa_token(KW_THEN, sc);
    comando(sc);
    if (token_atual.tipo == KW_ELSE) {
        casa_token(KW_ELSE, sc);
        comando(sc);
    }
    nivel_producao--;
}

void comando_repetitivo(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<comando repetitivo> ::= while <expressao> do <comando>");
    nivel_producao++;
    casa_token(KW_WHILE, sc);
    expressao(sc);
    casa_token(KW_DO, sc);
    comando(sc);
    nivel_producao--;
}

void expressao(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<expressao> ::= <expressao simples> [ <relacao> <expressao simples> ]");
    nivel_producao++;
    expressao_simples(sc);
    if (token_atual.tipo == OP_EQ || token_atual.tipo == OP_NE || 
        token_atual.tipo == OP_LT || token_atual.tipo == OP_LE || 
        token_atual.tipo == OP_GT || token_atual.tipo == OP_GE) {
        relacao(sc);
        expressao_simples(sc);
    }
    nivel_producao--;
}

void relacao(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<relacao> ::= = | <> | < | <= | >= | >");
    nivel_producao++;
    if (token_atual.tipo == OP_EQ) casa_token(OP_EQ, sc);
    else if (token_atual.tipo == OP_NE) casa_token(OP_NE, sc);
    else if (token_atual.tipo == OP_LT) casa_token(OP_LT, sc);
    else if (token_atual.tipo == OP_LE) casa_token(OP_LE, sc);
    else if (token_atual.tipo == OP_GT) casa_token(OP_GT, sc);
    else if (token_atual.tipo == OP_GE) casa_token(OP_GE, sc);
    nivel_producao--;
}

void expressao_simples(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<expressao simples> ::= [ + | - ] <termo> { ( + | - ) <termo> }");
    nivel_producao++;
    if (token_atual.tipo == OP_AD || token_atual.tipo == OP_MIN) {
        if (token_atual.tipo == OP_AD) casa_token(OP_AD, sc);
        else casa_token(OP_MIN, sc);
    }
    termo(sc);
    while ((token_atual.tipo == OP_AD || token_atual.tipo == OP_MIN) && !erro_sintatico) {
        if (token_atual.tipo == OP_AD) casa_token(OP_AD, sc);
        else casa_token(OP_MIN, sc);
        termo(sc);
    }
    nivel_producao--;
}

void termo(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<termo> ::= <fator> { ( * | / ) <fator> }");
    nivel_producao++;
    fator(sc);
    while ((token_atual.tipo == OP_MUL || token_atual.tipo == OP_DIV) && !erro_sintatico) {
        if (token_atual.tipo == OP_MUL) casa_token(OP_MUL, sc);
        else casa_token(OP_DIV, sc);
        fator(sc);
    }
    nivel_producao--;
}

void fator(Scanner *sc) {
    if (erro_sintatico) return;
    if (token_atual.tipo == ID) {
        imprimir_producao("<fator> ::= <variavel>");
        nivel_producao++; variavel(sc); nivel_producao--;
    } else if (token_atual.tipo == NUM_INT || token_atual.tipo == NUM_REAL) {
        imprimir_producao("<fator> ::= <numero>");
        nivel_producao++;
        if (token_atual.tipo == NUM_INT) casa_token(NUM_INT, sc);
        else casa_token(NUM_REAL, sc);
        nivel_producao--;
    } else if (token_atual.tipo == SMB_OPA) {
        imprimir_producao("<fator> ::= ( <expressao> )");
        nivel_producao++;
        casa_token(SMB_OPA, sc);
        expressao(sc);
        casa_token(SMB_CPA, sc);
        nivel_producao--;
    } else {
        erro_sintatico_msg("fator esperado (variavel, numero ou expressao)");
    }
}

void variavel(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<variavel> ::= <identificador>");
    nivel_producao++;
    casa_token(ID, sc);
    nivel_producao--;
}

// ======================================= MAIN =======================================
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <arquivo_entrada.pas> <arquivo_saida.txt>\n", argv[0]);
        return 1;
    }

    inicializar_tabela_simbolos();

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        printf("Erro ao abrir arquivo de entrada: %s\n", argv[1]);
        return 1;
    }

    char *codigo = (char*)malloc(MAX_SOURCE_SIZE);
    if (!codigo) { printf("Erro de alocacao de memoria.\n"); return 1; }
    
    size_t total = 0, lido;
    while ((lido = fread(codigo + total, 1, MAX_SOURCE_SIZE - total - 1, fp)) > 0) {
        total += lido;
    }
    codigo[total] = '\0';
    fclose(fp);

    arquivo_saida = fopen(argv[2], "w");
    if (!arquivo_saida) {
        printf("Erro ao criar arquivo de saida: %s\n", argv[2]);
        free(codigo);
        return 1;
    }

    fprintf(arquivo_saida, "======================================\n");
    fprintf(arquivo_saida, "   ANALISE SINTATICA - MICROPASCAL    \n");
    fprintf(arquivo_saida, "======================================\n\n");
    fprintf(arquivo_saida, "=== DERIVACOES E REGRAS DE PRODUCAO ===\n\n");

    Scanner sc;
    iniciar(&sc, codigo, 1);
    token_atual.lexema = NULL;
    proximo_token_sintatico(&sc); // Lê o primeiro token

    programa(&sc);

    if (!erro_sintatico && token_atual.tipo != END_TOKEN) {
        erro_sintatico_msg("fim de arquivo esperado (tokens sobraram).");
    }

    if (!erro_sintatico) {
        fprintf(arquivo_saida, "\n\n=== ANALISE SINTATICA CONCLUIDA COM SUCESSO ===\n");
        printf("Analise sintatica concluida com sucesso!\n");
    } else {
        fprintf(arquivo_saida, "\n\n=== ANALISE SINTATICA CONCLUIDA COM ERROS ===\n");
        printf("Analise sintatica encontrou erros. Verifique: %s\n", argv[2]);
    }

    imprimir_tabela_simbolos(arquivo_saida);

    if (token_atual.lexema) free(token_atual.lexema);
    free(codigo);
    fclose(arquivo_saida);
    
    printf("Resultado detalhado salvo em '%s'\n", argv[2]);
    return erro_sintatico ? 1 : 0;
}