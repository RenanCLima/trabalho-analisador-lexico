// Para rodar o código no terminal BASH primeiro utilize: gcc lexico.c -o lexico.exe
// Em seguida utilize no terminal: ./lexico.exe arquivo_entrada.pas arquivo_saida.lex
// ======================================= imports =======================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ======================================= Definições =======================================
#define MAX_SOURCE_SIZE 10000
#define MAX_TABELA_SIMBOLOS 1000
#define MAX_LEXEMA 256

// --------------------------------------- Definições de Variáveis ---------------------------------------
typedef enum {
    // Palavras-chave
    KW_PROGRAM, //  program
    KW_VAR,     //  var
    KW_INTEGER, //  integer
    KW_REAL,    //  real
    KW_BEGIN,   //  begin
    KW_END,     //  end
    KW_IF,      //  if
    KW_THEN,    //  then
    KW_ELSE,    //  else
    KW_WHILE,   //  while
    KW_DO,      //  do
    
    // Operadores (conforme especificação)
    OP_ASS,     //  := (atribuição)
    OP_EQ,      //  =  (igual)
    OP_LT,      //  <  (menor)
    OP_GT,      //  >  (maior)
    OP_LE,      //  <= (menor igual)
    OP_GE,      //  >= (maior igual)
    OP_NE,      //  <> (diferente)
    OP_AD,      //  +  (soma)
    OP_MIN,     //  -  (subtração)
    OP_MUL,     //  *  (multiplicação)
    OP_DIV,     //  /  (divisão)
    
    // Símbolos (conforme especificação)
    SMB_SEM,    //  ; (ponto e vírgula)
    SMB_COM,    //  , (vírgula)
    SMB_DOT,    //  . (ponto)
    SMB_COLON,  //  : (dois pontos)
    SMB_OPA,    //  ( (abre parênteses)
    SMB_CPA,    //  ) (fecha parênteses)
    SMB_OBC,    //  { (abre chaves)
    SMB_CBC,    //  } (fecha chaves)
    
    // Literais e identificadores
    ID,         //  Identificador
    NUM_INT,    //  Número inteiro
    NUM_REAL,   //  Número Real
    
    // Especiais
    END_TOKEN,      //  final do código
    ERROR_TOKEN     //  erro do código
} TipoToken;

// --------------------------------------- Declarações de Funções ---------------------------------------
char* nome_token(TipoToken t);

// --------------------------------------- Definições de structs ---------------------------------------
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

// --------------------------------------- Tabela de Símbolos ---------------------------------------
typedef struct {
    char lexema[MAX_LEXEMA];
    TipoToken tipo;
} EntradaTS;

typedef struct {
    EntradaTS entradas[MAX_TABELA_SIMBOLOS];
    int tamanho;
} TabelaSimbolos;

// Inicializa a tabela de símbolos com palavras reservadas
TabelaSimbolos tabela_simbolos;

