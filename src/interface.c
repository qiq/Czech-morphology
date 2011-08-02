#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <iconv.h>
#include <ctype.h>
#include "hb_base.h"
#include "hb_hash.h"
#include "hb_cpd.h"
#include "hh_cp.h"
#include "hf_modd.h"

typedef struct unkRec { /* unknown word handling based on b4023 data */
  hb_cpdRecType *pcpdUnk; /* unknown word pref/suff file */
  int cPrefMax; /* max prefix length */
  int cSuffMax; /* max suffix length */
  char szLF[HB_LONGSTRINGMAX]; /* lowest level default LT rule */
  int cMMt; /* size of MMt table */
  char **rgszMMt; /* MMt table; allocated if file present */
  int cLRT; /* size of LRT table */
  char **rgszLRT; /* LRT table; allocated if file present */
  char szFullData[HB_LONGSTRINGMAX]; /* for direct data, such as mfq tag */
} unkRecType;

typedef struct parRec {
  int fRoot; /* output root within a separate tag: MPHOUT_SEG_ROOT */
  int fEnding; /* output ending within a separate tag: MPHOUT_SEG_OUT */
  int fAllTags; /* output all tags into a unique set, from morphology,
                    AMBCLASSTAG */
  int fLemmaTagUpdate; /* output lemma/tag according to input lemma/tag:
                    disambiguate if possible
                    do it for lines containing input lemma/tag,
                    otherwise normal output
                    */
  int fForceLemmaUpdate; /* active only if fLemmaTagUpdate:
                    force updated lemma output (<l>) even in not found
                    in morphology output: take first lemma from <MMl>
                    */
  int fReportLTU; /* report if update req. but no lemma in input */
  int fMDCopy; /* if set & if MDl/MDt tags in input, copy them to output;
                  otherwise no MDl/MDt output even if in input */
  char szVarDelAlways[HB_STRINGMAX]; 
             /* always delete variant letters (digits) */
  char szVarDelIfExtra[HB_STRINGMAX]; 
             /* del variant letters (digits) if normal tags */
  char szLexDelAlways[HB_STRINGMAX]; 
             /* always delete lemma for sss letters (digits) */
  char szLexDelIfExtra[HB_STRINGMAX];
             /* del lemma if sss let. (digits) if other res */
  char szStyleOut[HB_STRINGMAX]; 
             /* string of letters to output after style tag */
  char szStyleSep[HB_STRINGMAX]; /* style "tag" */
  char szSemOut[HB_STRINGMAX]; /* string of letters to output after sem tag */
  char szSemSep[HB_STRINGMAX]; /* semantics abbr "tag" */
  char szSyntOut[HB_STRINGMAX];  
             /* string of letters to output after synt tag */
  char szSyntSep[HB_STRINGMAX];  /* synt abbr "tag" */
  char szDescSep[HB_STRINGMAX];  /* desciption info "tag" */
  char szDescOut[HB_STRINGMAX]; /* TMP description output? y, yes, 1 */
  int fDescOut; /* desc output flag */
  char szPatternsOut[HB_STRINGMAX]; /* pattern [pairs] out? y, yes, 1 */
  int fPatternsOut; /* patetrn name pair output flag */
  /* caching: */
  char szBaseCache[HB_STRINGMAX]; /* initial cache file name */
  hb_hashRecType *phashBaseCache; /* base hash table, data: result directly! */
  char szCacheEntries[HB_STRINGMAX]; /* cache size in lines from arg file */
  longint clCacheEntries; /* dtto in long int representaton; */
  longint ilCachedResultFree;
  char **rgszCachedResults; /* size: lCacheEntries., allocated dyn. */
                      /* strings to be output as pszOutput in lemmatize */
  hb_hashRecType *phashCache; /* hash table of input strings, data: index
                                 to rgszResults to #of results */
  longint clCacheQueries;
  longint clBaseCacheHits;
  longint clCacheHits;
  /* input/output code page (charset): */
  char szCharSet[HB_STRINGMAX]; /* supported: iso-8859-2, cp895 */
  char szXFFile[HB_STRINGMAX]; /* filename for X-tagged output */
  FILE *fXF;  /* for X-tagged output */
  char szTagTable[HB_STRINGMAX]; /* filename for output tag conversion table */
  hb_hashRecType *phashTagTable; /* tag hash table */
  hb_hashRecType *phashInvTagTable; /* tag hash table, inverted */
  unkRecType *punk; /* unknown word handling structure pointer */
  tlangEnvRec langEnvDic; /* language env. of dictionary, incl. prefixes */
} parRecType;

