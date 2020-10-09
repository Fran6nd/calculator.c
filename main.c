#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#include "token_list.h"
#include "token.h"

int DEBUG = 0;

void error(char *message)
{
    printf("Error: %s", message);
    exit(-1);
}

int is_char_operator(char c)
{
    if (c == '-' || c == '+' || c == '*' || c == '/')
        return 1;
    return 0;
}

int is_string_number(char *c)
{
    if (strlen(c) == 0)
    {
        return 0;
    }
    int point_counter = 0;
    for (unsigned int i = 0; i < strlen(c); i++)
    {
        if (c[i] == '.')
        {
            point_counter++;
        }
    }
    if (point_counter <= 1)
    {
        for (unsigned int i = 0; i < strlen(c); i++)
        {
            if ((c[i] < 48 || c[i] > 57) && c[i] != '.')
            {
                return 0;
            }
        }
        return 1;
    }
    return 0;
}

struct symbol is_symbol(char *c)
{
    for (int i = 0; i < AMOUNT_OF_SYMBOLS; i++)
    {
        if (strcmp(c, SYMBOLS[i].str) == 0)
        {
            return SYMBOLS[i];
        }
    }
    struct symbol s = {"", SYMBOL_NULL, 0};
    return s;
}

int is_beginning_of_symbol(char *c)
{
    for (unsigned int i = 0; i < AMOUNT_OF_SYMBOLS; i++)
    {
        if (strlen(SYMBOLS[i].str) > strlen(c))
        {
            int ok = 1;
            for (unsigned int j = 0; j < strlen(c); j++)
            {
                if (c[j] != SYMBOLS[i].str[j])
                {
                    ok = 0;
                    break;
                }
            }
            if (ok == 1)
            {
                return 1;
            }
        }
    }
    return 0;
}
int is_operator(struct token t)
{
    if (t.type == TYPE_TOKEN && (t.symbol == SYMBOL_SUB || t.symbol == SYMBOL_ADD || t.symbol == SYMBOL_MUL || t.symbol == SYMBOL_DIV))
        return 1;
    return 0;
}

struct token_list *tokenize(char *expression)
{
    struct token_list *tl = tl_new();
    char buffer[100] = {0};
    int buffer_index = 0;
    char c;
    for (unsigned int i = 0; i <= strlen(expression); i++)
    {
        c = expression[i];
        buffer[buffer_index] = c;
        buffer_index++;
        if ((is_symbol(buffer).id == SYMBOL_NULL && !is_string_number(buffer) && !is_beginning_of_symbol(buffer)) || i == strlen(expression) || c == ' ')
        {
            if (i < strlen(expression) && i > 0)
            {
                buffer_index--;
                buffer[buffer_index] = 0;
            }
            if (is_string_number(buffer))
            {
                tl_add(tl, t_new_value(atof(buffer)));
                memset(buffer, 0, 100);
                buffer_index = 0;
            }
            else if (is_symbol(buffer).id != SYMBOL_NULL)
            {
                tl_add(tl, t_new_symbol(is_symbol(buffer).id));
                tl->list[tl->size - 1].args = is_symbol(buffer).args;
            }
            else if (!is_beginning_of_symbol(buffer) && strlen(buffer) > 0)
            {
                char buffer_tmp[100] = {0};
                sprintf(buffer_tmp, "Unknown symbol %s.", buffer);
                error(buffer_tmp);
            }
            buffer_index = 0;
            memset(buffer, 0, 100);
            if (c != ' ')
            {
                buffer[0] = c;
                buffer_index++;
            }
        }
    }
    return tl;
}
/* 
 * This function will convert:
 *  (2+2)4 -> (2+2)*4
 *  4(2+2) -> 4*(2+2)
 *  4func(1,2) -> 4*func(1,2)
 * So it becomes more readable by the do_token_list func.
 */
void make_mul_op_explicit(struct token_list *tl)
{
    for (int i = 0; i < tl->size; i++)
    {
        if (tl->list[i].type == TYPE_TOKEN)
        {
            if (tl->list[i].symbol == SYMBOL_PAR_OPEN)
            {
                if (i > 0)
                {
                    if ((tl->list[i - 1].type == TYPE_VALUE) || (tl->list[i - 1].type == TYPE_TOKEN && tl->list[i - 1].symbol == SYMBOL_PAR_CLOSE))
                    {
                        tl_add_at(tl, t_new_symbol(SYMBOL_MUL), i);
                        i = 0;
                    }
                    if (IS_FUNC(tl->list[i]) && tl->list[i - 1].type == TYPE_VALUE)
                    {
                        tl_add_at(tl, t_new_symbol(SYMBOL_MUL), i);
                        i = 0;
                    }
                }
            }
            else if (tl->list[i].symbol == SYMBOL_PAR_CLOSE)
            {
                if (i < tl->size - 1)
                {
                    if (tl->list[i + 1].type == TYPE_VALUE || IS_FUNC(tl->list[i + 1]))
                    {
                        tl_add_at(tl, t_new_symbol(SYMBOL_MUL), i + 1);
                        i = 0;
                    }
                }
            }
            else if (IS_FUNC(tl->list[i]))
            {
                if (i > 0)
                {
                    if (tl->list[i - 1].type == TYPE_VALUE)
                    {
                        tl_add_at(tl, t_new_symbol(SYMBOL_MUL), i);
                        i = 0;
                    }
                }
            }
        }
    }
}

