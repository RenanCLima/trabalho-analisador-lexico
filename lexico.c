
// lexer_int_scanf_main.c - analisador léxico para inteiros e + - * / ( )
// Compilar: gcc -std=c11 -O2 lexer_int_scanf_main.c -o lexer_int
// Testar:   echo "12*(3+4) - 5/2" | ./lexer_int

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ========================= Definições de Variáveis ========================= */
typedef enum {
    // Operadores
    OP_EQ,     // = 
    OP_GE,     // >=  
    OP_MUL,    //  * 
    OP_NE,     // <>  
    OP_LE,     // <=  
    OP_DIV,    //  / 
    OP_GT,     // >  
    OP_AD,     // +  
    OP_ASS,    //  = 
    OP_LT,     // <  
    OP_MIN,    //  - 

    // Símbolos  
    SMB_OBC,   //  { 
    SMB_COM,   //  , 
    SMB_CBC,   //  } 
    SMB_SEM,   //  ; 
    SMB_OPA,   //  ( 
    SMB_CPA,   //  )

    // Palavras-Chave
    KW_PROGRAM,
    KW_VAR,
    KW_INTEGER,
    KW_REAL,
    KW_BEGIN,
    KW_END,
    KW_IF,
    KW_THEN,
    KW_ELSE,
    KW_WHILE,
    KW_DO,
    
    // Identificadores e literais
    ID,
    NUM_INT,
    NUM_REAL,
    
    // Especiais
    END_TOKEN,
    ERROR_TOKEN
} TipoToken;                // Tipo da variável: TipoToken

// Struct é como um Array com diferentes tipos de variáveis (por isso tem um TipoToken, char *, int)
typedef struct {
    TipoToken tipo;         // Tipo declarado acima
    char *lexema;           // 
    int linha;              // Linha do Token
    int coluna;             // Coluna do Token
} Token;                    // Tipo da variável: Token

// Struct do Scanner
typedef struct {
    const char *src;
    int i;
    int linha, coluna;      // Posição
    char caractere;                 // Caractere
} Scanner;                  // Tipo da variável: Scanner

// ======================================= Métodos =======================================

char* nome_token(TipoToken t){
    switch(t){
        case NUM_INT:   return "INTEIRO";
        case OP_AD:      return "MAIS";
        case OP_MIN:     return "MENOS";
        case OP_MUL:      return "MULT";
        case OP_DIV:       return "DIV";
        case SMB_OPA:  return "ABRE_PAR";
        case SMB_CPA: return "FECHA_PAR";
        case END_TOKEN:       return "FIM";
        case ERROR_TOKEN:      return "ERRO";
        default:              return "?";
    }
}

// Função que tem como retorno um ponteiro para o primeiro caractere de uma String
char *str_ndup(const char *s, size_t n){
    char *p = (char*)malloc(n+1);
    if(!p){ fprintf(stderr,"Memória insuficiente\n"); exit(1); }
    memcpy(p, s, n); p[n]='\0'; return p;
}

// Inicia o Scanner
void iniciar(Scanner *sc, const char *texto, int LinhaAntiga){
    sc->src = texto ? texto : "";
    sc->linha = LinhaAntiga; 
    sc->coluna = 1;
    sc->caractere = sc->src[0];
}

// Avança o Scanner
void avancar(Scanner *sc){
    if(sc->caractere=='\0') return;
    if(sc->caractere=='\n'){ sc->linha++; sc->coluna=1; }
    else           { sc->coluna++; }
    sc->i++;
    sc->caractere = sc->src[sc->i];
}

// Scanner pula espaços
void pular_espacos(Scanner *sc){
    while(isspace((unsigned char)sc->caractere)) avancar(sc); //unsigned char são caracteres como \n
}


// Token Texto
Token criar_token_texto(Scanner *sc, TipoToken tipo, const char *ini, size_t n, int lin, int col){
    (void)sc;
    Token t; t.tipo=tipo;
    t.lexema=str_ndup(ini,n); 
    t.linha=lin; t.coluna=col; 
    return t;
}

// Token Simples
Token token_simples(Scanner *sc, TipoToken tipo){
    int lin=sc->linha, col=sc->coluna;
    char ch=sc->caractere; avancar(sc);
    return criar_token_texto(sc, tipo, &ch, 1, lin, col);
}

// Token mensagem de erro 
Token token_erro_msg(Scanner *sc, const char *msg){
    return criar_token_texto(sc, ERROR_TOKEN, msg, strlen(msg), sc->linha, sc->coluna);
}

// Token número inteiro
Token coletar_inteiro(Scanner *sc){
    int lin=sc->linha, col=sc->coluna;
    size_t ini = sc->i;
    if(!isdigit((unsigned char)sc->caractere)) return token_erro_msg(sc, "Inteiro malformado");
    while(isdigit((unsigned char)sc->caractere)) avancar(sc);
    return criar_token_texto(sc, NUM_INT, sc->src+ini, sc->i-ini, lin, col);
}


// Avança para o próximo Token
Token proximo_token(Scanner *sc){
    // Primeiro pula todos os espaços
    pular_espacos(sc);
    //
    if(sc->caractere=='\0') return criar_token_texto(sc, END_TOKEN, "", 0, sc->linha, sc->coluna);
    //
    if(isdigit((unsigned char)sc->caractere)) return coletar_inteiro(sc);
    // 
    switch(sc->caractere){
        case '+': return token_simples(sc, OP_AD);
        case '-': return token_simples(sc, OP_MIN);
        case '*': return token_simples(sc, OP_MUL);
        case '/': return token_simples(sc, OP_MUL);
        case '(': return token_simples(sc, SMB_OPA);
        case ')': return token_simples(sc, SMB_CPA);
        default: {
            char msg[64];
            snprintf(msg, sizeof(msg), "Caractere inválido: '%c'", sc->caractere);
            avancar(sc);
            return token_erro_msg(sc, msg);
        }
    }
}



// ======================================= main =======================================

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
            if(t.tipo==END_TOKEN || t.tipo==ERROR_TOKEN) break;
        }
        Linha++;
    }

    return 0;
}
