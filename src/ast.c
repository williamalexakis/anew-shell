#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "utils.h"

/* Create an empty AST sequence */
AstSequence *create_ast_sequence(void) {

    AstSequence *sequence = xmalloc(sizeof(*sequence));
    sequence->pipelines = NULL;
    sequence->count = 0;
    sequence->cap = 0;

    return sequence;

}

/* Add a pipeline to the sequence */
void append_ast_sequence(AstSequence *sequence, AstPipeline *pipeline) {

    // Dynamically resize the sequence if needed
    if (sequence->count == sequence->cap) {

        size_t new_cap = sequence->cap ? sequence->cap * 2 : 4;
        AstPipeline **new_array = xmalloc(new_cap * sizeof(AstPipeline *));

        for (size_t i = 0; i < sequence->count; ++i) new_array[i] = sequence->pipelines[i];

        free(sequence->pipelines);

        sequence->pipelines = new_array;
        sequence->cap = new_cap;

    }

    sequence->pipelines[sequence->count++] = pipeline;

}

/* Create an empty AST pipeline */
AstPipeline *create_ast_pipeline(void) {

    AstPipeline *pipeline = xmalloc(sizeof(*pipeline));
    pipeline->commands = NULL;
    pipeline->count = 0;
    pipeline->cap = 0;

    return pipeline;

}

/* Add a command to the pipeline */
void append_ast_pipeline(AstPipeline *pipeline, AstCommand *command) {

    // Dynamically resize the pipeline if needed
    if (pipeline->count == pipeline->cap) {

        size_t new_cap = pipeline->cap ? pipeline->cap * 2 : 4;
        AstCommand **new_array = xmalloc(new_cap * sizeof(AstCommand *));

        for (size_t i = 0; i < pipeline->count; ++i) new_array[i] = pipeline->commands[i];

        free(pipeline->commands);

        pipeline->commands = new_array;
        pipeline->cap = new_cap;

    }

    pipeline->commands[pipeline->count++] = command;

}

/* Create an empty AST command */
AstCommand *create_ast_command(void) {

    AstCommand *command = xmalloc(sizeof(*command));
    command->argv = NULL;
    command->argc = 0;
    command->argv_cap = 0;
    command->redir.type = REDIR_NONE;
    command->redir.path = NULL;

    return command;

}

/* Add an arg to a command */
void add_ast_command_arg(AstCommand *command, const char *lexeme) {

    // Dynamically resize the command if needed
    if (command->argc + 1 >= command->argv_cap) {

        size_t new_cap = command->argv_cap ? command->argv_cap * 2 : 4;
        char **new_argv = xmalloc(new_cap * sizeof(char *));

        for (size_t i = 0; i < command->argc; ++i) new_argv[i] = command->argv[i];

        free(command->argv);

        command->argv = new_argv;
        command->argv_cap = new_cap;

    }

    command->argv[command->argc++] = xstrdup(lexeme);  // Copy the lexeme as the arg
    command->argv[command->argc] = NULL;  // Null-terminate the arg

}

/* Configure a command's redirection type and path */
void set_ast_command_redir(AstCommand *command, RedirType type, const char *path) {

    // Clear existing path if it exists
    if (command->redir.path) {

        free(command->redir.path);

        command->redir.path = NULL;

    }

    // Set new redirection type and path
    command->redir.type = type;

    if (path) command->redir.path = xstrdup(path);

}

/* Free up a command */
static void free_ast_command(AstCommand *command) {

    if (!command) return;

    if (command->argv) {

        for (size_t i = 0; i < command->argc; ++i) free(command->argv[i]);

        free(command->argv);

    }

    if (command->redir.path) free(command->redir.path);

    free(command);

}

/* Free up a pipeline */
static void free_ast_pipeline(AstPipeline *pipeline) {

    if (!pipeline) return;

    for (size_t i = 0; i < pipeline->count; ++i) free_ast_command(pipeline->commands[i]);

    free(pipeline->commands);
    free(pipeline);

}

/* Free up the whole AST */
void free_ast(AstSequence *sequence) {

    if (!sequence) return;

    for (size_t i = 0; i < sequence->count; ++i) free_ast_pipeline(sequence->pipelines[i]);

    free(sequence->pipelines);
    free(sequence);

}

static void print_indent(int depth) {

    for (int i = 0; i < depth; i++) printf("    ");

}

void print_ast_command(const AstCommand *command, int depth) {

    printf("COMMAND [");

    for (size_t i = 0; i < command->argc; i++) {

        printf("\"%s\"", command->argv[i]);

        if (i + 1 < command->argc) printf(", ");

    }

    printf("]\n");

    if (command->redir.type != REDIR_NONE) {

        print_indent(depth + 1);
        printf("╰ REDIRECTION (%s) [\"%s\"]\n", command->redir.type == REDIR_TRUNC ? ">" : ">>", command->redir.path);

    }

}

void print_ast_pipeline(const AstPipeline *pipeline, int depth) {

    printf("PIPELINE\n");

    for (size_t i = 0; i < pipeline->count; i++) {

        print_indent(depth + 1);
        printf("╰ ");
        print_ast_command(pipeline->commands[i], depth + 1);

    }

}

void print_ast_sequence(const AstSequence *sequence, int depth) {

    print_indent(depth);
    printf("SEQUENCE\n");

    for (size_t i = 0; i < sequence->count; i++) {

        print_indent(depth + 1);
        printf("╰ ");
        print_ast_pipeline(sequence->pipelines[i], depth + 1);

    }

}
