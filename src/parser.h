#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {

    const TokenVector *tokens;
    size_t pos;

} Parser;

AstSequence *parse_tokens(const TokenVector *tokens);

#endif