/*
 *
 * This function is here to check that the token list is valid.
 * It will alllow us to avoid having 4++4 or ((4+4)*3.
 *
 */
void validate_token_list(struct token_list *tl)
{
    if (DEBUG)
    {
        printf("-----------------------------------\n");
        printf("Validating:\n");
        tl_print(tl);
        printf("-----------------------------------\n");
    }
    int indent = 0;
    struct token previous_token;
    struct token next_token;
    for (int i = 0; i < tl->size; i++)
    {
        struct token t = tl->list[i];
        if (i == 0)
        {
            previous_token.type = TYPE_NULL;
        }
        else
        {
            previous_token = tl->list[i - 1];
        }
        if (i == tl->size - 1)
        {
            next_token.type = TYPE_NULL;
        }
        else
        {
            next_token = tl->list[i + 1];
        }
        if (t.type == TYPE_TOKEN)
        {
            if ((is_operator(t) && is_operator(previous_token)) || (is_operator(t) && is_operator(next_token)))
            {
                error("two operators without a value between them.");
            }
            if (t.symbol == SYMBOL_PAR_OPEN)
            {
                indent++;
            }
            if (t.symbol == SYMBOL_PAR_CLOSE)
            {
                indent--;
            }
        }
    }
    if (indent != 0)
    {
        char buffer[100] = {0};
        sprintf(buffer, "%d parenthesis never closed.", indent);
        error(buffer);
    }
    for (int i = 0; i < tl->size; i++)
    {
        struct token t = tl->list[i];
        if (i == 0)
        {
            previous_token.type = TYPE_NULL;
        }
        else
        {
            previous_token = tl->list[i - 1];
        }
        if (i == tl->size - 1)
        {
            next_token.type = TYPE_NULL;
        }
        else
        {
            next_token = tl->list[i + 1];
        }
        if (IS_FUNC(t))
        {
            if (next_token.type == TYPE_NULL)
            {
                error("All functions must have parenthesis after the name.");
            }
            struct token_list *args = tl_get_sub_tl(tl, i + 1);
            int separator_counter = 0;
            for (int j = 0; j < args->size; j++)
            {
                if (args->list[j].type == TYPE_TOKEN && args->list[j].symbol == SYMBOL_ARG_SEPARATOR)
                {
                    separator_counter++;
                }
            }
            if (separator_counter + 1 != t.args)
            {
                error("Error: invalid number of args.");
            }
            tl_free(args);
        }
    }
}

int get_sub_expression_size(struct token_list *tl, int first_parenthesis)
{
    int indent = 0;
    int j;
    for (j = first_parenthesis; j < tl->size; j++)
    {
        struct token t1 = tl->list[j];
        if (t1.type == TYPE_TOKEN)
        {
            if (t1.symbol == SYMBOL_PAR_OPEN)
            {
                indent++;
            }
            if (t1.symbol == SYMBOL_PAR_CLOSE)
            {
                indent--;
            }
            if (t1.symbol == SYMBOL_PAR_CLOSE && indent == 0)
            {
                break;
            }
        }
    }
    int delta = j - first_parenthesis;
    return delta;
}

/*
 *
 * This function isthe core of the calculator.
 * It will manage operations.
 * 
 * Example of action performed:
 * 
 * 4+4*2*(2+2) will become:
 * 1) Manage parenthesis.
 * 4+4*2*4
 * 2) Manage multiplication/division.
 * 4+32
 * 2) Manage addition/substrcation.
 * 36
 *
 */