extern hb_listszRecType *hb_listszFreeHead;
extern hb_llszRecType *hb_llszFreeHead;

extern parRecType *pparMain;
extern char szLogName[];
extern char szDictionary[];
extern char szNFName[];
extern FILE *fNF;
extern FILE *fLog;
extern char szFormIn[];
extern char *pszOutput;

pthread_mutex_t mutex;
iconv_t handle_ui;
iconv_t handle_iu;

int csts_decode_init();

// not thread safe
int lemmatize_init(const char *prefix, int guess) {
	char *dictionary = prefix;
	char *tag_table = malloc(strlen(prefix)+5+1);
	strcpy(tag_table, prefix);
	strcat(tag_table, "g.txt");
	char *unknown_rules = NULL;
	if (guess) {
		unknown_rules = malloc(strlen(prefix)+5+1);
		strcpy(unknown_rules, prefix);
		strcat(unknown_rules, "u.cpd");
	}

	pthread_mutex_init(&mutex, NULL);

	*szLogName = '\0';

	if (NULL == (pparMain = (parRecType *) hb_LongAlloc(sizeof(parRecType)))) {
		//fprintf(stderr, "%s: Cannot allocate memory for parameter record\n",szPN);
		return 204;
	}

	strncpy(szDictionary, dictionary, HB_STRINGMAX-2);

	*szNFName = '\0';

	strncpy(pparMain->szCharSet, "iso-8859-2", HB_STRINGMAX-2);

	if (hhSetTabFillInv(pparMain->szCharSet) != 0) {
		// fprintf(stderr,"%s: cannot init code table (code %s)\n", pparMain->szCharSet);
		return 377;
	}

	*pparMain->szVarDelAlways = '\0';

	*pparMain->szVarDelIfExtra = '\0';

	*pparMain->szLexDelAlways = '\0';

	*pparMain->szLexDelIfExtra = '\0';

	pparMain->fLemmaTagUpdate = 0;

	pparMain->fForceLemmaUpdate = 0;

	pparMain->fMDCopy = 1;

	pparMain->fReportLTU = 0;

	pparMain->fAllTags = 0;

	pparMain->fRoot = 0;

	pparMain->fEnding = 0;

	strcpy(pparMain->szDescOut, "y");
	pparMain->fDescOut = 1;

	strcpy(pparMain->szDescSep, "^");

	strcpy(pparMain->szPatternsOut, "NO");
	pparMain->fPatternsOut = 0;

	strcpy(pparMain->szSyntOut, "_W_T_B");

	strcpy(pparMain->szSyntSep, ":");

	strcpy(pparMain->szSemOut, "_j_g_c_y_b_u_w_p_z_o_G_Y_S_E_R_K_m_H_U_L");

	strcpy(pparMain->szSemSep, ";");

	strcpy(pparMain->szStyleOut, "_x_s_a_n_h_e_l_v_t");

	strcpy(pparMain->szStyleSep, ",");

	strcpy(pparMain->szCacheEntries, "20000");
	pparMain->clCacheEntries = atoi(pparMain->szCacheEntries);

	*(pparMain->szBaseCache) = HB_EOS;

	pparMain->punk = NULL;
	if (unknown_rules) {
		/* parameter found */
		pparMain->punk = (unkRecType *) hb_LongAlloc(sizeof(unkRecType));
		if (pparMain->punk == NULL) {
			// fprintf(stderr,"%s: cannot allocate space for unk record\n",szPN);
			return 235;
		}
		pparMain->punk->pcpdUnk = hb_CpdOpen((char*)unknown_rules);
		if (pparMain->punk->pcpdUnk == NULL) {
			// fprintf(stderr,"%s: cannot open unknown word rules file %s\n", szPN,szT);
			return 234;
		}
		/* read basic parameters here: */

		char szKeyC[HB_STRINGMAX];
		char szKey[HB_STRINGMAX];
		char szData[HB_LONGSTRINGMAX];

		strcpy(szKey,"^PREFMAX^");
		hb_CpdCvs(szKey,szKeyC);
		int iPattern;
		int fCFL;
		if (hb_CpdFindFirst(pparMain->punk->pcpdUnk,szKeyC, szData, HB_LONGSTRINGMAX-2, &iPattern, &fCFL) != 0) {
			// fprintf(stderr, "%s: error in hb_CpdFindFirst(%s), key = %s\n", szPN,pparMain->punk->pcpdUnk->szFileName,szKey);
			return 236;
		}
		pparMain->punk->cPrefMax = atoi(szData);
		strcpy(szKey,"^SUFFMAX^");
		hb_CpdCvs(szKey,szKeyC);
		if (hb_CpdFindFirst(pparMain->punk->pcpdUnk,szKeyC, szData,HB_LONGSTRINGMAX-2, &iPattern, &fCFL) != 0) {
			// fprintf(stderr, "%s: error in hb_CpdFindFirst(%s), key = %s\n", szPN,pparMain->punk->pcpdUnk->szFileName,szKey);
			return 238;
		}
		pparMain->punk->cSuffMax = atoi(szData);
		strcpy(szKey,"^LF^");
		hb_CpdCvs(szKey,szKeyC);
		if (hb_CpdFindFirst(pparMain->punk->pcpdUnk,szKeyC, szData,HB_LONGSTRINGMAX-2, &iPattern, &fCFL) != 0) {
			// fprintf(stderr, "%s: error in hb_CpdFindFirst(%s), key = %s\n", szPN,pparMain->punk->pcpdUnk->szFileName,szKey);
			return 232;
		}
		strcpy(pparMain->punk->szLF,szData);

		strcpy(szKey,"^^0");
		hb_CpdCvs(szKey,szKeyC);
		if (hb_CpdFindFirst(pparMain->punk->pcpdUnk,szKeyC, szData,HB_LONGSTRINGMAX-2, &iPattern, &fCFL) != 0) {
			// fprintf(stderr, "%s: error in hb_CpdFindFirst(%s), key = %s\n", szPN,pparMain->punk->pcpdUnk->szFileName,szKey);
			return 237;
		}
		pparMain->punk->cMMt = atoi(szData) + 1;
		int il = pparMain->punk->cMMt;
		pparMain->punk->rgszMMt = (char **) hb_LongAlloc(sizeof(char *)*il);
		if (pparMain->punk->rgszMMt == NULL) {
			// fprintf(stderr, "%s: error cannot allocate memory (%ld items) for MMt table.\n", szPN,il);
			return 241;
		}
		int iT;
		for (iT = 0; iT < pparMain->punk->cMMt; iT++) {
			pparMain->punk->rgszMMt[iT] = NULL; /* initialize to NULL only; loaded on demand */
		}

		strcpy(szKey,"^^^0");
		hb_CpdCvs(szKey,szKeyC);
		if (hb_CpdFindFirst(pparMain->punk->pcpdUnk,szKeyC, szData,HB_LONGSTRINGMAX-2, &iPattern, &fCFL) != 0) {
			// fprintf(stderr, "%s: error in hb_CpdFindFirst(%s), key = %s\n", szPN,pparMain->punk->pcpdUnk->szFileName,szKey);
			return 249;
		}
		pparMain->punk->cLRT = atoi(szData) + 1;
		il = pparMain->punk->cLRT;
		pparMain->punk->rgszLRT = (char **) hb_LongAlloc(sizeof(char *)*il);
		if (pparMain->punk->rgszLRT == NULL) {
			// fprintf(stderr, "%s: error cannot allocate memory (%ld items) for LRT table.\n", szPN,il);
			return 248;
		}
		for (iT = 0; iT < pparMain->punk->cLRT; iT++) {
			pparMain->punk->rgszLRT[iT] = NULL; /* initialize to NULL only; loaded on demand */
		}
	}

	fNF = fopen("/dev/null", "w");
	fLog = fopen("/dev/null", "w");

	pparMain->phashTagTable = NULL;
	pparMain->phashInvTagTable = NULL;
	*(pparMain->szTagTable) = HB_EOS;
	if (!tag_table) {
		if (pparMain->fLemmaTagUpdate || pparMain->fForceLemmaUpdate)
			return 339;
		*(pparMain->szTagTable) = HB_EOS;
	} else {
		strcpy(pparMain->szTagTable, tag_table);
		if (hb_HashFileIn(&(pparMain->phashTagTable),pparMain->szTagTable,0) > 0) {
			// fprintf(fLog,"%s: cannot init tag hash table %s\n", szPN,pparMain->szTagTable);
			return 303;
		}
		if (hb_HashFileIn(&(pparMain->phashInvTagTable),pparMain->szTagTable,1) > 0) {
			// fprintf(fLog,"%s: cannot init inverted tag hash table %s\n", szPN,pparMain->szTagTable);
			return 304;
		}
	}

	/* initialize CACHE here: */
	if (*(pparMain->szBaseCache) != HB_EOS) {
		if (hb_HashFileIn(&(pparMain->phashBaseCache),pparMain->szBaseCache,0) > 0) {
			// fprintf(fLog,"%s: cannot init base cache hash table %s\n", szPN,pparMain->szBaseCache);
			return 302;
		}
	} /* base cache used */
	if (pparMain->clCacheEntries <= 0l) {
		pparMain->clCacheEntries = 0l;
	} else {
		pparMain->clCacheQueries = 0l;
		pparMain->clCacheHits = 0l;
		pparMain->clBaseCacheHits = 0l;
		pparMain->rgszCachedResults = (char **) hb_LongAlloc(sizeof(char *) * pparMain->clCacheEntries);
		if (pparMain->rgszCachedResults == NULL) {
			// fprintf(fLog, "%s: cannot allocate cached results ptr array: %ld bytes.\n", szPN,pparMain->clCacheEntries);
			pparMain->clCacheEntries = 0l;
		}
		longint il;
		for (il = 0; il < pparMain->clCacheEntries; il++) 
			pparMain->rgszCachedResults[il] = NULL;
		pparMain->ilCachedResultFree = 0l;
		longint lCE = pparMain->clCacheEntries;
		lCE += (lCE / 4);
		pparMain->phashCache = hb_HashNew(lCE, lCE*48,20);
		if (pparMain->phashCache == NULL) {
			// fprintf(fLog, "%s: cannot allocate hash table for cache: %ld entries/%ld bytes.\n", szPN,lCE,lCE*48);
			pparMain->clCacheEntries = 0l;
		}
	}
	/* end of cache initialization */

	int iHFError = hf_init(szDictionary,pparMain->szCharSet);
	if (iHFError != 0) {
		// fprintf(stderr, "%s: initialization of %s (codepage %s) failed (%d)\n",szPN,szDictionary, pparMain->szCharSet,iHFError);
		return 1000+iHFError;
	}

	hb_SetLangEnv(&(pparMain->langEnvDic),szLangDict);

	/* iconv initialization */
	handle_ui = iconv_open("iso88592//TRANSLIT//IGNORE", "utf8");
	handle_iu = iconv_open("utf8", "iso88592");
	if (handle_iu == (iconv_t)-1 || handle_ui == (iconv_t)-1)
		return 65535;

	free(tag_table);
	free(unknown_rules);

	if (!csts_decode_init())
		return 65534;
	return 0;
}