void inicializar_tabela_simbolos() {
    tabela_simbolos.tamanho = 0;
    
    // Adiciona palavras reservadas
    strcpy(tabela_simbolos.entradas[tabela_simbolos.tamanho].lexema, "program");
    tabela_simbolos.entradas[tabela_simbolos.tamanho].tipo = KW_PROGRAM;
    tabela_simbolos.tamanho++;
    
    strcpy(tabela_simbolos.entradas[tabela_simbolos.tamanho].lexema, "var");
    tabela_simbolos.entradas[tabela_simbolos.tamanho].tipo = KW_VAR;
    tabela_simbolos.tamanho++;
    
    strcpy(tabela_simbolos.entradas[tabela_simbolos.tamanho].lexema, "integer");
    tabela_simbolos.entradas[tabela_simbolos.tamanho].tipo = KW_INTEGER;
    tabela_simbolos.tamanho++;
    
    strcpy(tabela_simbolos.entradas[tabela_simbolos.tamanho].lexema, "real");
    tabela_simbolos.entradas[tabela_simbolos.tamanho].tipo = KW_REAL;
    tabela_simbolos.tamanho++;
    
    strcpy(tabela_simbolos.entradas[tabela_simbolos.tamanho].lexema, "begin");
    tabela_simbolos.entradas[tabela_simbolos.tamanho].tipo = KW_BEGIN;
    tabela_simbolos.tamanho++;
    
    strcpy(tabela_simbolos.entradas[tabela_simbolos.tamanho].lexema, "end");
    tabela_simbolos.entradas[tabela_simbolos.tamanho].tipo = KW_END;
    tabela_simbolos.tamanho++;
    
    strcpy(tabela_simbolos.entradas[tabela_simbolos.tamanho].lexema, "if");
    tabela_simbolos.entradas[tabela_simbolos.tamanho].tipo = KW_IF;
    tabela_simbolos.tamanho++;
    
    strcpy(tabela_simbolos.entradas[tabela_simbolos.tamanho].lexema, "then");
    tabela_simbolos.entradas[tabela_simbolos.tamanho].tipo = KW_THEN;
    tabela_simbolos.tamanho++;
    
    strcpy(tabela_simbolos.entradas[tabela_simbolos.tamanho].lexema, "else");
    tabela_simbolos.entradas[tabela_simbolos.tamanho].tipo = KW_ELSE;
    tabela_simbolos.tamanho++;
    
    strcpy(tabela_simbolos.entradas[tabela_simbolos.tamanho].lexema, "while");
    tabela_simbolos.entradas[tabela_simbolos.tamanho].tipo = KW_WHILE;
    tabela_simbolos.tamanho++;
    
    strcpy(tabela_simbolos.entradas[tabela_simbolos.tamanho].lexema, "do");
    tabela_simbolos.entradas[tabela_simbolos.tamanho].tipo = KW_DO;
    tabela_simbolos.tamanho++;
}

// Busca na tabela de símbolos
TipoToken buscar_tabela_simbolos(const char *lexema) {
    for (int i = 0; i < tabela_simbolos.tamanho; i++) {
        if (strcasecmp(tabela_simbolos.entradas[i].lexema, lexema) == 0) {
            return tabela_simbolos.entradas[i].tipo;
        }
    }
    return ID; // Se não encontrou, é identificador
}

// Adiciona identificador na tabela de símbolos
void adicionar_identificador_ts(const char *lexema) {
    // Verifica se já existe
    for (int i = 0; i < tabela_simbolos.tamanho; i++) {
        if (strcasecmp(tabela_simbolos.entradas[i].lexema, lexema) == 0) {
            return; // Já existe, não adiciona
        }
    }
    
    // Adiciona novo identificador
    if (tabela_simbolos.tamanho < MAX_TABELA_SIMBOLOS) {
        strcpy(tabela_simbolos.entradas[tabela_simbolos.tamanho].lexema, lexema);
        tabela_simbolos.entradas[tabela_simbolos.tamanho].tipo = ID;
        tabela_simbolos.tamanho++;
    }
}

// Imprime tabela de símbolos
void imprimir_tabela_simbolos(FILE *out) {
    fprintf(out, "\n======= TABELA DE SÍMBOLOS =======\n");
    for (int i = 0; i < tabela_simbolos.tamanho; i++) {
        fprintf(out, "%s - %s\n", 
                tabela_simbolos.entradas[i].lexema,
                nome_token(tabela_simbolos.entradas[i].tipo));
    }
    fprintf(out, "==================================\n\n");
}

// ======================================= Métodos =======================================
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

// Cria uma cópia da String e armazena a cópia em um espaço na memória (Será o token)
char *str_ndup(const char *s, size_t n) {
    char *p = (char*)malloc(n + 1);
    if (!p) { fprintf(stderr, "Memória insuficiente\n"); exit(1); }
    memcpy(p, s, n); p[n] = '\0';
    return p;
}

// Cria um Token
Token criar_token_texto(TipoToken tipo, const char *ini, size_t n, int lin, int col) {
    Token t;
    t.tipo = tipo;
    t.lexema = str_ndup(ini, n);
    t.linha = lin;
    t.coluna = col;
    return t;
}

// Inicia um Scanner que analisa o token como um todo, a linha atual, coluna atual e o caractere do token atual.
void iniciar(Scanner *sc, const char *texto, int linha_inicial) {
    sc->src = texto ? texto : "";
    sc->linha = linha_inicial;
    sc->coluna = 1;
    sc->i = 0;
    sc->caractere = sc->src[0];
}

