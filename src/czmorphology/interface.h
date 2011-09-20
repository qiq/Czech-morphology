#ifndef CZMORPHOLOGY_INTERFACE_H_
#define CZMORPHOLOGY_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize lemmatizer library. This method is not thread-safe.
 * @param prefix Path to dictionary (prefix), e.g. CZ100404a. Three files are
 * required: ${prefix}e.cpd, ${prefix}w.cpd and ${prefix}g.txt.
 * @param guess	Run guesser for unknown words, ${prefix}u.cpd is required.
 * @return Result is 0 or error code.
 */
int lemmatize_init(const char *prefix, int guess);

/**
 * Lemmatize one token, you have to specify several attributes. This method is
 * synchronized (one thread may enter it at a time), so it is thread-safe.
 * @param token Token to lemmatize.
 * @param is_punct Whether the token is a punctuation or a word.
 * @param is_abbr Whether the token is abbreviation.
 * @param is_number Whether the token is in fact a number.
 * @param dot_follows Whether next token is a dot (possibly end of sentence).
 * @param hyphen_follows Whether next token is a hyphen.
 * @param guessed In case of non-NULL, 1 is set when lemmatization was guessed.
 * @return Tab-separated list of lemmas, every lemma is followed by a list of
 * tags (separated by spaces).
 */
char *lemmatize_token(const char *token, int is_punct, int is_abbr, int
is_number, int dot_follows, int hyphen_follows, int *guessed);

/**
 * Deallocates all memory and finishes lemmatizer. This method is not
 * thread-safe.
 */
void lemmatize_destroy();

#ifdef __cplusplus
}
#endif

#endif
