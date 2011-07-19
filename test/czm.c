#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "czmorphology/interface.h"

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "usage: test <data_prefix>\n");
		exit(1);
	}
	int result = lemmatize_init(argv[1], 1);
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
		if (result) {
			puts(result);
			free(result);
		} else {
			printf("error processing: %s\n", s);
		}
	}
	lemmatize_destroy();
	return 0;
}