// Avança o scanner para a direita, caso tenha um \n ele vai para a linha seguinte e volta a coluna 1. Caso encontre um \0  ele finaliza o Token
void avancar(Scanner *sc) {
    if (sc->caractere == '\0') return;
    if (sc->caractere == '\n') { sc->linha++; sc->coluna = 1; }
    else { sc->coluna++; };
    sc->i++;
    sc->caractere = sc->src[sc->i];
}

// Pula espaços em branco
void pular_espacos(Scanner *sc) {
    while (isspace((unsigned char)sc->caractere)) avancar(sc);
}


// ------------------------------------------- Métodos de Tokens -------------------------------------------
// Cria um token utilizando como ponteiro a linha e coluna do primeiro caractere do Token
Token token_simples(Scanner *sc, TipoToken tipo) {
    int lin = sc->linha, col = sc->coluna;
    char ch = sc->caractere;
    avancar(sc);
    return criar_token_texto(tipo, &ch, 1, lin, col);
}

// Cria um token de erro e retorna a linha e coluna onde encontrou o erro
Token token_erro_msg(Scanner *sc, const char *msg) {
    return criar_token_texto(ERROR_TOKEN, msg, strlen(msg), sc->linha, sc->coluna);
}

// Verifica se a palavra é um Token específico ou um Identifier
TipoToken verificar_palavra_chave(const char *lexema) {
    return buscar_tabela_simbolos(lexema);
}

// Coleta identificador e adiciona na tabela de símbolos se necessário
Token coletar_identificador(Scanner *sc) {
    int lin = sc->linha, col = sc->coluna;
    size_t ini = sc->i;
    while (isalnum((unsigned char)sc->caractere)) avancar(sc);
    size_t len = sc->i - ini;

    // Cria uma cópia do lexema
    char lexema_temp[MAX_LEXEMA];
    strncpy(lexema_temp, sc->src + ini, len);
    lexema_temp[len] = '\0';

    // Converte para minúsculo para garantir case insensitive
    for (size_t i = 0; i < len; i++) {
        lexema_temp[i] = (char)tolower((unsigned char)lexema_temp[i]);
    }
    TipoToken tipo = buscar_tabela_simbolos(lexema_temp);
    
    // Se for identificador, adiciona na tabela de símbolos
    if (tipo == ID) {
        adicionar_identificador_ts(lexema_temp);
    }
    return criar_token_texto(tipo, sc->src + ini, len, lin, col);
}


// Token número, verifica se tem um . ou não, assim definindo se é um número REAL ou número INT
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

// Token operador, verifica o tipo de caractere e define seu tipo
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
    return token_erro_msg(sc, "Operador inválido");
}

// Tratamento de strings (detecção de erro de string não-fechada)
Token coletar_string(Scanner *sc) {
    int lin = sc->linha, col = sc->coluna;
    size_t ini = sc->i;
    avancar(sc); // Pula a primeira aspas
    
    while (sc->caractere != '\0' && sc->caractere != '"' && sc->caractere != '\n') {
        avancar(sc);
    }
    
    if (sc->caractere == '\n' || sc->caractere == '\0') {
        return token_erro_msg(sc, "String não-fechada antes de quebra de linha");
    }
    
    avancar(sc); // Pula a última aspas
    size_t len = sc->i - ini;
    return criar_token_texto(ERROR_TOKEN, sc->src + ini, len, lin, col);
}

// Busca o próximo Token, caso encontre um símbolo, define um Token
Token proximo_token(Scanner *sc) {
    pular_espacos(sc);
    if (sc->caractere == '\0') return criar_token_texto(END_TOKEN, "", 0, sc->linha, sc->coluna);
    if (isalpha((unsigned char)sc->caractere)) return coletar_identificador(sc);
    if (isdigit((unsigned char)sc->caractere))
        return coletar_numero(sc);
    if (sc->caractere == '"') return coletar_string(sc);

    if (strchr(":<>=", sc->caractere))
        return coletar_operador(sc);

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
            snprintf(msg, sizeof(msg), "Caractere inválido: '%c'", sc->caractere);
            avancar(sc);
            return token_erro_msg(sc, msg);
        }
    }
}