// not thread safe
void lemmatize_destroy() {
	int i;
	hf_end();

	/* unknown words (guesser) shutdown */
	if (pparMain->punk) {
		hb_CpdClose(pparMain->punk->pcpdUnk);

		for (i = 0; i < pparMain->punk->cLRT; i++)
			hb_Free(pparMain->punk->rgszLRT[i]);
		hb_Free(pparMain->punk->rgszLRT);

		for (i = 0; i < pparMain->punk->cMMt; i++)
			hb_Free(pparMain->punk->rgszMMt[i]);
		hb_Free(pparMain->punk->rgszMMt);

		hb_Free(pparMain->punk);
	}

	/* hb_base free: we cannot use hb_listszFreeList() as it does not
	 * really free items, just put them in a cache */
	while (hb_listszFreeHead) {
		hb_listszRecType *tf = hb_listszFreeHead;
		hb_listszFreeHead = hb_listszFreeHead->plistszNext;
		hb_Free(tf);
	}

	/* generic shutdown */
	for (i = 0; i < pparMain->ilCachedResultFree; i++)
		hb_Free(pparMain->rgszCachedResults[i]);
	hb_Free(pparMain->rgszCachedResults);
	hb_HashFree(&pparMain->phashCache);
	hb_HashFree(&pparMain->phashTagTable);
	hb_HashFree(&pparMain->phashInvTagTable);
	hb_Free(pparMain);

	fclose(fNF);
	fclose(fLog);

	iconv_close(handle_ui);
	iconv_close(handle_iu);

	pthread_mutex_destroy(&mutex);
}

