#include "string.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const float GROW_RATE      =  1.5f;
const float SHRINK_RATE    =  0.66f;

const float GROW_TRIGGER   =  0.66f;
const float SHRINK_TRIGGER =  0.33f;

string newStr(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	string s = newStrVa(fmt, apDup(ap));
	va_end(ap);
	return s;
}

int getFmtSize(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int size = vsnprintf(NULL, 0, fmt, apDup(ap));
	va_end(ap);
	return size;
}

int getFmtSizeVa(const char *fmt, va_list ap)
{
	return vsnprintf(NULL, 0, fmt, ap);
}

string newStrVa(const char *fmt, va_list ap)
{
	string s;
	s.len = getFmtSizeVa(fmt, apDup(ap));
	s.alocatedLen = s.len * 2;
	s.data = malloc(s.alocatedLen * sizeof(char));
	vsnprintf(s.data, s.len + 1, fmt, apDup(ap));
    return s;
}

string newFromExisting(string s)
{
	return newStr("%.*s", s.len, s.data);
}

void growByOne(string *s)
{
	s->len++;
	if (s->alocatedLen == 0) {
		s->alocatedLen = s->len * 2;
		s->data = realloc(s->data, s->alocatedLen);
	} else if ((float) s->len / s->alocatedLen > GROW_TRIGGER)	{
		s->alocatedLen *= GROW_RATE;
		s->data = realloc(s->data, s->alocatedLen);
	}
}

void shrinkByOne(string *s)
{
	s->len--;
	if ((float) s->len / s->alocatedLen < SHRINK_TRIGGER) {
		s->alocatedLen /= GROW_RATE;
		s->data = realloc(s->data, s->alocatedLen);
	}
}

void strPushc(string *s, char c)
{
	growByOne(s);
	s->data[s->len - 1] = c;
}

char strTopc(string *s)
{
	return s->data[s->len - 1];
}

char strPopc(string *s)
{
	char c = strTopc(s);
	shrinkByOne(s);
	return c;
}

void strPushs(string *s, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	strStack tmp = newStackStrVa(fmt, ap);
	va_end(ap);

	for (int i = 0; i < tmp.len; i++)
		strPushc(s, tmp.data[i]);
}

void strPushsExisting(string *s, string src)
{
	strPushs(s, "%.*s", src.len, src.data);
}

void strEmpty(string *s)
{
	while (s->len > 0)
		strPopc(s);
}

bool strIsEmpty(string s)
{
	return s.len == 0;
}

int strCmp(string s1, string s2)
{
	return strncmp(s1.data, s2.data, s1.len);
}

int strnCmp(string s1, string s2, int n)
{
	if (s1.len < n)
		return -1; 

	if (s2.len < n)
		return 1; 

	return strncmp(s1.data, s2.data, n);
}

bool strIsEqual(string s1, string s2)
{
	if (s1.len != s2.len)
		return false;
	return strCmp(s1, s2) == 0;
}

bool strnIsEqual(string s1, string s2, int n)
{
	return strnCmp(s1, s2, n) == 0;
}

bool isStrViewAtEnd(string s, strView view)
{
	return view.data + view.len > s.data + s.len;
}

strView strTokStart(string s, const char *delim)
{
	strView view;
	view.data = s.data;
	view.len = 0;
	return strTok(s, view, delim);
}

strView strTok(string s, strView previuosView, const char *delim)
{
	strView view;
	view.data = previuosView.data + previuosView.len;
	view.len = 0;

	while (!isStrViewAtEnd(s, view) && 
			strchr(delim, view.data[view.len]))
	{
		view.data++;
	}

	while (!isStrViewAtEnd(s, view) && 
			!strchr(delim, view.data[view.len]))
	{
		view.len++;
	}

	return view;
}

void strFree(string s)
{
	free(s.data);
}

void strPrint(string s)
{
    printf("%.*s", s.len, s.data);
}

int getFileSize(FILE *fp)
{
	int curr = ftell(fp);
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, curr, SEEK_SET);
	return size;
}

strView getHomePath()
{
	strView s;
	s.data = getenv("HOME");
	if (s.data == NULL)
		s.data = ".";
	s.len = strlen(s.data);
	return s;
}

void replaceTildeWithHome(string *path)
{
	if (path->data[0] == '~') {
		strView home = getHomePath();
		strStack tmp = newStackStr("%.*s%.*s", 
				home.len, home.data, path->len - 1, path->data + 1);
		strEmpty(path);
		strPushsExisting(path, tmp);
	}
}

void replaceHomeWithTilde(string *path)
{
	strView home = getHomePath();
	if (strnIsEqual(*path, home, home.len)) {
		strStack tmp = newStackStr("~%.*s", 
				path->len - home.len, path->data + home.len);
		strEmpty(path);
		strPushsExisting(path, tmp);
	}
}

string readWholeFile(const char *fileName)
{
	string expandedPath = newStr(fileName);
	replaceTildeWithHome(&expandedPath);
	strPushc(&expandedPath, '\0');

	FILE *fp = fopen(expandedPath.data, "r");
	strFree(expandedPath);

	string s = newStr("");

	if (fp == NULL)
		return s;

	int fileSize = getFileSize(fp);
	char c;

	while ((c = fgetc(fp)) != EOF)
		strPushc(&s, c);

	fclose(fp);
	return s;
}

strView getPathTail(const char *path)
{
	// strView s;
	// int pathLen = strlen(path);
	// int i = pathLen - 1;

	// if (path[i] == '/') {
	// 	s.len = 0;
	// 	return s;
	// }

	// while (i > 0 && path[i - 1] != '/')
	// 	i--;

	// return path + i;
}
