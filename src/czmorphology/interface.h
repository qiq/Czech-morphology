#ifndef _INTERFACE_H_
#define _INTERFACE_H_

/**
  Initialize lemmatizer library. This method is not thread-safe.
  @dictionary	Path to dictionary (prefix), e.g. CZ100404a. Two files are
		required: *e.cpd *w.cpd.
  @tag_table	Path to tag table file, e.g. CZ100404ag.txt
  @unknown	Path to the unknown words (guesser) rules, e.g.
		CZ100404au.cpd. Specify NULL to disable guesser.
  @return	Result is 0 or error code.
*/
int lemmatize_init(const char *dictionary, const char *tag_table, const char *unknown);

/**
  Lemmatize one token, you have to specify several attributes. This method is
  synchronized (one thread may enter it at a time), so it is thread-safe.
  @token	Token to lemmatize.
  @is_punct	Whether the token is a punctuation or a word.
  @is_abbr	Whether the token is abbreviation.
  @is_number	Whether the token is in fact a number.
  @dot_follows	Whether next token is a dot (possibly end of sentence).
  @hyphen_follows	Whether next token is a hyphen.
*/
char *lemmatize_token(const char *token, int is_punct, int is_abbr, int is_number, int dot_follows, int hyphen_follows);

/**
  Deallocates all memory and finishes lemmatizer. This method is not
  thread-safe.
*/
void lemmatize_destroy();

#endif