int convert(int direction, const char *src, char *dst, int maxlen) {
	size_t srclen = strlen(src);
	size_t dstlen = maxlen-1;
	size_t res = iconv(direction ? handle_iu : handle_ui, &src, &srclen, &dst, &dstlen);
	if (res == -1 || srclen != 0 || (srclen > 0 && dstlen == 0))
		return 0;
	*dst = '\0';
	return 1;
}

char *csts_encode_table[256] = {
	['%'] = "&percnt;",
	['&'] = "&amp;",
	['#'] = "&num;",
	['*'] = "&ast;",
	['$'] = "&dollar;",
	['<'] = "&lt;",
	['>'] = "&gt;",
	['_'] = "&lowbar;",
	['['] = "&lsqb;",
	[']'] = "&rsqb;",
	['|'] = "&verbar;",
	['\\'] = "&bsol;",
	['^'] = "&circ;",
	['@'] = "&commat;",
	['{'] = "&lcub;",
	['}'] = "&rcub;",
};
//	"\x00E0", "&agrave"	-- special treatement
//	&macron;		-- ignored

int csts_encode_agrave(const char *src, char *dst, int maxlen) {
	int encoded = 0;
	char *last = dst+maxlen-1;
	char *s;
	while ((s = strstr(src, "\xC3\xA0"))) {
		int len = s-src;
		if (dst+len+8 >= last)
			return 0;
		memcpy(dst, src, len);
		memcpy(dst+len, "&agrave;", 8);
		dst += len+8;
		src += len+2;
		encoded = 1;
	}
	if (!encoded)
		return 0;
	int len = strlen(src);
	if (dst+len+1 >= last)
		return 0;
	memcpy(dst, src, len+1);

	return 1;
}

