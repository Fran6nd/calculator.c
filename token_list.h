#ifndef TOKEN_LIST_H
#define TOKEN_LIST_H

#include "token.h"

struct token_list
{
    struct token *list;
    int size;
};
struct token_list *tl_new();
void tl_free(struct token_list *tl);
void tl_add(struct token_list *tl, struct token t);
void tl_add_at(struct token_list *tl, struct token t, int index);
struct token_list *tl_get_sub_tl(struct token_list *tl, int index);
void tl_rm_sub_tl(struct token_list *tl, int index);
void tl_substract(struct token_list *tl, int index, int size);
void tl_rm_token_at(struct token_list *tl, int index);
void tl_check_free_all();
void tl_print(struct token_list *tl);

#endif