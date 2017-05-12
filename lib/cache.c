/***************************************************************************
 *   Copyright (C) 2009+ by Ekrem Karacan                                  *
 *   Ekrem.Karacan@gmail.com                                               *
 ***************************************************************************/
#include	<stdint.h>
#include	<ctype.h>

#include	"cache.h"

typedef	int32_t	int32;

int	strzncmp(char *s, char *d, int n) {
	for ( ; n; n--, s++, d++) {
		if (!*s && !*d)
			return 0;
		if (*s == *d)
			continue;
		if (*s == '?' && !*d)
			return 0;
		if (*s > *d)
			return 1;
		return -1;
	}
	return 0;
}

int	strcasezncmp(char *s, char *d, int n) {
	char	sc, dc;
	for ( ; n; n--, s++, d++) {
		if (!*s && !*d)
			return 0;
		if ((sc = toupper(*s)) == (dc = toupper(*d)))
			continue;
		if (dc == '?') {
			if (!sc)
				return 1;
			continue;
		}
		if (sc > dc)
			return 1;
		return -1;
	}
	return 0;
}



int	el_find(char *p, char *t) {
	int	i;
	if (!t)
		return 0;
	if (!*t)
		return 0;
l0:
	for (i=*(t++); i; i--) {
		int	c= *(t++);

		if (strzncmp(p, t, c) == 0) {
			if (!t[c-1]) {
				t+=c;
				p+=c;
				i = *((int32 *) t);
				t+=4;
				return i;
			}
			t+=c;
			p+=c;
			i = *((int32 *) t);
			t+=4;
			if (i>= 0) {
				t+=i;
				goto l0;
			} else
				return -i;
		} else {
			t+=c;
			t+=4;
		}
	}
	return 0;
}

int	el_casefind(char *p, char *t) {
	int	i;
	if (!t)
		return 0;
	if (!*t)
		return 0;
l0:
	for (i=*(t++); i; i--) {
		int	c= *((int32 *) (t++));

		if (strcasezncmp(p, t, c) == 0) {
			if (!t[c-1]) {
				t+=c;
				p+=c;
				i = *((int32 *) t);
				t+=4;
				return i;
			}
			t+=c;
			p+=c;
			i = *((int32 *) t);
			t+=4;
			if (i>= 0) {
				t+=i;
				goto l0;
			} else
				return -i;
		} else {
			t+=c;
			t+=4;
		}
	}
	return 0;
}
