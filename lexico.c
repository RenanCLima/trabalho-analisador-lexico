
// lexer_int_scanf_main.c - analisador léxico para inteiros e + - * / ( )
// Compilar: gcc -std=c11 -O2 lexer_int_scanf_main.c -o lexer_int
// Testar:   echo "12*(3+4) - 5/2" | ./lexer_int

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ========================= Definições ========================= */

typedef enum {
    TOKEN_INTEIRO,
    TOKEN_MAIS,      // +
    TOKEN_MENOS,     // -
    TOKEN_MULT,      // *
    TOKEN_DIV,       // /
    TOKEN_ABRE_PAR,  // (
    TOKEN_FECHA_PAR, // )
    TOKEN_FIM,
    TOKEN_ERRO
} TipoToken;

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
    char c;
} Scanner;

char* nome_token(TipoToken t){
    switch(t){
        case TOKEN_INTEIRO:   return "INTEIRO";
        case TOKEN_MAIS:      return "MAIS";
        case TOKEN_MENOS:     return "MENOS";
        case TOKEN_MULT:      return "MULT";
        case TOKEN_DIV:       return "DIV";
        case TOKEN_ABRE_PAR:  return "ABRE_PAR";
        case TOKEN_FECHA_PAR: return "FECHA_PAR";
        case TOKEN_FIM:       return "FIM";
        case TOKEN_ERRO:      return "ERRO";
        default:              return "?";
    }
}

char *str_ndup(const char *s, size_t n){
    char *p = (char*)malloc(n+1);
    if(!p){ fprintf(stderr,"Memória insuficiente\n"); exit(1); }
    memcpy(p, s, n); p[n]='\0'; return p;
}

void iniciar(Scanner *sc, const char *texto, int LinhaAntiga){
    sc->src = texto ? texto : "";
    sc->i = 0; 
    sc->linha = LinhaAntiga; 
    sc->coluna = 1;
    sc->c = sc->src[0];
}

void avancar(Scanner *sc){
    if(sc->c=='\0') return;
    if(sc->c=='\n'){ sc->linha++; sc->coluna=1; }
    else           { sc->coluna++; }
    sc->i++;
    sc->c = sc->src[sc->i];
}

void pular_espacos(Scanner *sc){
    while(isspace((unsigned char)sc->c)) avancar(sc);
}

Token criar_token_texto(Scanner *sc, TipoToken tipo, const char *ini, size_t n, int lin, int col){
    (void)sc;
    Token t; t.tipo=tipo;
    t.lexema=str_ndup(ini,n); 
    t.linha=lin; t.coluna=col; 
    return t;
}

Token token_simples(Scanner *sc, TipoToken tipo){
    int lin=sc->linha, col=sc->coluna;
    char ch=sc->c; avancar(sc);
    return criar_token_texto(sc, tipo, &ch, 1, lin, col);
}

Token token_erro_msg(Scanner *sc, const char *msg){
    return criar_token_texto(sc, TOKEN_ERRO, msg, strlen(msg), sc->linha, sc->coluna);
}

Token coletar_inteiro(Scanner *sc){
    int lin=sc->linha, col=sc->coluna;
    size_t ini = sc->i;
    if(!isdigit((unsigned char)sc->c)) return token_erro_msg(sc, "Inteiro malformado");
    while(isdigit((unsigned char)sc->c)) avancar(sc);
    return criar_token_texto(sc, TOKEN_INTEIRO, sc->src+ini, sc->i-ini, lin, col);
}

Token proximo_token(Scanner *sc){
    pular_espacos(sc);
    if(sc->c=='\0') return criar_token_texto(sc, TOKEN_FIM, "", 0, sc->linha, sc->coluna);

    if(isdigit((unsigned char)sc->c)) return coletar_inteiro(sc);

    switch(sc->c){
        case '+': return token_simples(sc, TOKEN_MAIS);
        case '-': return token_simples(sc, TOKEN_MENOS);
        case '*': return token_simples(sc, TOKEN_MULT);
        case '/': return token_simples(sc, TOKEN_DIV);
        case '(': return token_simples(sc, TOKEN_ABRE_PAR);
        case ')': return token_simples(sc, TOKEN_FECHA_PAR);
        default: {
            char msg[64];
            snprintf(msg, sizeof(msg), "Caractere inválido: '%c'", sc->c);
            avancar(sc);
            return token_erro_msg(sc, msg);
        }
    }
}



/* ========================= main ========================= */

int main(void){

    FILE * fp = fopen("teste.mat", "r");
    FILE * out = fopen("teste.lex", "a");
    
    char Entrada[1024];
    int Linha = 1;

    while(fscanf(fp, "%[^\n]\n", Entrada) > 0) {
        Scanner S; 
        strcat(Entrada, "\n");
        printf("%s", Entrada);
        iniciar(&S, Entrada, Linha);

        for(;;){
            Token t = proximo_token(&S);
            fprintf(out, "(%s, %s) \t\t linha %d, col %d\n",
                nome_token(t.tipo), t.lexema, Linha, t.coluna);
            free(t.lexema);
            if(t.tipo==TOKEN_FIM || t.tipo==TOKEN_ERRO) break;
        }
        Linha++;
    }

    return 0;
}
