#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "builtins.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"

/* cd command */
static int builtin_cd(int argc, char **argv) {

    const char *path = NULL;

    if (argc > 1) {

        path = argv[1];

    } else {

        path = getenv("HOME");

    }

    if (!path) {

        fprintf(stderr, "[vsh] Error: No path and HOME not set.\n");

        return 1;

    }

    if (chdir(path) != 0) {

        perror("[vsh] Error");

        return 1;

    }

    return 0;

}

/* echo command */
static int builtin_echo(int argc, char **argv) {

    for (int i = 1; i < argc; ++i) {

        fputs(argv[i], stdout);

        if (i + 1 < argc) fputc(' ', stdout);

    }

    fputc('\n', stdout);

    return 0;

}

/* help command */
static int builtin_help(int argc, char **argv) {

    (void)argc;
    (void)argv;

    printf("Ash builtin commands:\n");
    printf("cd <optional: path>          Change directory to a specified path; no path changes to HOME directory.\n");
    printf("echo <message>               Print a specified message in the standard output.\n");
    printf("help                         Display all builtin commands and their uses.\n");
    printf("tokens \"<command string>\"    Display the tokenized form of a command string.\n");
    printf("ast \"<command string>\"       Display the abstract syntax tree of a command string.\n");
    printf("exit                         Exit the shell.\n");

    return 0;

}

static int builtin_tokens(int argc, char **argv) {

    if (argc != 2) {

        fprintf(stderr, "[vsh] Usage: tokens \"<command string>\"\n");

        return 1;

    }

    TokenVector tokens;

    if (lex(argv[1], &tokens) != 0) {

        fprintf(stderr, "[vsh] Error: Lexer error.\n");

        return 1;

    }

    for (size_t i = 0; i < tokens.count; ++i) {

        Token *token = &tokens.tokens[i];
        const char *type = NULL;

        switch (token->type) {

            case TOK_WORD: type = "WORD"; break;
            case TOK_PIPE: type = "PIPE"; break;
            case TOK_SEMICOLON: type = "SEMICOLON"; break;
            case TOK_REDIR_OUT: type = "REDIRECT OUT"; break;
            case TOK_REDIR_APPEND: type = "REDIRECT APPEND"; break;
            case TOK_EOF: type = "EOF"; break;
            case TOK_ERROR: type = "ERROR"; break;

        }

        if (token->type == TOK_WORD) {

            printf("%zu | %s '%s'\n", token->pos, type, token->lexeme);

        } else {

            printf("%zu | %s\n", token->pos, type);

        }

    }

    token_vector_free(&tokens);

    return 0;

}

static int builtin_ast(int argc, char **argv) {

    if (argc != 2) {

        fprintf(stderr, "[vsh] Usage: ast \"<command string>\"\n");

        return 1;

    }

    TokenVector tokens;

    if (lex(argv[1], &tokens) != 0) {

        fprintf(stderr, "[vsh] Error: Lexer error.");

        return 1;

    }

    AstSequence *sequence = parse_tokens(&tokens);

    if (!sequence) {

        fprintf(stderr, "[vsh] Error: Parser error.");

        token_vector_free(&tokens);

        return 1;

    }

    print_ast_sequence(sequence, 0);
    free_ast(sequence);
    token_vector_free(&tokens);

    return 0;

}

/* exit command; actual exit logic is controlled by the executor */
static int builtin_exit(int argc, char **argv) {

    (void)argc;
    (void)argv;

    return 0;

}

/* Store all builtin commands */
static BuiltinEntry builtin_commands[] = {

    {"cd", builtin_cd},
    {"echo", builtin_echo},
    {"help", builtin_help},
    {"tokens", builtin_tokens},
    {"ast", builtin_ast},
    {"exit", builtin_exit}

};

static const size_t builtin_count = sizeof(builtin_commands) / sizeof(builtin_commands[0]);

/* Search for a builtin command */
builtin_func builtin_lookup(const char *name) {

    for (size_t i = 0; i < builtin_count; ++i) {

        if (strcmp(name, builtin_commands[i].name) == 0) return builtin_commands[i].func;

    }

    return NULL;

}
