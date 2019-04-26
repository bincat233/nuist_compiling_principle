#ifndef __lexical__analysis_h
#define __lexical__analysis_h
typedef enum { KEY = 0, ID, NUM, CHAR, STR, AI, DELIM, OP } TokenType;
typedef struct {
  TokenType type;
  char *value;
} TokenRecord;
extern void init_input(char *path);
extern void init_tokens();
extern void analysis();
extern void clean_tokens();
extern void print_tokens();
extern void print_token(TokenRecord *t);
extern bool has_token();
extern int get_tokens_size();
extern TokenRecord *next_token();
extern void reset_token_iterator();
extern char *get_typename(TokenType t);
#endif
