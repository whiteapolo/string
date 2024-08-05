#ifndef STRING_H
#define STRING_H

#include <alloca.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
	char *data;
	int len;
	int alocatedLen;
} string;

typedef string strStack;
typedef string strView;

#define apDup(ap)\
({\
	va_list dup;\
	va_copy(dup, ap);\
	dup;\
})

#define newStackStrVa(fmt, ap)                         \
({                                                     \
	strStack s;                                        \
	s.len = getFmtSizeVa(fmt, apDup(ap));              \
	s.data = alloca((s.len + 1) * sizeof(char));       \
	vsnprintf(s.data, s.len + 1, fmt, apDup(ap));      \
	s;                                                 \
})

#define newStackStr(fmt, ...)                          \
({                                                     \
	strStack s;                                        \
	s.len = getFmtSize(fmt, ##__VA_ARGS__);            \
	s.data = alloca((s.len + 1) * sizeof(char));       \
	snprintf(s.data, s.len + 1, fmt, ##__VA_ARGS__);   \
	s;                                                 \
})


string newStr(const char *fmt, ...);

string newStrVa(const char *fmt, va_list ap);

string newFromExisting(string s);

int getFmtSize(const char *fmt, ...);

int getFmtSizeVa(const char *fmt, va_list ap);

void strPushc(string *s, char c);

char strTopc(string *s);

char strPopc(string *s);

void strPushs(string *s, const char *fmt, ...);

void strPushsExisting(string *s, string src);

void strEmpty(string *s);

bool strIsEmpty(string s);

int strCmp(string s1, string s2);

int strnCmp(string s1, string s2, int n);

bool strIsEqual(string s1, string s2);

bool strnIsEqual(string s1, string s2, int n);

strView strTokStart(string s, const char *delim);

strView strTok(string s, strView previuosView, const char *delim);

void strFree(string s);

void strPrint(string s);

string readWholeFile(const char *fileName);

int getFileSize(FILE *fp);

strView getHomePath();

void replaceTildeWithHome(string *path);

void replaceHomeWithTilde(string *path);

#endif
