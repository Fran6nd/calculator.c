#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#define TYPE_VALUE 1
#define TYPE_TOKEN 2
#define TYPE_NULL 3

#define SYMBOL_NULL 0x00
#define SYMBOL_ADD 0x11
#define SYMBOL_SUB 0x12
#define SYMBOL_DIV 0x13
#define SYMBOL_MUL 0x14
#define SYMBOL_PERCENT 0x15
#define SYMBOL_POW 0x16
#define SYMBOL_PAR_OPEN 0x21
#define SYMBOL_PAR_CLOSE 0x22
#define SYMBOL_ARG_SEPARATOR 0x23
#define SYMBOL_COS 0x31
#define SYMBOL_SIN 0X32
#define SYMBOL_ATAN2 0X33

#define AMOUNT_OF_SYMBOLS ((int)(sizeof(SYMBOLS) / sizeof(SYMBOLS[0])))

#define IS_OPERATOR(op) (op.type == TYPE_TOKEN && ((op.symbol & 0x10) == 0x10))
#define IS_SYMBOL(op) (op.type == TYPE_TOKEN && ((op.symbol & 0x20) == 0x20))
#define IS_FUNC(op) (op.type == TYPE_TOKEN && ((op.symbol & 0x30) == 0x30))

struct symbol
{
    const char *str;
    const int id;
    const int args;
};

const struct symbol SYMBOLS[] = {
    {"+", SYMBOL_ADD, 0},
    {"-", SYMBOL_SUB, 0},
    {"*", SYMBOL_MUL, 0},
    {"/", SYMBOL_DIV, 0},
    {"%", SYMBOL_PERCENT, 0},
    {"^", SYMBOL_POW, 0},
    {"(", SYMBOL_PAR_OPEN, 0},
    {")", SYMBOL_PAR_CLOSE, 0},
    {",", SYMBOL_ARG_SEPARATOR, 0},
    {"cos", SYMBOL_COS, 1},
    {"sin", SYMBOL_SIN, 1},
    {"atan2", SYMBOL_ATAN2, 2},
};

int DEBUG = 0;

struct token
{
    int type;
    union
    {
        double value;
        int symbol;
        int args;
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

int tokenize(char *expression, struct token *tlist)
{
    char buffer[100] = {0};
    int buffer_index = 0;
    int token_index = 0;
    char c;
    for (unsigned int i = 0; i <= strlen(expression); i++)
    {
        if (i <= strlen(expression))
        {
            c = expression[i];

            buffer[buffer_index] = c;
            buffer_index++;
        }

        if ((is_symbol(buffer).id == SYMBOL_NULL && !is_string_number(buffer) && !is_beginning_of_symbol(buffer)) || i == strlen(expression))
        {
            //printf("Not symbol and not num: %s\n", buffer);
            if (i < strlen(expression))
            {
                buffer_index--;
                buffer[buffer_index] = 0;
                //next_char = c;
            }
            if (is_string_number(buffer))
            {
                //printf("Num: %s\n", buffer);
                tlist[token_index].type = TYPE_VALUE;
                tlist[token_index].value = atof(buffer);
                memset(buffer, 0, 100);
                buffer_index = 0;
                token_index++;
            }

            else if (is_symbol(buffer).id != SYMBOL_NULL)
            {
                //printf("Symbol: %s\n", buffer);
                tlist[token_index].type = TYPE_TOKEN;
                tlist[token_index].symbol = is_symbol(buffer).id;
                token_index++;
            }
            else if (!is_beginning_of_symbol(buffer) && strlen(buffer) > 0)
            {
                char buffer_tmp[100] = {0};
                sprintf(buffer_tmp, "Unknown symbol %s.", buffer);
                error(buffer_tmp);
            }
            buffer_index = 0;
            memset(buffer, 0, 100);
            buffer[buffer_index] = c;
            buffer_index++;
        }
    }
    return token_index;
}
/* 
 * This function will convert:
 *  (2+2)4 -> (2+2)*4
 *  4(2+2) -> 4*(2+2)
 * So it becomes more readable by the do_token_list func.
 */
int make_mul_op_explicit(struct token *token_list, int token_count)
{
    for (int i = 0; i < token_count; i++)
    {
        if (token_list[i].type == TYPE_TOKEN)
        {
            if (token_list[i].symbol == SYMBOL_PAR_OPEN)
            {
                if (i > 0)
                {
                    if ((token_list[i - 1].type == TYPE_VALUE) || (token_list[i - 1].type == TYPE_TOKEN && token_list[i - 1].symbol == SYMBOL_PAR_CLOSE))
                    {
                        token_count++;
                        for (int j = token_count - 1; j > i; j--)
                        {
                            token_list[j] = token_list[j - 1];
                        }
                        token_list[i].symbol = SYMBOL_MUL;
                        i = 0;
                    }
                    if (IS_FUNC(token_list[i]) && token_list[i - 1].type == TYPE_VALUE)
                    {
                        token_count++;
                        for (int j = token_count - 1; j > i; j--)
                        {
                            token_list[j] = token_list[j - 1];
                        }
                        token_list[i].symbol = SYMBOL_MUL;
                        i = 0;
                    }
                }
            }
            else if (token_list[i].symbol == SYMBOL_PAR_CLOSE)
            {
                if (i < token_count - 1)
                {
                    if (token_list[i + 1].type == TYPE_VALUE)
                    {
                        token_count++;
                        for (int j = token_count - 1; j > i; j--)
                        {
                            token_list[j] = token_list[j - 1];
                        }
                        token_list[i + 1].symbol = SYMBOL_MUL;
                        i = 0;
                    }
                }
            }
        }
    }
    return token_count;
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
            for (int j = 0; j < AMOUNT_OF_SYMBOLS; j++)
            {
                if (SYMBOLS[j].id == token_list[i].symbol)
                {
                    printf("%s", SYMBOLS[j].str);
                }
            }
        }
    }
    printf("\n");
}