// --------------------------------------- Variáveis Globais para Análise Sintática ---------------------------------------
Token token_atual;
FILE *arquivo_saida;
int erro_sintatico = 0;
int nivel_producao = 0;

// --------------------------------------- Declarações de Funções Sintáticas ---------------------------------------
void casa_token(TipoToken esperado, Scanner *sc);
void proximo_token_sintatico(Scanner *sc);
void erro_sintatico_msg(const char *msg);
void imprimir_producao(const char *regra);

// Funções para cada não-terminal da gramática
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

// ======================================= Funções de Análise Sintática =======================================

// Imprime regra de produção com indentação
void imprimir_producao(const char *regra) {
    for (int i = 0; i < nivel_producao; i++) {
        fprintf(arquivo_saida, "  ");
    }
    fprintf(arquivo_saida, "%s\n", regra);
}

// Reporta erro sintático
void erro_sintatico_msg(const char *msg) {
    fprintf(arquivo_saida, "\nERRO SINTÁTICO:\n");
    fprintf(arquivo_saida, "%d: %s\n", token_atual.linha, msg);
    fprintf(stderr, "\nERRO SINTÁTICO:\n");
    fprintf(stderr, "%d: %s\n", token_atual.linha, msg);
    erro_sintatico = 1;
}

// Lê o próximo token
void proximo_token_sintatico(Scanner *sc) {
    if (token_atual.lexema) {
        free(token_atual.lexema);
    }
    token_atual = proximo_token(sc);
}

// Casa o token esperado com o token atual
void casa_token(TipoToken esperado, Scanner *sc) {
    if (erro_sintatico) return;
    
    if (token_atual.tipo == esperado) {
        fprintf(arquivo_saida, "Token casado: <%s, %s> na linha %d\n", 
                nome_token(token_atual.tipo), token_atual.lexema, token_atual.linha);
        proximo_token_sintatico(sc);
    } else if (token_atual.tipo == END_TOKEN) {
        erro_sintatico_msg("fim de arquivo não esperado.");
    } else {
        char msg[256];
        snprintf(msg, sizeof(msg), "token não esperado [%s]. Esperado: %s", 
                 token_atual.lexema, nome_token(esperado));
        erro_sintatico_msg(msg);
    }
}

// <programa> ::= program <identificador> ; <bloco> .
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

// <bloco> ::= <parte de declarações de variáveis> <comando composto>
void bloco(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<bloco> ::= <parte de declarações de variáveis> <comando composto>");
    nivel_producao++;
    
    parte_declaracao_variaveis(sc);
    comando_composto(sc);
    
    nivel_producao--;
}

// <parte de declarações de variáveis> ::= { var <declaração de variáveis> {; <declaração de variáveis>} ; }
void parte_declaracao_variaveis(Scanner *sc) {
    if (erro_sintatico) return;
    
    if (token_atual.tipo == KW_VAR) {
        imprimir_producao("<parte de declarações de variáveis> ::= var <declaração de variáveis> {; <declaração de variáveis>} ;");
        nivel_producao++;
        
        casa_token(KW_VAR, sc);
        declaracao_variaveis(sc);
        
        while (token_atual.tipo == SMB_SEM && !erro_sintatico) {
            casa_token(SMB_SEM, sc);
            if (token_atual.tipo == ID) {
                declaracao_variaveis(sc);
            } else {
                break;
            }
        }
        
        nivel_producao--;
    } else {
        imprimir_producao("<parte de declarações de variáveis> ::= <vazio>");
    }
}

// <declaração de variáveis> ::= <lista de identificadores> : <tipo>
void declaracao_variaveis(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<declaração de variáveis> ::= <lista de identificadores> : <tipo>");
    nivel_producao++;
    
    lista_identificadores(sc);
    casa_token(SMB_COLON, sc);
    tipo(sc);
    
    nivel_producao--;
}

// <lista de identificadores> ::= <identificador> { , <identificador> }
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

// <tipo> ::= integer | real
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

// <comando composto> ::= begin <comando> ; { <comando> ; } end
void comando_composto(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<comando composto> ::= begin <comando> ; { <comando> ; } end");
    nivel_producao++;
    
    casa_token(KW_BEGIN, sc);
    comando(sc);
    casa_token(SMB_SEM, sc);
    
    while (token_atual.tipo != KW_END && token_atual.tipo != END_TOKEN && !erro_sintatico) {
        comando(sc);
        casa_token(SMB_SEM, sc);
    }
    
    casa_token(KW_END, sc);
    
    nivel_producao--;
}

