#ifndef _INTERFACE_H_
#define _INTERFACE_H_

int lemmatize_init(const char *dictionary, const char *unknown_rules, const char *tag_table);
char *lemmatize_token(const char *token, int is_punct, int is_abbr, int is_number, int dot_follows, int hyphen_follows);
void lemmatize_destroy();

#endif