/*
 *
 * This function is here to check that the token list is valid.
 * It will alllow us to avoid having 4++4 or ((4+4)*3.
 *
 */
void validate_token_list(struct token *token_list, int token_count)
{
    if (DEBUG)
    {
        printf("-----------------------------------\n");
        printf("Validating:\n");
        print_token_list(token_list, token_count);
        printf("-----------------------------------\n");
    }
    int indent = 0;
    struct token previous_token;
    struct token next_token;
    for (int i = 0; i < token_count; i++)
    {
        struct token t = token_list[i];
        if (i == 0)
        {
            previous_token.type = TYPE_NULL;
        }
        else
        {
            previous_token = token_list[i - 1];
        }
        if (i == token_count - 1)
        {
            next_token.type = TYPE_NULL;
        }
        else
        {
            next_token = token_list[i + 1];
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
    /* atan2 ( 2 , 2 ) -> atan2 2 2 */
    for (int i = 0; i < token_count; i++)
    {
        struct token t = token_list[i];
        if(IS_FUNC(t)){

        }
    }
}

int get_sub_expression_size(struct token *token_list,int token_count, int first_parenthesis)
{
    int indent = 0;
    int j;
    for (j = first_parenthesis; j < token_count; j++)
    {
        struct token t1 = token_list[j];
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

double do_token_list(struct token *token_list, int token_count)
{
    if (DEBUG)
    {
        printf("-----------------------------------\n");
        printf("Round 0.\n");
        print_token_list(token_list, token_count);
        printf("-----------------------------------\n");
    }
    /* 1st round: manage functions. */
    for (int i = 0; i < token_count; i++)
    {
        struct token t = token_list[i];
        if (IS_FUNC(t))
        {
            /* Get args */
            int indent = 0;
            int j;
            /* Make the operation. */
            switch (t.symbol)
            {
            case SYMBOL_SIN:
                /* code */
                break;
            case SYMBOL_COS:
                /* code */
                break;
            case SYMBOL_ATAN2:
                /* code */
                break;
            default:
                break;
            }
        }
    }
    if (DEBUG)
    {
        printf("-----------------------------------\n");
        printf("Round 1.\n");
        print_token_list(token_list, token_count);
        printf("-----------------------------------\n");
    }
    /* 2nd round: manage parenthesis! */
    for (int i = 0; i < token_count; i++)
    {
        struct token t = token_list[i];
        if (t.type == TYPE_TOKEN)
        {
            if (t.symbol == SYMBOL_PAR_OPEN)
            {

                int delta = get_sub_expression_size(token_list, token_count, i);
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
    if (DEBUG)
    {
        printf("-----------------------------------\n");
        printf("Round 2.\n");
        print_token_list(token_list, token_count);
        printf("-----------------------------------\n");
    }
    /* 3rd round: manage multiplication and division! */
    for (int i = 0; i < token_count; i++)
    {
        struct token t = token_list[i];
        if (t.type == TYPE_TOKEN)
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
    if (DEBUG)
    {
        printf("-----------------------------------\n");
        printf("Round 2.\n");
        print_token_list(token_list, token_count);
        printf("-----------------------------------\n");
    }
    /* 4th round: manage addition and substraction. */
    for (int i = 0; i < token_count; i++)
    {
        struct token t = token_list[i];
        if (t.type == TYPE_TOKEN)
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
    if (DEBUG)
    {
        printf("-----------------------------------\n");
        printf("Round 3.\n");
        print_token_list(token_list, token_count);
        printf("-----------------------------------\n");
    }
    return token_list[0].value;
}

double do_expression(char *expression)
{
    struct token token_list[100];
    int token_count = tokenize(expression, token_list);
    token_count = make_mul_op_explicit(token_list, token_count);
    validate_token_list(token_list, token_count);
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
        printf("%f\n", do_expression("atan2(0,-1)"));
    }
    return 0;
}