// <comando> ::= <atribuição> | <comando composto> | <comando condicional> | <comando repetitivo>
void comando(Scanner *sc) {
    if (erro_sintatico) return;
    
    if (token_atual.tipo == ID) {
        imprimir_producao("<comando> ::= <atribuição>");
        nivel_producao++;
        atribuicao(sc);
        nivel_producao--;
    } else if (token_atual.tipo == KW_BEGIN) {
        imprimir_producao("<comando> ::= <comando composto>");
        nivel_producao++;
        comando_composto(sc);
        nivel_producao--;
    } else if (token_atual.tipo == KW_IF) {
        imprimir_producao("<comando> ::= <comando condicional>");
        nivel_producao++;
        comando_condicional(sc);
        nivel_producao--;
    } else if (token_atual.tipo == KW_WHILE) {
        imprimir_producao("<comando> ::= <comando repetitivo>");
        nivel_producao++;
        comando_repetitivo(sc);
        nivel_producao--;
    } else {
        erro_sintatico_msg("comando esperado");
    }
}

// <atribuição> ::= <variável> := <expressão>
void atribuicao(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<atribuição> ::= <variável> := <expressão>");
    nivel_producao++;
    
    variavel(sc);
    casa_token(OP_ASS, sc);
    expressao(sc);
    
    nivel_producao--;
}

