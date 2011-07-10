#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
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

	free(tag_table);
	free(unknown_rules);

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

	pthread_mutex_destroy(&mutex);
}

int lemmatize(parRecType *ppar, int dot, int hyph, int fAbbrIn, int fNum, int fPhDot, int fForm, char *szLemmaIn);

// thread safe
char *lemmatize_token(const char *token, int is_punct, int is_abbr, int is_number, int dot_follows, int hyphen_follows) {
	int contain_hyphen = 0;
	int last_dot = 0;

	if (strchr(token, '-'))
		contain_hyphen = 1;
	if (*token && token[strlen(token)-1] == '.')
		last_dot = 1;

	pthread_mutex_lock(&mutex);
	//int lemmatize(ppar,dot,hyph,fAbbrIn,fNum,fPhDot,fForm,szLemmaIn)
	strncpy(szFormIn, token, HB_STRINGMAX-2);
	char *lemma = "";
	int rc = lemmatize(pparMain, dot_follows, hyphen_follows, is_abbr, is_number, last_dot, !is_punct, lemma);
	char *result = NULL;
	if (rc == 0) {
		int first = 1;
		result = strdup(pszOutput);
		char *r = result;
		char *p = result;
		while (*p) {
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
	}
	pthread_mutex_unlock(&mutex);
	return result;
}
