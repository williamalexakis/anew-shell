#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "builtins.h"

/* cd command */
static int builtin_cd(int argc, char **argv) {

    const char *path = NULL;

    if (argc > 1) {

        path = argv[1];

    } else {

        path = getenv("HOME");

    }

    if (!path) {

        fprintf(stderr, "[ash] Error: No path and HOME not set.\n");

        return 1;

    }

    if (chdir(path) != 0) {

        perror("[ash] Error");

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

    printf("Ash builtin commands:");
    printf("cd <optional: path>    Change directory to a specified path; no path changes to HOME directory.");
    printf("echo <message>         Print a specified message in the standard output.");
    printf("help                   Display all builtin commands and their uses.");
    printf("exit                   Exit the shell.");

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
