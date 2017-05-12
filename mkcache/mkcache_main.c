/***************************************************************************
 *   Copyright (C) 2009+ by Ekrem Karacan                                  *
 *   Ekrem.Karacan@gmail.com                                               *
 ***************************************************************************/

#include	<stdio.h>
#include	<time.h>
#include	<sys/stat.h>
#include	<malloc.h>
#include	"zlib.h"
#include	<fcntl.h>
#include	<stdlib.h>

#include	"foreachfile.h"
#include	"mimetypes.h"
#include	"utils.h"

char	docsrc[512] = "webroot";
char	docroot[512] = "webroot_cache";
char	modules[512] = "modules.conf";
//int	gIsDaemon = 0;

cmdline_param Params[]={

	{"webroot=", "%s", docsrc},
	{"websrc=", "%s", docroot},
	{"modules=", "%s", modules},
	{"", "", NULL, ""}
};

typedef	struct el_dnode_st {
	char	c;
	int	depth, count;
	int	value;
 	struct	el_dnode_st	*n, *p, *f, *o;
} el_dnode;

int	prepare_table(el_dnode *n, char *table);

el_dnode	*init_el(void) {
	el_dnode	*p = calloc(sizeof(el_dnode), 1);
	//sprintf("init_el: %p <%d>\n", p, sizeof(el_dnode));
	return p;
}
/*

int	el_find(char *p, char *t) {
	int	i;
	printf("El find\n");
	if (!t)
		return 0;
	if (!*t)
		return 0;
l0:
	for (i=*(t++); i; i--) {
		BYTE	c= *(t++);
		printf("[%d:%d|", i, (int) c);
		fwrite(t, c, 1, stdout);
		printf(":%d]\n", c);
		//return 0;
/ *		if (*p == 0 && *t == 0 && t == 1) {
			t++;
			i = *((int *) t);
			return -1;
		}
* /
		if (strzncmp(p, t, c) == 0) {
			if (!t[c-1]) {
				t+=c;
				p+=c;
				i = *((int32 *) t);
				t+=4;
				printf("<%d>", i);
				return i;
			}
			t+=c;
			p+=c;
			i = *((int32 *) t);
			t+=4;
			printf("<%d>", i);
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
*/

el_dnode	*add_el(el_dnode *n, char c, int value) {
	el_dnode *p, *l = NULL;

	for (p = n->f; p; p = p->n) {
		if (p->c == c)
			return p;
		if (p->c > c)
			break;
		l = p;
	}
	p = calloc(sizeof(el_dnode), 1);
	p->c = c;
	p->o = n;
	p->depth = n->depth+1;
	p->value = value;
	//printf("Add el %p->%p(d:%d,c:%c,v:%d)\n", n, p, p->depth, p->c, p->value);

	if ((p->p = l)) {
		if ((p->n = l->n))
			p->n->p = p;
		l->n = p;
	} else {
		if ((p->n = n->f))
			p->n->p = p;
		n->f = p;
	}
	n->count++;
	return p;
}
void	addstr_el(el_dnode *n, char *c, int value) {
	for ( ; *c; c++)
		n = add_el(n, *c, 0);
	n = add_el(n, 0, value);
//	if (n)
//		n->value = value;
}

int	el_count_leafs(el_dnode *n) {
	int	r = 0;
	if (!n->f)
		return 1;
	for (n=n->f; n; n=n->n)
		r += el_count_leafs(n);
	return r;
}

void	print_el(el_dnode *n, int d) {
	int	a, c, j;
	//printf("Print El %d\n", d);
	if (!n) {
		return;
	}
	//printf("El depth = %d\n", n->depth);
	if (d == n->depth) {
		a = el_count_leafs(n)*3;
		j = (a % 2) ^ 1;
		a = a/2;
		if (!(c=n->c))
			c='*';
		printf("%-*c%c%*c", a, '[', c, a-j, ']');
		return;
	} else if (!n->f) {
		a = el_count_leafs(n)*3;
		j = (a % 2) ^ 1;
		a = a/2;
			c=' ';
		printf("%-*c%c%*c", a, '[', c, a-j, ']');

	}
	for (n=n->f; n; n=n->n)
		print_el(n, d);

}

int	idx = -1;

