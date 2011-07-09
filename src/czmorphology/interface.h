#ifndef _INTERFACE_H_
#define _INTERFACE_H_

/* use unknown_rules=NULL to disable guesser */
int lemmatize_init(const char *dictionary, const char *tag_table, const char *unknown_rules);
char *lemmatize_token(const char *token, int is_punct, int is_abbr, int is_number, int dot_follows, int hyphen_follows);
void lemmatize_destroy();

#endif