// <comando condicional> ::= if <expressão> then <comando> [ else <comando> ]
void comando_condicional(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<comando condicional> ::= if <expressão> then <comando> [ else <comando> ]");
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

// <comando repetitivo> ::= while <expressão> do <comando>
void comando_repetitivo(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<comando repetitivo> ::= while <expressão> do <comando>");
    nivel_producao++;
    
    casa_token(KW_WHILE, sc);
    expressao(sc);
    casa_token(KW_DO, sc);
    comando(sc);
    
    nivel_producao--;
}

// <expressão> ::= <expressão simples> [ <relação> <expressão simples> ]
void expressao(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<expressão> ::= <expressão simples> [ <relação> <expressão simples> ]");
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

// <relação> ::= = | <> | < | <= | >= | >
void relacao(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<relação> ::= = | <> | < | <= | >= | >");
    nivel_producao++;
    
    if (token_atual.tipo == OP_EQ) {
        casa_token(OP_EQ, sc);
    } else if (token_atual.tipo == OP_NE) {
        casa_token(OP_NE, sc);
    } else if (token_atual.tipo == OP_LT) {
        casa_token(OP_LT, sc);
    } else if (token_atual.tipo == OP_LE) {
        casa_token(OP_LE, sc);
    } else if (token_atual.tipo == OP_GT) {
        casa_token(OP_GT, sc);
    } else if (token_atual.tipo == OP_GE) {
        casa_token(OP_GE, sc);
    }
    
    nivel_producao--;
}

// <expressão simples> ::= [ + | - ] <termo> { ( + | - ) <termo> }
void expressao_simples(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<expressão simples> ::= [ + | - ] <termo> { ( + | - ) <termo> }");
    nivel_producao++;
    
    if (token_atual.tipo == OP_AD || token_atual.tipo == OP_MIN) {
        if (token_atual.tipo == OP_AD) {
            casa_token(OP_AD, sc);
        } else {
            casa_token(OP_MIN, sc);
        }
    }
    
    termo(sc);
    
    while ((token_atual.tipo == OP_AD || token_atual.tipo == OP_MIN) && !erro_sintatico) {
        if (token_atual.tipo == OP_AD) {
            casa_token(OP_AD, sc);
        } else {
            casa_token(OP_MIN, sc);
        }
        termo(sc);
    }
    
    nivel_producao--;
}

// <termo> ::= <fator> { ( * | / ) <fator> }
void termo(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<termo> ::= <fator> { ( * | / ) <fator> }");
    nivel_producao++;
    
    fator(sc);
    
    while ((token_atual.tipo == OP_MUL || token_atual.tipo == OP_DIV) && !erro_sintatico) {
        if (token_atual.tipo == OP_MUL) {
            casa_token(OP_MUL, sc);
        } else {
            casa_token(OP_DIV, sc);
        }
        fator(sc);
    }
    
    nivel_producao--;
}

// <fator> ::= <variável> | <número> | ( <expressão> )
void fator(Scanner *sc) {
    if (erro_sintatico) return;
    
    if (token_atual.tipo == ID) {
        imprimir_producao("<fator> ::= <variável>");
        nivel_producao++;
        variavel(sc);
        nivel_producao--;
    } else if (token_atual.tipo == NUM_INT || token_atual.tipo == NUM_REAL) {
        imprimir_producao("<fator> ::= <número>");
        nivel_producao++;
        if (token_atual.tipo == NUM_INT) {
            casa_token(NUM_INT, sc);
        } else {
            casa_token(NUM_REAL, sc);
        }
        nivel_producao--;
    } else if (token_atual.tipo == SMB_OPA) {
        imprimir_producao("<fator> ::= ( <expressão> )");
        nivel_producao++;
        casa_token(SMB_OPA, sc);
        expressao(sc);
        casa_token(SMB_CPA, sc);
        nivel_producao--;
    } else {
        erro_sintatico_msg("fator esperado (variável, número ou expressão entre parênteses)");
    }
}

// <variável> ::= <identificador>
void variavel(Scanner *sc) {
    if (erro_sintatico) return;
    imprimir_producao("<variável> ::= <identificador>");
    nivel_producao++;
    
    casa_token(ID, sc);
    
    nivel_producao--;
}

// ======================================= função main =======================================
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <arquivo_entrada.pas> <arquivo_saida.lex>\n", argv[0]);
        return 1;
    }

    // Inicializa a tabela de símbolos
    inicializar_tabela_simbolos();

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        printf("Erro ao abrir o arquivo de entrada: %s\n", argv[1]);
        return 1;
    }

    // Lê todo o conteúdo do arquivo
    char *codigo_fonte = (char*)malloc(MAX_SOURCE_SIZE);
    if (!codigo_fonte) {
        printf("Erro ao alocar memória\n");
        fclose(fp);
        return 1;
    }

    size_t total = 0;
    size_t lido;
    while ((lido = fread(codigo_fonte + total, 1, MAX_SOURCE_SIZE - total - 1, fp)) > 0) {
        total += lido;
    }
    codigo_fonte[total] = '\0';
    fclose(fp);

    arquivo_saida = fopen(argv[2], "w");
    if (!arquivo_saida) {
        printf("Erro ao criar o arquivo de saída: %s\n", argv[2]);
        free(codigo_fonte);
        return 1;
    }

    fprintf(arquivo_saida, "======= ANÁLISE SINTÁTICA - MICROPASCAL =======\n\n");
    fprintf(arquivo_saida, "=== DERIVAÇÕES E REGRAS DE PRODUÇÃO ===\n\n");

    // Inicializa o scanner com todo o código fonte
    Scanner sc;
    iniciar(&sc, codigo_fonte, 1);

    // Lê o primeiro token
    token_atual.lexema = NULL;
    proximo_token_sintatico(&sc);

    // Inicia a análise sintática
    programa(&sc);

    // Verifica se chegou ao fim do arquivo
    if (!erro_sintatico && token_atual.tipo != END_TOKEN) {
        erro_sintatico_msg("fim de arquivo esperado");
    }

    if (!erro_sintatico) {
        fprintf(arquivo_saida, "\n\n=== ANÁLISE SINTÁTICA CONCLUÍDA COM SUCESSO ===\n");
        printf("Análise sintática concluída com sucesso!\n");
    } else {
        fprintf(arquivo_saida, "\n\n=== ANÁLISE SINTÁTICA CONCLUÍDA COM ERROS ===\n");
        printf("Análise sintática concluída com erros. Verifique o arquivo de saída.\n");
    }

    // Imprime a tabela de símbolos no final
    imprimir_tabela_simbolos(arquivo_saida);

    if (token_atual.lexema) {
        free(token_atual.lexema);
    }
    free(codigo_fonte);
    fclose(arquivo_saida);
    
    printf("Resultado salvo em '%s'.\n", argv[2]);
    return erro_sintatico ? 1 : 0;
}