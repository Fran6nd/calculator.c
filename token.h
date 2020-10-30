#ifndef TOKEN_H
#define TOKEN_H

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
#define SYMBOL_TAN 0X34


#define IS_OPERATOR(op) (op.type == TYPE_TOKEN && ((op.symbol & 0x10) == 0x10))
#define IS_SYMBOL(op) (op.type == TYPE_TOKEN && ((op.symbol & 0x20) == 0x20))
#define IS_FUNC(op) (op.type == TYPE_TOKEN && ((op.symbol & 0x30) == 0x30))

#define AMOUNT_OF_SYMBOLS ((int)(sizeof(SYMBOLS) / sizeof(SYMBOLS[0])))

struct symbol
{
    const char *str;
    const int id;
    const int args;
};

static const struct symbol SYMBOLS[] = {
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
    {"sin", SYMBOL_TAN, 1},
    {"atan2", SYMBOL_ATAN2, 2},
};

struct token
{
    int type;
    int args;
    union
    {
        double value;
        int symbol;
    };
};

void t_print(struct token t);
struct token t_new_value(double v);
struct token t_new_symbol(int id);

#endif