/***************************************************************************
 *   Copyright (C) 2009+ by Ekrem Karacan                                  *
 *   Ekrem.Karacan@gmail.com                                               *
 ***************************************************************************/
#ifndef	foreachfile_h
#define	foreachfile_h

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include	"utils.h"

typedef	struct {
	int	DirectoriesProcessed;
	int	Err;
	int	FilesProcessed;
	int	FilesMatching;
	int	TotalBytes;
	struct	timeval start_time;
	struct	timeval end_time;
	DWORD	total_time;
	char	*ExtraData;
} ForEachFileStats;

typedef	int	(*ForEachFileCB)(
			ForEachFileStats *fefst,
			char *fname,
			char *path,
			char *uri,
			int size,
			struct stat *st
		);
int	ForEachFile(ForEachFileStats *fefst, char *src, char *dst, ForEachFileCB cb, int level);

#endif

