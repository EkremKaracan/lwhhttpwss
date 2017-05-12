/*   Copyright (C) 2009+ by Ekrem Karacan                                  *
 *   Ekrem.Karacan@gmail.com                                               *
 ***************************************************************************/

#include	<stdio.h>
#include	<time.h>
#include	<sys/stat.h>
#include	<stdlib.h>
#include	<string.h>

#include	"utils.h"
#include	"mimetypes.h"


static	e_mime_types	*gMimes = NULL;

char	*GetToken(char *p, char *d) {
	while (*p == ' ' || *p == 13 || *p == 10 || *p == 9)
		p++;
	for ( ; *p; p++, d++) {
		if (*p == ' ' || *p == 13 || *p == 10 || *p == 9)
			break;
		*d = *p;
	}
	*d = 0;
	return p;
}

e_mime_types	*CheckMimeTypesFromFile(char *fname, e_mime_types *last_one) {
	struct	stat	st;
	FILE		*fp;
	char		linebuf[1024];
	int		i, j;
	int		deflateable;
	char		temp[1024];
	char		ext[16];
	char		mime[64];
	e_mime_types	*res = NULL;
	e_mime_type	*m;
	char	*p;

	if (stat(fname, &st)) {
		return NULL;
	}
	if (last_one) {
		if (last_one->last_modified == st.st_mtime)
			return NULL;
	}
	fp = fopen(fname, "rb");
	if (!fp)
		goto l0;

	for (i=0, j=1; fgets(linebuf, 1024-1, fp); j++) {
		if (linebuf[0] != '0' && linebuf[0] != '1' && linebuf[0] != '2')
			continue;
		p = linebuf;
		p = GetToken(p, temp);
		if (sscanf(temp, "%d", &deflateable) != 1)
			continue;
		if (!*(p = GetToken(p, mime)))
			continue;
		if (!*(p = GetToken(p, ext)))
			continue;
		while (*p) {
//			printf("%d : <%s><%s><%s>\n", i+1, temp, mime, ext);
			p = GetToken(p, ext);
			i++;
		}
	}
//	getchar();
	fseek(fp, 0, SEEK_SET);
//	printf("Total lines: %d\n", i);
//	getchar();
	res = calloc(sizeof(e_mime_types) + sizeof(e_mime_type) * (i - 1), 1);
	if (!res)
		goto l1;
	res->last_modified = st.st_mtime;
	res->count = i;
	for (i=0, j=1, m = res->mimes; fgets(linebuf, 1024-1, fp) && i < res->count; j++) {
		if (linebuf[0] != '0' && linebuf[0] != '1' && linebuf[0] != '2')
			continue;
		p = linebuf;
		p = GetToken(p, temp);
		if (sscanf(temp, "%d", &deflateable) != 1)
			continue;
		if (!*(p = GetToken(p, mime)))
			continue;
		if (!*(p = GetToken(p, ext)))
			continue;
		while (*p) {
		//	LogPrint("%d : <%s><%s><%s>\n", i+1, temp, mime, ext);
			strcpy(m->ext, ext);
			m->deflateable = deflateable;
			strcpy(m->mime, mime);
			i++;
			m++;
			p = GetToken(p, ext);
		}
	}
	qsort((void *) res->mimes, res->count, sizeof(e_mime_type), (__compar_fn_t) strcmp);
	for (i=0, m = res->mimes; i < res->count; i++, m++)
		LogPrint("%d: %d, \"%s\", \"%s\"\n", i+1, m->deflateable, m->ext, m->mime);
l1:
	fclose(fp);
l0:
	return res;
}

e_mime_types	*LoadMimeTypesFromFile(char *fname) {
	e_mime_types	*newmimes;
	if ((newmimes = CheckMimeTypesFromFile(fname, gMimes))) {
		if (gMimes)
			free(gMimes);
		gMimes = newmimes;
		LogPrint("Mimes changed\n");
	} else
		LogPrint("Mimes not changed\n");
	return gMimes;
}

e_mime_type	*FindMime(char *ext) {
	if (!gMimes)
		return NULL;
	return bsearch(ext, gMimes->mimes, gMimes->count, sizeof(e_mime_type), (__compar_fn_t) strcasecmp);
}