int	prepare_table2(el_dnode *n, char *table) {
	el_dnode	*c;
	int32		i, m, l = 0;
	char		*tp, *p;
	int32		cnt = 0, rcnt;
	el_dnode	*node = n, *nd;

	tp = table;
	*table = 0;
	if (!n)
		return 0;
	if (!n->f)
		return 0;
	*(table++) = n->count;
	printf("n->count = %d\n", n->count);
	cnt++;
	n = n->f;
	for (; n; n = n->n) {
		p = table++;
		cnt++;
//		if (n->c)
			*(table++) = n->c;
//		else
//			*(table++) = '*';
		cnt++;
		l = 1;
		if (n->count == 1) {
			for (c=n->f; c; c = c->f) {
//				if (c->c)
					*(table++) = c->c;
//				else
//					*(table++) = '*';
				cnt++;
				l++;
				if (c->count != 1)
					break;
			}
		}
		*p = l;

		*((uint32_t *) table) = 0;
		table += 4;
		cnt += 4;
/*		if (n->count == 0) {
			*((uint32_t *) table) = 0;
			table += 4;
			cnt += 4;
		}
*/	}
	rcnt = cnt;
	printf("\nCNT: <%d>\n",cnt);
	n = node = node->f;

	for (table = tp, l=0; l<rcnt; ) {
		int32	n, j;
		m = *(table++);
		l++;
		printf("{(N:%d) ", m);
		for (i=0; (i<m) && node; i++, node = node->n) {
			n = *(table++);
			l++;
			printf("[%d:%d:\"", i, n);
			for (j=0; j<n; j++, l++) //, table++);
				putchar(*(table++));
			l +=4;
			p = table;
			*((int32_t *) table) = cnt-l;
			table += 4;
			if (node->count == 1) {
				nd = node->f;
				while (nd->count == 1)
					nd = nd->f;
			} else
				nd = node;

			if (!nd || nd->count==0) {
				*((int32_t *) p) = nd->value; //-(idx++);
				printf("\":%d]=%d", cnt-l, *((int32_t *) p));
				continue;
			}
			printf("\":%d] (%d) {", cnt-l, node->count);
			if (nd)
				cnt += prepare_table(nd, tp+cnt);
			else
				*((int32_t *) p) = -(idx++);
			printf("}");
		}
	}
	return cnt;
}

int	prepare_table(el_dnode *n, char *table) {
	el_dnode	*c;
	int32		i, m, l = 0;
	char		*tp, *p;
	int32		cnt = 0, rcnt;
	el_dnode	*node = n, *nd;

	tp = table;
	*table = 0;
	if (!n)
		return 0;
	if (!n->f)
		return 0;
	*(table++) = (BYTE) n->count;
	printf("n->count = %d\n", n->count);
	cnt++;
	n = n->f;
	for (; n; n = n->n) {
		p = table++;
		cnt++;
		printf("C(%c)\n", n->c);
//		if (n->c)
			*(table++) = n->c;
//		else
//			*(table++) = '*';
		cnt++;
		l = 1;
		if (n->count == 1) {
			for (c=n->f; c; c = c->f) {
//				if (c->c)
					*(table++) = c->c;
//				else
//					*(table++) = '*';
				cnt++;
				l++;
				if (c->count != 1)
					break;
			}
		}
		*p = l;

		*((uint32_t *) table) = 0;
		table += 4;
		cnt += 4;
/*		if (n->count == 0) {
			*((uint32_t *) table) = 0;
			table += 4;
			cnt += 4;
		}
*/	}
	rcnt = cnt;
	printf("\nCNT: <%d>\n",cnt);
	n = node = node->f;

	for (table = tp, l=0; l<rcnt; ) {
		int32	n, j;
		m = *(table++);
		l++;
		printf("{(N:%d) ", m);
		for (i=0; (i<m) && node; i++, node = node->n) {
			n = *(table++);
			l++;
			printf("[%d:%d:\"", i, n);
			for (j=0; j<n; j++, l++) //, table++);
				putchar(*(table++));
			l +=4;
			p = table;
			*((int32_t *) table) = cnt-l;
			table += 4;
			if (node->count == 1) {
				nd = node->f;
				while (nd->count == 1)
					nd = nd->f;
			} else
				nd = node;

			if (!nd || nd->count==0) {
				*((int32_t *) p) = nd->value; //-(idx++);
				printf("\":%d]=%d", cnt-l, *((int32_t *) p));
				continue;
			}
			printf("\":%d] (%d) {", cnt-l, node->count);
			if (nd)
				cnt += prepare_table(nd, tp+cnt);
			else
				*((int32_t *) p) = -(idx++);
			printf("}\r\n");
		}
	}
	return cnt;
}

el_dnode *root = NULL;

/*
int	strzncmp(char *s, char *d, int n) {
	for ( ; n; n--, s++, d++) {
		if (!*s && !*d)
			return 0;
		if (*s == *d)
			continue;

		if (*s > *d)
			return 1;
		return -1;
	}
	return 0;
}
*/

int	cdata_fd = -1;
int	cidx_fd = -1;
off_t	cdata_ofs = 0;


