#include <stdio.h>
#include "token.h"

void t_print(struct token t)
{
    if (t.type == TYPE_VALUE)
    {
        printf("%f", t.value);
    }
    else
    {
        for (int j = 0; j < AMOUNT_OF_SYMBOLS; j++)
        {
            if (SYMBOLS[j].id == t.symbol)
            {
                printf("%s", SYMBOLS[j].str);
            }
        }
    }
}
struct token t_new_value(double v)
{
    struct token t;
    t.type = TYPE_VALUE;
    t.value = v;
    return t;
}
struct token t_new_symbol(int id)
{
    struct token t;
    t.type = TYPE_TOKEN;
    t.symbol = id;
    return t;
}
struct token t_new_from_str(char *str)
{
}