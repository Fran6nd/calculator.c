#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <memory.h>

#define TYPE_VALUE 1
#define TYPE_OPERATOR 2

#define SYMBOL_ADD 1
#define SYMBOL_SUB 2
#define SYMBOL_DIV 3
#define SYMBOL_MUL 4
#define SYMBOL_PAR_OPEN 5
#define SYMBOL_PAR_CLOSE 6

int DEBUG = 0;

struct token
{
    int type;
    union
    {
        float value;
        int symbol;
    };
};

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
int is_char_numeric(char c)
{
    if (c >= 60 && c <= 71)
    {
        return 1;
    }
    return 0;
}
int is_symbol(char *c)
{
    if (strcmp(c, "-") == 0 || strcmp(c, "+") == 0 || strcmp(c, "*") == 0 || strcmp(c, "/") == 0 || strcmp(c, "(") == 0 || strcmp(c, ")") == 0)
        return 1;
    return 0;
}

int tokenize(char *expression, struct token *tlist)
{
    char buffer[100] = {0};
    int buffer_index = 0;
    int token_index = 0;
    char previous_char = '+';
    char next_char = 0;
    for (int i = 0; i < strlen(expression); i++)
    {
        char c = expression[i];
        if (i < strlen(expression) - 1)
        {
            next_char = expression[i + 1];
        }
        else
        {
            next_char = 0;
        }
        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')')
        {
            if (strlen(buffer) > 0)
            {
                tlist[token_index].type = TYPE_VALUE;
                tlist[token_index].value = atof(buffer);
                memset(buffer, 0, 100);
                buffer_index = 0;
                token_index++;
            }
            tlist[token_index].type = TYPE_OPERATOR;
            switch (c)
            {
            case '+':
                tlist[token_index].symbol = SYMBOL_ADD;
                break;
            case '-':
                tlist[token_index].symbol = SYMBOL_SUB;
                break;
            case '*':
                tlist[token_index].symbol = SYMBOL_MUL;
                break;
            case '/':
                tlist[token_index].symbol = SYMBOL_DIV;
                break;
            case '(':
                /* We add the symbol * if needed. (2)(2) will become (2)*(2). */
                if (!is_char_operator(previous_char) && previous_char != '(')
                {
                    tlist[token_index].symbol = SYMBOL_MUL;
                    token_index++;
                    tlist[token_index].type = TYPE_OPERATOR;
                }
                tlist[token_index].symbol = SYMBOL_PAR_OPEN;
                break;
            case ')':
                tlist[token_index].symbol = SYMBOL_PAR_CLOSE;
                /* Here we transform (4x4)5 so it becomes (4x4)*5. */
                if (!is_char_operator(next_char) && next_char != 0 && next_char != '(' && next_char != ')')
                {
                    token_index++;
                    tlist[token_index].type = TYPE_OPERATOR;
                    tlist[token_index].symbol = SYMBOL_MUL;
                }
                break;
            default:
                break;
            }
            token_index++;
        }
        else if (i == strlen(expression) - 1)
        {
            buffer[buffer_index] = c;
            buffer_index++;
            tlist[token_index].type = TYPE_VALUE;
            tlist[token_index].value = atof(buffer);
            memset(buffer, 0, 100);
            buffer_index = 0;
            token_index++;
        }
        else
        {
            buffer[buffer_index] = c;
            buffer_index++;
        }
        previous_char = c;
    }
    return token_index;
}

void print_token_list(struct token *token_list, int token_count)
{
    for (int i = 0; i < token_count; i++)
    {
        struct token t = token_list[i];
        if (t.type == TYPE_VALUE)
        {
            printf("%f", t.value);
        }
        else
        {
            switch (t.symbol)
            {
            case SYMBOL_PAR_CLOSE:
                printf(")");
                break;
            case SYMBOL_PAR_OPEN:
                printf("(");
                break;
            case SYMBOL_ADD:
                printf("+");
                break;
            case SYMBOL_SUB:
                printf("-");
                break;
            case SYMBOL_MUL:
                printf("*");
                break;
            case SYMBOL_DIV:
                printf("/");
                break;
            default:
                break;
            }
        }
    }
    printf("\n");
}

