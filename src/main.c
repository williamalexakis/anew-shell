#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "exec.h"
#include "ast.h"

/* Display prompt */
static void print_prompt(void) {

    fputs("vsh> ", stdout);
    fflush(stdout);

}

int main(void) {

    char *line = NULL;
    size_t cap = 0;

    while (1) {

        print_prompt();

        ssize_t nread = getline(&line, &cap, stdin);

        if (nread < 0) {

            if (feof(stdin)) {

                putchar('\n');

                break;

            }

            perror("[vsh] Error");

            break;

        }

        if (nread == 0) continue;

        // Strip trailing newline
        if (line[nread - 1] == '\n') line[nread - 1] = '\0';

        if (line[0] == '\0') continue;

        TokenVector tokens;

        if (lex(line, &tokens) != 0) {

            token_vector_free(&tokens);

            continue;

        }

        AstSequence *sequence = parse_tokens(&tokens);

        if (!sequence) {

            token_vector_free(&tokens);

            continue;

        }

        int status = exec_sequence(sequence);

        free_ast(sequence);
        token_vector_free(&tokens);

        if (status == SHELL_STATUS_EXIT) break;

    }

    free(line);

    return 0;

}
