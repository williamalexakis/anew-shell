#ifndef AST_H
#define AST_H

#include <stddef.h>

typedef enum {

    REDIR_NONE = 0,
    REDIR_TRUNC,
    REDIR_APPEND

} RedirType;

typedef struct {

    RedirType type;
    char *path;

} Redirection;

typedef struct AstCommand {

    char **argv;
    size_t argc;
    size_t argv_cap;
    Redirection redir;

} AstCommand;

typedef struct AstPipeline {

    AstCommand **commands;
    size_t count;
    size_t cap;

} AstPipeline;

typedef struct AstSequence {

    AstPipeline **pipelines;
    size_t count;
    size_t cap;

} AstSequence;

AstSequence *create_ast_sequence(void);
void append_ast_sequence(AstSequence *sequence, AstPipeline *pipeline);
AstPipeline *create_ast_pipeline(void);
void append_ast_pipeline(AstPipeline *pipeline, AstCommand *command);
AstCommand *create_ast_command(void);
void add_ast_command_arg(AstCommand *command, const char *lexeme);
void set_ast_command_redir(AstCommand *command, RedirType type, const char *path);
void free_ast(AstSequence *sequence);
void print_ast_sequence(const AstSequence *sequence, int depth);
void print_ast_pipeline(const AstPipeline *pipeline, int depth);
void print_ast_command(const AstCommand *command, int depth);

#endif