double do_token_list(struct token_list *tl)
{
    if (DEBUG)
    {
        printf("-----------------------------------\n");
        printf("Round 0.\n");
        tl_print(tl);
        printf("-----------------------------------\n");
    }
    /* 1st round: manage functions. */
    for (int i = 0; i < tl->size; i++)
    {
        struct token t = tl->list[i];
        if (IS_FUNC(t))
        {
            /* Get args */
            struct token_list *args = tl_get_sub_tl(tl, i + 1);
            /* Then we will put them into the stack. */
            struct token stack[10];
            int counter = 0;
            struct token_list *arg_tmp = tl_new();
            for (int j = 0; j < args->size; j++)
            {
                if (args->list[j].symbol != SYMBOL_ARG_SEPARATOR)
                {
                    tl_add(arg_tmp, args->list[j]);
                }
                if (args->list[j].symbol == SYMBOL_ARG_SEPARATOR || j == args->size - 1)
                {
                    stack[counter] = t_new_value(do_token_list(arg_tmp));
                    tl_free(arg_tmp);
                    arg_tmp = tl_new();
                    counter++;
                }
            }
            tl_free(arg_tmp);
            tl_substract(tl, i, args->size + 1);
            int indent = 0;
            int j;
            /* Perform function calls. */
            switch (t.symbol)
            {
            case SYMBOL_SIN:
                tl_add_at(tl, t_new_value(sin(stack[0].value)), i);
                break;
            case SYMBOL_COS:
                tl_add_at(tl, t_new_value(cos(stack[0].value)), i);
                break;
            case SYMBOL_ATAN2:
                tl_add_at(tl, t_new_value(atan2(stack[0].value, stack[1].value)), i);
                break;
            case SYMBOL_TAN:
                tl_add_at(tl, t_new_value(tan(stack[0].value)), i);
                break;
            default:
                break;
            }
            tl_free(args);
        }
    }
    if (DEBUG)
    {
        printf("-----------------------------------\n");
        printf("Round 1.\n");
        tl_print(tl);
        printf("-----------------------------------\n");
    }
    /* 2nd round: manage parenthesis! */

    for (int i = 0; i < tl->size; i++)
    {
        struct token t = tl->list[i];
        if (t.type == TYPE_TOKEN)
        {
            if (t.symbol == SYMBOL_PAR_OPEN)
            {
                struct token_list *tl_tmp = tl_get_sub_tl(tl, i);
                tl_substract(tl, i, tl_tmp->size);
                tl_add_at(tl, t_new_value(do_token_list(tl_tmp)), i);
                tl_free(tl_tmp);
                i = 0;
            }
        }
    }
    if (DEBUG)
    {
        printf("-----------------------------------\n");
        printf("Round 2.\n");
        tl_print(tl);
        printf("-----------------------------------\n");
    }
    /* 3rd round: manage multiplication and division! */
    for (int i = 0; i < tl->size; i++)
    {
        struct token t = tl->list[i];
        if (t.type == TYPE_TOKEN)
        {
            if (t.symbol == SYMBOL_MUL || t.symbol == SYMBOL_DIV)
            {
                struct token prev = tl->list[i - 1];
                struct token next = tl->list[i + 1];
                tl_rm_token_at(tl, i + 1);
                tl_rm_token_at(tl, i);
                tl_rm_token_at(tl, i - 1);
                switch (t.symbol)
                {
                case SYMBOL_MUL:
                    tl_add_at(tl, t_new_value(prev.value * next.value), i - 1);
                    break;
                case SYMBOL_DIV:
                    tl_add_at(tl, t_new_value(prev.value / next.value), i - 1);
                    break;
                default:
                    break;
                }
                i = 0;
            }
        }
    }
    if (DEBUG)
    {
        printf("-----------------------------------\n");
        printf("Round 3.\n");
        tl_print(tl);
        printf("-----------------------------------\n");
    }
    /* 4th round: manage addition and substraction. */
    for (int i = 0; i < tl->size; i++)
    {
        struct token t = tl->list[i];
        if (t.type == TYPE_TOKEN)
        {
            if (t.symbol == SYMBOL_ADD || t.symbol == SYMBOL_SUB)
            {
                /* Manage having expressions such as: -2+4 instead of 4-2 with positive or negative character at the first position. */
                if (i == 0)
                {
                    struct token next = tl->list[i + 1];
                    tl_rm_token_at(tl, i + 1);
                    tl_rm_token_at(tl, i);

                    switch (t.symbol)
                    {
                    case SYMBOL_SUB:
                        tl_add_at(tl, t_new_value(0 - next.value), i);
                        break;
                    case SYMBOL_ADD:
                        tl_add_at(tl, t_new_value(next.value), i);
                        break;
                    default:
                        break;
                    }
                    i = 0;
                }
                else
                {
                    struct token prev = tl->list[i - 1];
                    struct token next = tl->list[i + 1];
                    tl_rm_token_at(tl, i + 1);
                    tl_rm_token_at(tl, i);
                    tl_rm_token_at(tl, i - 1);
                    switch (t.symbol)
                    {
                    case SYMBOL_SUB:
                        tl_add_at(tl, t_new_value(prev.value - next.value), i - 1);
                        break;
                    case SYMBOL_ADD:
                        tl_add_at(tl, t_new_value(prev.value + next.value), i - 1);
                        break;
                    default:
                        break;
                    }
                    i = 0;
                }
            }
        }
    }
    if (DEBUG)
    {
        printf("-----------------------------------\n");
        printf("Round 4.\n");
        tl_print(tl);
        printf("-----------------------------------\n");
    }
    return tl->list[0].value;
}

double do_expression(char *expression)
{
    struct token_list *tl = tokenize(expression);
    make_mul_op_explicit(tl);
    validate_token_list(tl);
    double res = do_token_list(tl);
    tl_free(tl);
    return res;
}

int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        DEBUG = 0;
        printf("%f\n", do_expression(argv[1]));
    }
    else
    {
        DEBUG = 1;
        printf("%f\n", do_expression("sin(1)"));
    }
    tl_check_free_all();
    return 0;
}