int	ElCB(ForEachFileStats *fefst, char *fname, char *path, char *uri, int size, struct stat *st) {
	char		ext[16];
	e_mime_type	*m;
	int		fh, fz;
	off_t		ofs = cdata_ofs, zofs = 0;
	z_stream	zs;
	int		zlen;
	char		hdr[2048*32], zdata[2048*32];
	int		l;
	int32		tot, dw;
	int	fd, hlen, hlenz;

	if (!(m = FindMime(GetFileExt(fname, ext)))) {
		LogPrint("Mime type not found [{1|1|0}%s{0||}]\n", fname);
		return 0;
	}
	LogPrint("File [{1|2|0}%s{0||}]\n", fname);
	tot = 0;
	zlen = 0;
	ofs = cdata_ofs;
	fd = cdata_fd;

//LogPrint("[%s] offset %d\n", fname, cdata_ofs);
	lseek(cdata_fd, cdata_ofs, SEEK_SET);
	{
//		time_t	t;
//		struct tm tmt;
		char	datestr[128];
		timetohttptime(&st->st_mtime, datestr);
		//LogPrint("\t{1|3|0}%s{0||}\n", datestr);

		write(cdata_fd, &st->st_mtime, 8);
	}
	write(cdata_fd, &tot, sizeof(tot));
	write(cdata_fd, &tot, sizeof(tot));
	write(cdata_fd, &tot, sizeof(tot));
	write(cdata_fd, &tot, sizeof(tot));

	if ((fh = open(path, O_RDONLY)) < 0)
		return 0;
 //l = sprintf(hdr, "Content-type: %s; charset=iso-8859-1\r\n\r\n", m->mime);
	l = sprintf(hdr, "Content-type: %s; charset=utf-8\r\nAccept-Ranges: bytes\r\n\r\n", m->mime);
	//l = sprintf(hdr, "Content-type: %s\r\nAccept-Ranges: bytes\r\n\r\n", m->mime);
	hlen = tot = write(cdata_fd, hdr, l);
	//LogPrint("\t{1|3|0}hdr length: {1|4|0}%d{0||}\n", tot);
	//LogPrint("\t{1|3|0}content length: {1|4|0}%d {1|2|0}%s{0||}\n", st->st_size, m->mime);


	if (m->deflateable == 1) {
		zs.zalloc = (alloc_func)0;
		zs.zfree = (free_func)0;
		zs.opaque = (voidpf)0;

		deflateInit2(
			&zs, Z_BEST_COMPRESSION,
	 		Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY
		);
		zlen = 0;
		fz = open("/tmp/tmp_cachef.tmpz", O_RDWR | O_CREAT | O_TRUNC, 0777);
	}
	while ((l = read(fh, hdr, sizeof(hdr)))>0) {
		tot += write(cdata_fd, hdr, l);
		//LogPrint("[%s] %d written\n", fname, tot);
		//LogPrint("[%s] read %d  tot: %d\n", fname, l, tot);
		if (m->deflateable != 1)
			continue;
		zs.next_in = (BYTE *) hdr;
		zs.avail_in = l;
		zs.next_out = (BYTE *) zdata;
		zs.avail_out = sizeof(zdata);

		deflate(&zs, Z_NO_FLUSH);
		zlen += sizeof(zdata)-zs.avail_out;
		write(fz, zdata, sizeof(zdata)-zs.avail_out);
	}
//	LogPrint("[%s] offset %d, length %d\n", fname, cdata_ofs, tot);
	zofs = lseek(cdata_fd, 0, SEEK_CUR);
	if (m->deflateable == 1) {
		for ( ; ; ) {
			zs.next_out = (BYTE *) zdata;
			zs.avail_out = sizeof(zdata);
			zs.next_in = (BYTE *) hdr;
			zs.avail_in = 0;
			deflate(&zs, Z_FINISH);
			if (sizeof(zdata) == zs.avail_out)
				break;
			zlen += sizeof(zdata)-zs.avail_out;
			write(fz, zdata, sizeof(zdata)-zs.avail_out);
		}
		close(fz);

		zs.next_out = NULL;
		zs.avail_out = 0;
		zs.next_in = NULL;
		zs.avail_in = 0;
		deflateEnd(&zs);
		if (zlen < (tot * 0.7)) {
			hlenz = l = sprintf(hdr, "Content-encoding: deflate\r\nContent-type: %s; charset=utf-8\r\n\r\n", m->mime);
			fz = open("/tmp/tmp_cachef.tmpz", O_RDONLY);
			zlen = write(cdata_fd, hdr, l);
			while ((l=read(fz, hdr, 2048))>0) {
				zlen += write(cdata_fd, hdr, l);
			}
			close(fz);
		} else {
			LogPrint("\t{1|1|0}Not worth compressing {0|1|0}%12d{1|1|0} to {0|1|0}%12d{0||}\n", tot, zlen);
			zlen=0;
		}
	}
	LogPrint("\t{1|3|0}length{1|4|0} %12d{1|3|0}, compressed length {1|4|0}%12d\n{0||}", tot, zlen);
	ofs = lseek(cdata_fd, 0, SEEK_CUR);
	lseek(cdata_fd, cdata_ofs+8, SEEK_SET);
	dw = tot;
	write(cdata_fd, &dw, sizeof(dw));
	dw = zlen;
	write(cdata_fd, &dw, sizeof(dw));
	dw = hlen;
	write(cdata_fd, &dw, sizeof(dw));
	dw = hlenz;
	write(cdata_fd, &dw, sizeof(dw));

	addstr_el(root, uri, cdata_ofs);
	LogPrint("Stored: %s, %d, %d\n", uri, tot, zlen);
	cdata_ofs = lseek(cdata_fd, ofs, SEEK_SET);
	close(fh);
	LogPrint("%4d %4d %10d %12d %-46s\n", fefst->FilesProcessed,  fefst->FilesMatching, fefst->TotalBytes, st->st_size, fname);
//	HashInsertFile(path, uri, m->mime, m->deflateable, st);
	return 1;
}