float do_token_list(struct token *token_list, int token_count)
{
    if (DEBUG)
    {
        print_token_list(token_list, token_count);
    }
    /* First round: manage parenthesis! */
    for (int i = 0; i < token_count; i++)
    {
        struct token t = token_list[i];
        if (t.type == TYPE_OPERATOR)
        {
            if (t.symbol == SYMBOL_PAR_OPEN)
            {
                int indent = 0;
                int j;
                for (j = i; j < token_count; j++)
                {
                    struct token t1 = token_list[j];
                    if (t1.type == TYPE_OPERATOR)
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
                if (indent > 0)
                {
                    char buffer[100] = {0};
                    sprintf(buffer, "%d parenthesis never closed.", indent);
                    error(buffer);
                }
                int delta = j - i;
                struct token sub_token_list[delta - 1];
                for (int k = 0; k < delta - 1; k++)
                {
                    sub_token_list[k] = token_list[i + k + 1];
                }
                token_list[i].type = TYPE_VALUE;
                token_list[i].value = do_token_list(sub_token_list, delta - 1);
                for (int k = i + 1; k < token_count - delta; k++)
                {
                    token_list[k] = token_list[k + delta];
                }
                token_count = token_count - delta;
                i = 0;
            }
        }
    }
    /* 2nd round: manage multiplication and division! */
    for (int i = 0; i < token_count; i++)
    {
        struct token t = token_list[i];
        if (t.type == TYPE_OPERATOR)
        {
            if (t.symbol == SYMBOL_MUL || t.symbol == SYMBOL_DIV)
            {
                switch (t.symbol)
                {
                case SYMBOL_MUL:
                    token_list[i - 1].value = token_list[i - 1].value * token_list[i + 1].value;
                    break;
                case SYMBOL_DIV:
                    token_list[i - 1].value = token_list[i - 1].value / token_list[i + 1].value;
                    break;
                default:
                    break;
                }
                for (int j = i + 2; j < token_count; j++)
                {
                    token_list[j - 2] = token_list[j];
                }
                token_count -= 2;
                i = 0;
            }
        }
    }
    /* 3rd round: manage addition and substraction. */
    for (int i = 0; i < token_count; i++)
    {
        struct token t = token_list[i];
        if (t.type == TYPE_OPERATOR)
        {
            if (t.symbol == SYMBOL_ADD || t.symbol == SYMBOL_SUB)
            {
                /* Manage having expressions such as: -2+4 instead of 4-2 with positive or negative character at the first position. */
                if (i == 0)
                {
                    switch (t.symbol)
                    {
                    case SYMBOL_SUB:
                        token_list[i].value = 0 - token_list[i + 1].value;
                        break;
                    case SYMBOL_ADD:
                        token_list[i].value = token_list[i + 1].value;
                        break;
                    default:
                        break;
                    }
                    /* erase the case containing the token at i+1. */
                    for (int j = i + 2; j < token_count; j++)
                    {
                        token_list[j - 1] = token_list[j];
                    }
                    token_count -= 1;
                    i = 0;
                }
                else
                {
                    switch (t.symbol)
                    {
                    case SYMBOL_SUB:
                        token_list[i - 1].value = token_list[i - 1].value - token_list[i + 1].value;
                        break;
                    case SYMBOL_ADD:
                        token_list[i - 1].value = token_list[i - 1].value + token_list[i + 1].value;
                        break;
                    default:
                        break;
                    }
                    for (int j = i + 2; j < token_count; j++)
                    {
                        token_list[j - 2] = token_list[j];
                    }
                    token_count -= 2;
                    i = 0;
                }
            }
        }
    }
    return token_list[0].value;
}

float do_expression(char *expression)
{
    struct token token_list[100];
    int token_count = tokenize(expression, token_list);
    return do_token_list(token_list, token_count);
}

int main(int argc, char *argv[])
{
    /* Print 62.5. */
    if (argc == 2)
    {
        printf("%f\n", do_expression(argv[1]));
    }
    else
    {
        DEBUG = 1;
        printf("%f\n", do_expression("(4*(2*2))"));
    }
    return 0;
}