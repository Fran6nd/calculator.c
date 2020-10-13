#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

#include "token_list.h"

int TL_COUNTER = 0;

struct token_list *tl_new()
{
    struct token_list *tl;
    tl = malloc(sizeof(struct token_list));
    tl->list = NULL;
    tl->size = 0;
    TL_COUNTER++;
    return tl;
}

void tl_free(struct token_list *tl)
{
    if (tl != NULL)
    {
        if (tl->size != 0)
            free(tl->list);
        free(tl);
        TL_COUNTER--;
    }
}
void tl_add(struct token_list *tl, struct token t)
{
    if (tl->size == 0)
    {
        tl->list = malloc(sizeof(struct token));
        tl->size++;
    }
    else
    {
        tl->size++;
        tl->list = (struct token *)realloc(tl->list, tl->size * sizeof(struct token));
    }
    tl->list[tl->size - 1] = t;
}
void tl_add_at(struct token_list *tl, struct token t, int index)
{
    if (tl->size >= 0 && index <= tl->size)
    {
        tl->size++;
        tl->list = (struct token *)realloc(tl->list, tl->size * sizeof(struct token));
        for (int i = tl->size - 1; i > index; i--)
        {
            tl->list[i] = tl->list[i - 1];
        }
        tl->list[index] = t;
    }
    else
    {
        /* Error */
        printf("ERr\n");
    }
}
struct token_list *tl_get_sub_tl(struct token_list *tl, int first_parenthesis)
{
    struct token_list *tl1 = tl_new();
    int indent = 0;
    int j;
    for (j = first_parenthesis; j < tl->size; j++)
    {
        struct token t1 = tl->list[j];

            tl_add(tl1, t1);
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
    tl_rm_token_at(tl1, 0);
    tl_rm_token_at(tl1, tl1->size-1);
    return tl1;
}
/* BUG SOMEWHERE */
void tl_rm_sub_tl(struct token_list *tl, int first_parenthesis)
{
    int indent = 0;
    int j;
    for (j = first_parenthesis; j < tl->size; j++)
    {
        struct token t1 = tl->list[j];
        if (t1.type == TYPE_TOKEN)
        {
            if (t1.symbol == SYMBOL_PAR_CLOSE)
            {
                indent++;
            }
            if (t1.symbol == SYMBOL_PAR_OPEN)
            {
                indent--;
            }
            if (t1.symbol == SYMBOL_PAR_OPEN && indent == 0)
            {
                break;
            }
        }
    }
    for (int i = j; i >= first_parenthesis; i--)
    {
        tl_rm_token_at(tl, i);
    }
}
void tl_substract(struct token_list *tl, int index, int size)
{
    for (int i = index + size +1; i >= index; i--)
    {
        tl_rm_token_at(tl, i);
    }
}
void tl_rm_token_at(struct token_list *tl, int index)
{
    for (int i = index; i < tl->size; i++)
    {
        tl->list[i] = tl->list[i + 1];
    }
    tl->size--;
    tl->list = (struct token *)realloc(tl->list, tl->size * sizeof(struct token));
}
void tl_check_free_all()
{
    if (TL_COUNTER != 0)
    {
        printf("Error: %d token list remaining active.", TL_COUNTER);
    }
}
void tl_print(struct token_list *tl)
{
    for (int i = 0; i < tl->size; i++)
    {
        t_print(tl->list[i]);
    }
    printf("\n");
}