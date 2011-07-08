#ifndef _INTERFACE_H_
#define _INTERFACE_H_

int lemmatize_init(const char *dictionary, const char *unknown_rules, const char *tag_table);
char *lemmatize_token(const char *token);
void lemmatize_destroy();

#endif
