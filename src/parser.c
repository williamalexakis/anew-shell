#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include "ast.h"
#include "lexer.h"
#include "utils.h"
#include "parser.h"

static AstPipeline *parse_pipeline(Parser *parser);  // Forward declaration

/* Get the next token */
static const Token *peek(Parser *parser) {

    if (parser->pos < parser->tokens->count) return &parser->tokens->tokens[parser->pos];

    return NULL;

}

/* Advance the parser */
static const Token *advance(Parser *parser) {

    const Token *token = peek(parser);

    if (token && token->type != TOK_EOF) parser->pos++;  // Move forward if we haven't reached EOF

    return token;

}

/* Check if token matches the expected type */
static bool match(Parser *parser, TokenType type) {

    const Token *token = peek(parser);

    if (token && token->type == type) {

        advance(parser);

        return true;

    }

    return false;

}

/* Handle parsing errors */
static void parse_error(const Token *token, const char *message) {

    if (token) {

        fprintf(stderr, "[ash] Error (pos: %zu): %s\n", token->pos, message);

    } else {

        fprintf(stderr, "[ash] Error: %s", message);

    }

}

/* Process a sequence node */
static AstSequence *parse_sequence(Parser *parser) {

    const Token *token = peek(parser);

    if (!token || token->type == TOK_EOF) return NULL;  // EOF reached

    AstSequence *sequence = create_ast_sequence();
    AstPipeline *pipeline = create_ast_pipeline();

    if (!pipeline) {

        free_ast(sequence);

        return NULL;

    }

    append_ast_sequence(sequence, pipeline);

    while (match(parser, TOK_SEMICOLON)) {

        const Token *next_token = peek(parser);

        if (!next_token || next_token->type == TOK_EOF) break;

        pipeline = parse_pipeline(parser);

        if (!pipeline) {

            free_ast(sequence);

            return NULL;

        }

        append_ast_sequence(sequence, pipeline);

    }

    return sequence;

}

/* Process a command node */
static AstCommand *parse_command(Parser *parser) {

    const Token *token = peek(parser);

    if (!token || token->type != TOK_WORD) {

        parse_error(token, "Expected command name.");

        return NULL;

    }

    AstCommand *command = create_ast_command();

    while ((token = peek(parser)) && token->type == TOK_WORD) {

        add_ast_command_arg(command, token->lexeme);
        advance(parser);

    }

    while ((token = peek(parser))) {

        if (token->type == TOK_REDIR_OUT || token->type == TOK_REDIR_APPEND) {

            RedirType r_type = (token->type == TOK_REDIR_OUT) ? REDIR_TRUNC : REDIR_APPEND;

            advance(parser);

            const Token *file_name = peek(parser);

            if (!file_name || file_name->type != TOK_WORD) {

                // For simplicity, we just leak the command on parse error, and
                // parse errors aren't high performance paths either way
                parse_error(file_name, "Expected file name after redirection.");
                free_ast(NULL);  // No-op placeholder

                return NULL;

            }

            set_ast_command_redir(command, r_type, file_name->lexeme);
            advance(parser);

        } else {

            break;

        }

    }

    if (command->argc == 0) {

        // Small leak on the error is also acceptable here
        parse_error(token, "Empty command.");

        return NULL;

    }

    return command;

}

/* Process a pipeline node */
static AstPipeline *parse_pipeline(Parser *parser) {

    AstCommand *command = parse_command(parser);

    if (!command) return NULL;

    AstPipeline *pipeline = create_ast_pipeline();

    append_ast_pipeline(pipeline, command);

    while (match(parser, TOK_PIPE)) {

        const Token *token = peek(parser);

        if (!token || token->type != TOK_WORD) {

            parse_error(token, "Expected command after '|'.");
            free_ast(NULL);  // No-op placeholder

            return NULL;

        }

        command = parse_command(parser);

        if (!command) return NULL;

        append_ast_pipeline(pipeline, command);

    }

    return pipeline;

}

/* Process the tokens into an AST sequence */
AstSequence *parse_tokens(const TokenVector *tokens) {

    Parser parser = { .tokens = tokens, .pos = 0 };

    if (tokens->count == 0) return NULL;

    const Token *first_token = &tokens->tokens[0];

    if (first_token->type == TOK_EOF) return NULL;

    if (first_token->type == TOK_ERROR) {

        parse_error(first_token, "Lexer error.");

        return NULL;

    }

    AstSequence *sequence = parse_sequence(&parser);

    if (!sequence) return NULL;

    const Token *token = peek(&parser);

    if (token && token->type != TOK_EOF) {

        parse_error(token, "Unexpected tokens at end of line.");
        free_ast(sequence);

        return NULL;

    }

    return sequence;

}
