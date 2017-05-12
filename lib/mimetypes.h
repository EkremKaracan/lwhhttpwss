/***************************************************************************
 *   Copyright (C) 2009+ by Ekrem Karacan                                  *
 *   Ekrem.Karacan@gmail.com                                               *
 ***************************************************************************/

#ifndef	mimetypes_h
#define	mimetypes_h

typedef	struct {
	char	ext[16];
	int	deflateable;
	char	mime[64];
} e_mime_type;

typedef	struct {
	int		count;
	time_t		last_modified;
	e_mime_type	mimes[1];
} e_mime_types;


e_mime_types	*LoadMimeTypesFromFile(char *fname);
e_mime_type	*FindMime(char *ext);

#endif