int csts_encode(const char *src, char *dst, int maxlen, int agrave) {
	int encoded = 0;
	char *last = dst+maxlen-1;
	while (*src) {
		char *entity = csts_encode_table[(unsigned char)*src];
		if (entity && !(agrave && !strncmp(src, "&agrave;", strlen("&agrave;")))) {
			int len = strlen(entity);
			if (dst+len >= last)
				return 0;
			memcpy(dst, entity, len);
			dst += len;
			encoded = 1;
		} else {
			*dst++ = *src;
			if (dst >= last)
				return 0;
		}
		src++;
	}
	*dst = '\0';
	return encoded;
}

// filled by csts_decode_init()
int csts_decode_table[256];

inline int hash(const char *s, int len) {
	int c = 0;
	while (len--)
		c += *s++;
	return c % 256;
}

int csts_decode_init() {
	int i;
	char **s = csts_encode_table;
	memset(csts_decode_table, 0, sizeof(csts_decode_table));
	csts_decode_table[hash("agrave", strlen("agrave"))] = 'X';
	for (i = 0; i < 256; i++) {
		if (s[i]) {
			int h = hash(s[i]+1, strlen(s[i]+1)-1);
			if (csts_decode_table[h])
				return 0;
			csts_decode_table[h] = i;
		}
	}
	return 1;
}