void	LoadModules(char *root, char *modules);

int	run_mkcache(int argc, char **argv, int isfirstrun) {
	ForEachFileStats	fef;
	char	*str;
	char	*stres;
	char	/*key[128],*/ keypath[256], idxpath[256],
			keypathd[256], idxpathd[256];
//	int		mkcache = 0;
	int		i = 0;

	str = calloc(1024, 400);
//	el_dnode *n;
	int	n;

	LogPrint("Basladi\n");
	root = init_el();

	LogPrint("Loading mime\n");
	LoadMimeTypesFromFile("mime.types");
	LogPrint("Loaded mime\n");


	sprintf(keypath, "%s/web.cache.tmp.XXXXXX", docsrc);
	LogPrint("keypath = %s\n", keypath);
	if ((cdata_fd = mkstemp(keypath)) <= 0)
		goto l0;
	LogPrint("writing data\n");
	write(cdata_fd, &cdata_fd, 4);
	cdata_ofs = 4;
	ForEachFile(&fef, docroot, "", ElCB, 0);
//	LoadModules((char *) root, (char *) modules);
//	fflush(stdout);
	i = prepare_table(root, str);
	LogPrint("prepare_table = %d\n", i);
//	fflush(stdout);
/*	stres = (char *) el_find("/index.html", str);

	fflush(stdout);
	LogPrint("Search test: %p\n", i, stres);
	
	 */
	sprintf(idxpath, "%s/web.index.tmp.XXXXXX", docsrc);
	n = mkstemp(idxpath);
	write(n, str, i);
	close(n);
	close(cdata_fd);

	sprintf(keypathd, "%s/web.data", docsrc);
	sprintf(idxpathd, "%s/web.index", docsrc);
	LogPrint("Idx(%s -> %s), Data(%s -> %s)\n", idxpath, idxpathd, keypath, keypathd);
	if (rename(keypath, keypathd)) {
		perror(keypathd);
	}
	if (rename(idxpath, idxpathd)) {
		perror(keypathd);
	}
l0:
	return 0;
}

void	cleanup_mkcache(int sig) {
//	int	i, j, k;
	LogPrint("{1|2|0}Bitti{1|1|0}[%d]{0||}\n", sig);
/*	for (i=0; i<8; i++)
	LogPrint("Deneme{0|%d|7}[%d]{0||}\n", i, i);
	for (i=0; i<8; i++)
	LogPrint("Deneme{0|7|%d}[%d]{0||}\n", i, i);
	for (i=0; i<8; i++)
	LogPrint("Deneme{%d|7|0}[%d]{0||}\n", i, i);
	LogPrint("{1|1|0}BITTI{0||}\n");
		LogPrint("{1|2|0}BITTI{0||}\n");
*/
}

int	main(int argc, char **argv) {
	strcpy(Sys.AppName, "mkcache");
	return	LinMain(
		argc, argv,
		run_mkcache,
		cleanup_mkcache,
		(timerhandler_t) NULL,
		Params
	);
}

void	LoadModules(char *root, char *modules) {
	FILE	*fp = fopen(modules, "r");
	char	line[1024];
	int	id;
	char	uri[1024];
	if (!fp)
		return;
	while (fgets(line, 1023, fp)) {
		if (line[0] == '#')
			continue;
		if (sscanf(line, "%s %d", uri, &id) != 2)
			continue;
		addstr_el((el_dnode *) root, uri, id);
		LogPrint("module added %s %d\n", uri, id);
	}
	fclose(fp);
}
