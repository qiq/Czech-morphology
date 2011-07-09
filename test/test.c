#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "czmorph/interface.h"

int main() {
	int result = lemmatize_init("../data/CZ100404a", "../data/CZ100404au.cpd", "../data/CZ100404ag.txt");
	if (result != 0) {
		fprintf(stderr, "error: %d\n", result);
		exit(1);
	}
	char s[1024];
	while (fgets(s, sizeof(s), stdin)) {
		if (*s && s[strlen(s)-1] == '\n')
			s[strlen(s)-1] = '\0';
		int punct = 0;
		int number = 0;
		char *p = s;
		while (*p) {
			if (ispunct(*p))
				punct = 1;
			else if (isdigit(*p))
				number = 1;
			p++;
		}
		char *result = lemmatize_token(s, punct, 0, number, 0, 0);
		puts(result);
		free(result);
	}
	lemmatize_destroy();
	return 0;
}
