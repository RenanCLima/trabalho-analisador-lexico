// Para rodar o código no terminal BASH primeiro utilize: gcc lexico.c -o lexico.exe
// Em seguida utilize no terminal: ./lexico.exe arquivo_entrada.pas arquivo_saida.lex
// ======================================= imports =======================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ======================================= Definições =======================================
#define MAX_SOURCE_SIZE 10000

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
    
    // Operadores
    OP_ASSIGN,  //  =:
    OP_EQ,      //  =
    OP_LT,      //  <
    OP_GT,      //  >
    OP_LE,      //  <=
    OP_GE,      //  >=
    OP_NE,      //  <>
    OP_PLUS,    //  +
    OP_MINUS,   //  -
    OP_MUL,     //  *
    OP_DIV,     //  /
    
    // Símbolos
    SMB_SEMICOLON,  //  ;
    SMB_COMMA,      //  ,
    SMB_DOT,        //  .
    SMB_COLON,      //  :
    SMB_LPAREN,     //  (
    SMB_RPAREN,     //  )
    SMB_LBRACK,     //  {
    SMB_RBRACK,     //  }
    
    // Literais e identificadores
    IDENTIFIER,     //  Identificador
    NUM_INT,        //  Número inteiro
    NUM_REAL,       //  Número Real
    
    // Especiais
    END_TOKEN,      //  final do código
    ERROR_TOKEN     //  erro do código
} TipoToken;

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
        case OP_ASSIGN: return "ASSIGN";
        case OP_EQ: return "EQUAL";
        case OP_LT: return "LESS";
        case OP_GT: return "GREATER";
        case OP_LE: return "LESS_EQUAL";
        case OP_GE: return "GREATER_EQUAL";
        case OP_NE: return "NOT_EQUAL";
        case OP_PLUS: return "PLUS";
        case OP_MINUS: return "MINUS";
        case OP_MUL: return "MULT";
        case OP_DIV: return "DIV";
        case SMB_SEMICOLON: return "SEMICOLON";
        case SMB_COMMA: return "COMMA";
        case SMB_DOT: return "DOT";
        case SMB_COLON: return "COLON";
        case SMB_LPAREN: return "LPAREN";
        case SMB_RPAREN: return "RPAREN";
        case SMB_LBRACK: return "LBRACK";
        case SMB_RBRACK: return "RBRACK";
        case IDENTIFIER: return "IDENTIFIER";
        case NUM_INT: return "INTEGER_LITERAL";
        case NUM_REAL: return "REAL_LITERAL";
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
    if (strcmp(lexema, "program") == 0) return KW_PROGRAM;
    if (strcmp(lexema, "var") == 0) return KW_VAR;
    if (strcmp(lexema, "integer") == 0) return KW_INTEGER;
    if (strcmp(lexema, "real") == 0) return KW_REAL;
    if (strcmp(lexema, "begin") == 0) return KW_BEGIN;
    if (strcmp(lexema, "end") == 0) return KW_END;
    if (strcmp(lexema, "if") == 0) return KW_IF;
    if (strcmp(lexema, "then") == 0) return KW_THEN;
    if (strcmp(lexema, "else") == 0) return KW_ELSE;
    if (strcmp(lexema, "while") == 0) return KW_WHILE;
    if (strcmp(lexema, "do") == 0) return KW_DO;
    return IDENTIFIER;
}

// 
Token coletar_identificador(Scanner *sc) {
    int lin = sc->linha, col = sc->coluna;
    size_t ini = sc->i;
    while (isalnum((unsigned char)sc->caractere)) avancar(sc);
    size_t len = sc->i - ini;
    TipoToken tipo = verificar_palavra_chave(sc->src + ini);
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
        return criar_token_texto(OP_ASSIGN, ":=", 2, lin, col);
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

// Busca o próximo Token, caso encontre um símbolo, define um Token
Token proximo_token(Scanner *sc) {
    pular_espacos(sc);
    if (sc->caractere == '\0') return criar_token_texto(END_TOKEN, "", 0, sc->linha, sc->coluna);
    if (isalpha((unsigned char)sc->caractere)) return coletar_identificador(sc);
    if (isdigit((unsigned char)sc->caractere))
        return coletar_numero(sc);

    if (strchr(":<>=", sc->caractere))
        return coletar_operador(sc);

    switch (sc->caractere) {
        case '+': return token_simples(sc, OP_PLUS);
        case '-': return token_simples(sc, OP_MINUS);
        case '*': return token_simples(sc, OP_MUL);
        case '/': return token_simples(sc, OP_DIV);
        case ';': return token_simples(sc, SMB_SEMICOLON);
        case ',': return token_simples(sc, SMB_COMMA);
        case '.': return token_simples(sc, SMB_DOT);
        case '(': return token_simples(sc, SMB_LPAREN);
        case ')': return token_simples(sc, SMB_RPAREN);
        case '[': return token_simples(sc, SMB_LBRACK);
        case ']': return token_simples(sc, SMB_RBRACK);
        default: {
            char msg[64];
            snprintf(msg, sizeof(msg), "Caractere inválido: '%c'", sc->caractere);
            avancar(sc);
            return token_erro_msg(sc, msg);
        }
    }
}

// ======================================= função main que testa os códigos =======================================
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <arquivo_entrada.pas> <arquivo_saida.lex>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        printf("Erro ao abrir o arquivo de entrada: %s\n", argv[1]);
        return 1;
    }

    FILE *out = fopen(argv[2], "w");
    if (!out) {
        printf("Erro ao criar o arquivo de saída: %s\n", argv[2]);
        fclose(fp);
        return 1;
    }

    char linha[1024];
    int num_linha = 1;

    while (fgets(linha, sizeof(linha), fp)) {
        Scanner sc;
        iniciar(&sc, linha, num_linha);

        for (;;) {
            Token t = proximo_token(&sc);
            fprintf(out, "%s, %s\t\t\tlinha %d, col%d\n",
                nome_token(t.tipo), t.lexema, t.linha, t.coluna);
            free(t.lexema);
            if (t.tipo == END_TOKEN || t.tipo == ERROR_TOKEN) break;
        }

        fprintf(out, "\n");
        num_linha++;
    }

    fclose(fp);
    fclose(out);
    printf("Análise léxica concluída. Tokens salvos em '%s'.\n", argv[2]);
    return 0;
}