#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "utils.h"

/* Dynamically resize token vector */
static void token_vector_push(TokenVector *vec, Token token) {

    if (vec->count == vec->cap) {

        size_t new_cap = vec->cap ? vec->cap * 2 : 16;
        Token *new_tokens = xmalloc(new_cap * sizeof(Token));

        // We copy all tokens into the new memory space
        for (size_t i = 0; i < vec->count; ++i) new_tokens[i] = vec->tokens[i];

        free(vec->tokens);

        vec->tokens = new_tokens;
        vec->cap = new_cap;

    }

    vec->tokens[vec->count++] = token;

}

/* Create an empty token vector */
void token_vector_init(TokenVector *vec) {

    vec->tokens = NULL;
    vec->count = 0;
    vec->cap = 0;

}

/* Free vector memory if any heap-allocated lexemes are present */
void token_vector_free(TokenVector *vec) {

    if (!vec) return;

    // Free all token lexemes
    for (size_t i = 0; i < vec->count; ++i) free(vec->tokens[i].lexeme);

    free(vec->tokens);

    vec->tokens = NULL;
    vec->count = 0;
    vec->cap = 0;

}

/* Create a token */
static Token make_token(TokenType type, char *lexeme, size_t pos) {

    Token token = { .type = type, .lexeme = lexeme, .pos = pos };

    return token;

}

/* Lex a string and return a lexeme */
static char *lex_string(const char *input, size_t *pos, char quote_char, int *error) {

    size_t start_pos = *pos;

    (*pos)++;  // Skip the opening quote

    size_t buffer_cap = 32;
    size_t buffer_len = 0;
    char *buffer = xmalloc(buffer_cap);

    while (input[*pos] != '\0') {

        char c = input[*pos];

        if (c == quote_char) {

            (*pos)++;  // Skip the closing quote

            buffer[buffer_len] = '\0';

            return buffer;

        }

        // Support \" and \\ escape chars
        if (quote_char == '"' && c == '\\') {

            char next_char = input[*pos + 1];

            if (next_char == '"' || next_char == '\\') {

                c = next_char;
                *pos += 2;

            } else {

                (*pos)++;  // Skip if it's an unknown escape char

            }

        } else {

            (*pos)++;

        }

        // Dynamically resize buffer if it exceeds cap
        if (buffer_len + 1 >= buffer_cap) {

            buffer_cap *= 2;
            char *new_buffer = xmalloc(buffer_cap);

            memcpy(new_buffer, buffer, buffer_len);
            free(buffer);

            buffer = new_buffer;

        }

        buffer[buffer_len++] = c;  // Add the current char to the buffer

    }

    // We throw an error if no closing quote is found
    fprintf(stderr, "[vsh] Error: Unterminated string starting at position %zu.\n", start_pos);
    free(buffer);

    *error = 1;

    return NULL;

}

/* Lex an arbitrary word and return a lexeme */
static char *lex_word(const char *input, size_t *pos) {

    size_t start_pos = *pos;

    while (input[*pos] != '\0') {

        char c = input[*pos];

        // Check if char is whitespace, string, or an operator
        if (isspace((unsigned char)c) || c == '|' || c == ';' || c == '>' || c == '\'' || c == '"') break;

        (*pos)++;

    }

    // Allocate a buffer for the lexeme
    size_t buffer_len = *pos - start_pos;
    char *buffer = xmalloc(buffer_len + 1);

    memcpy(buffer, &input[start_pos], buffer_len);

    buffer[buffer_len] = '\0';

    return buffer;

}

/* Lex a line and push its tokens to the vector */
int lex(const char *input, TokenVector *output) {

    token_vector_init(output);

    size_t pos = 0;
    int error = 0;

    while (input[pos] != '\0') {

        char c = input[pos];

        // We ignore whitespace
        if (isspace((unsigned char)c)) {

            pos++;

            continue;

        }

        // Semicolon
        if (c == ';') {

            token_vector_push(output, make_token(TOK_SEMICOLON, NULL, pos));

            pos++;

            continue;

        }

        // Pipe
        if (c == '|') {

            token_vector_push(output, make_token(TOK_PIPE, NULL, pos));

            pos++;

            continue;

        }

        // Redirect
        if (c == '>') {

            // Append
            if (input[pos + 1] == '>') {

                token_vector_push(output, make_token(TOK_REDIR_APPEND, NULL, pos));

                pos += 2;

            // Out
            } else {

                token_vector_push(output, make_token(TOK_REDIR_OUT, NULL, pos));

                pos++;

            }

            continue;

        }

        // Check for strings
        if (c == '\'' || c == '"') {

            char *lexeme = lex_string(input, &pos, c, &error);

            if (error) {

                Token token = make_token(TOK_ERROR, NULL, pos);

                token_vector_push(output, token);

                return -1;

            }

            token_vector_push(output, make_token(TOK_WORD, lexeme, pos));

            continue;

        }

        // Handle arbitrary words
        char *lexeme = lex_word(input, &pos);

        if (lexeme[0] == '\0') {

            free(lexeme);

            continue;

        }

        token_vector_push(output, make_token(TOK_WORD, lexeme, pos));

    }

    token_vector_push(output, make_token(TOK_EOF, NULL, pos));

    return 0;

}