int csts_decode(const char *src, char *dst, int maxlen, int agrave) {
	char *last = dst+maxlen-1;
	while (*src) {
		if (*src == '&') {
			const char *s = src+1;
			while (*s && isalnum(*s))
				s++;
			if (*s != ';')
				return 0;
			// src+1 .. s-1 is entity
			int h = hash(src+1, s-src-1);
			if (agrave && csts_decode_table[h] == 'X') {
				if (dst+1 >= last)
					return 0;
				*dst++ = '\xC3';
				*dst++ = '\xA0';
			} else if (csts_decode_table[h]) {
				if (dst >= last)
					return 0;
				*dst++ = csts_decode_table[h];
			} else {
				return 0;
			}
			src = s;
		} else {
			*dst++ = *src;
			if (dst >= last)
				return 0;
		}
		src++;
	}
	*dst = '\0';
	return 1;
}

int lemmatize(parRecType *ppar, int dot, int hyph, int fAbbrIn, int fNum, int fPhDot, int fForm, char *szLemmaIn);

// thread safe
#define MAXLEN 10*1024
char *lemmatize_token(const char *token, int is_punct, int is_abbr, int is_number, int dot_follows, int hyphen_follows) {
	char tmp1[MAXLEN];
	char tmp2[MAXLEN];
	char *buf1 = tmp1;
	char *buf2 = tmp2;

	// replace agrave char by &agrave;
	int agrave = csts_encode_agrave(token, buf1, MAXLEN);
	if (!agrave)
		buf1 = (char*)token;

	// convert utf-8 to iso-8859-2
	if (!convert(0, buf1, buf2, MAXLEN-1))
		return NULL;
	buf1 = tmp2;
	buf2 = tmp1;	

	// replace special chars by entities
	int encoded = csts_encode(buf1, buf2, MAXLEN, agrave);
	if (encoded) {
		char *s = buf1;
		buf1 = buf2;
		buf2 = s;
	}
	
	int contain_hyphen = 0;
	int last_dot = 0;

	if (strchr(token, '-'))
		contain_hyphen = 1;
	if (*token && token[strlen(token)-1] == '.')
		last_dot = 1;

	pthread_mutex_lock(&mutex);
	//int lemmatize(ppar,dot,hyph,fAbbrIn,fNum,fPhDot,fForm,szLemmaIn)
	strncpy(szFormIn, buf1, HB_STRINGMAX-2);
	szFormIn[HB_STRINGMAX-1] = '\0';
	char *lemma = "";
	int rc = lemmatize(pparMain, dot_follows, hyphen_follows, is_abbr, is_number, last_dot, !is_punct, lemma);
	if (rc != 0) {
		pthread_mutex_unlock(&mutex);
		return NULL;
	}

	int first = 1;
	char *p = pszOutput;
	char *r = buf1;
	while (*p && r-buf1 < MAXLEN-1) {
		if (*p == '<') {
			if (!strncmp(p, "<MMl", 4)) {
				if (first)
					first = 0;
				else
					*r++ = '\t';
			} else if (!strncmp(p, "<MMt", 4)) {
				*r++ = ' ';
			}
			while (*p != '>')
				p++;
			p++;
		} else {
			*r++ = *p++;
		}
	}
	*r = '\0';

	pthread_mutex_unlock(&mutex);

	// convert iso-8859-2 to utf-8
	if (!convert(1, buf1, buf2, MAXLEN))
		return NULL;
	char *s = buf1;
	buf1 = buf2;
	buf2 = s;

	if (encoded || agrave) {
		if (!csts_decode(buf1, buf2, MAXLEN, agrave))
			return NULL;
		char *s = buf1;
		buf1 = buf2;
		buf2 = s;
	}


	return strdup(buf1);
}
