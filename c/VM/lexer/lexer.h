#ifndef LEXER_H
#define LEXER_H

#include "../token.h"


// Runs the lexer over source and writes up to MAX_TOKENS tokens into out_tokens.
// Returns the number of tokens written, or -1 on error.
void lex(const char *source, Token **out_tokens, int init_capacity);



#endif