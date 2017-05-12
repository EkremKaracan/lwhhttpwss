/***************************************************************************
 *   Copyright (C) 2009+ by Ekrem Karacan                                  *
 *   Ekrem.Karacan@gmail.com                                               *
 ***************************************************************************/

#define	foreachfile_c

#include	<stdio.h>
#include	<errno.h>
#include	<dirent.h>
#include	<sys/stat.h>
#include	<sys/time.h>
#include	<string.h>
#include	<sys/types.h>
#include	<fcntl.h>
//#include	<time.h>
#include	"utils.h"
#include	"foreachfile.h"

int	ForEachFile(ForEachFileStats *fefst, char *src, char *dst, ForEachFileCB cb, int level) {
	DIR		*dir;
	struct	dirent	*f;
	struct	stat	st;
//	time_t		curtime;
	char		fname[1024],
			dname[1024];
//	DWORD		fresh = 0, cnt = 0;

	if (!level && fefst) {
		void	*p = fefst->ExtraData;
		bzero(fefst, sizeof(*fefst));
		fefst->ExtraData = p;
		gettimeofday(&fefst->start_time, NULL);
		fefst->end_time = fefst->start_time;
	}
	if (!(dir = opendir(src)))
		return errno;
	for (; (f = readdir(dir)) && !Sys.DoStop; ) {
		sprintf(fname, "%s/%s", src, f->d_name);
		if (dst)
			sprintf(dname, "%s/%s", dst, f->d_name);

		if (stat(fname, &st)) {
			if (fefst)
				fefst->Err++;
			continue;
		}
		if (S_ISDIR(st.st_mode)) {
			continue;
		}
		if (st.st_size == 0) {
			continue;
		}
		if (fefst) {
			fefst->FilesMatching++;
			gettimeofday(&fefst->end_time, NULL);
		}
		if ((*cb)(fefst, f->d_name, fname, dst?dname:"", st.st_size, &st))
			if (fefst) {
				fefst->FilesProcessed++;
				fefst->TotalBytes += st.st_size;
			}
	}
	closedir(dir);
	if (!(dir = opendir(src)))
		return errno;
	for (; (f = readdir(dir) ) && !Sys.DoStop; ) {
		sprintf(fname, "%s/%s", src, f->d_name);
		if (stat(fname, &st)) {
			continue;
		}
		if (!S_ISDIR(st.st_mode)) {
			continue;
		}
		if (!strcmp(f->d_name, "."))
			continue;
		if (!strcmp(f->d_name, ".."))
			continue;
		if (dst)
			sprintf(dname, "%s/%s", dst, f->d_name);
		if (fefst)
			fefst->DirectoriesProcessed++;
		ForEachFile(fefst, fname, dst?dname:NULL, cb, level+1);
	}
	closedir(dir);

	if (fefst) {

		gettimeofday(&fefst->end_time, NULL);
		fefst->total_time = (fefst->end_time.tv_sec - fefst->start_time.tv_sec) * 1000;
		fefst->total_time +=  fefst->end_time.tv_usec / 1000;
		fefst->total_time -=  fefst->start_time.tv_usec / 1000;
	}
	return 0;
}

