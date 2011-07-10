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
	char prefix[1024*10];
	char tagtable[1024*10];
	char unknown[1024*10];
	snprintf(prefix, sizeof(prefix), "%s", argv[1]);
	snprintf(tagtable, sizeof(tagtable), "%sg.txt", argv[1]);
	snprintf(unknown, sizeof(unknown), "%su.cpd", argv[1]);
	int result = lemmatize_init(prefix, tagtable, unknown);
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