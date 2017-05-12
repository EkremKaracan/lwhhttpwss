/*
	Copyright (C) 2015+ Ekrem Karacan
	ekrem.karacan@gmail.com
*/
//#define	POLL_DEBUG
#define		utils_c

#include	<stdio.h>
#include	<dirent.h>
#include	<string.h>
#include	<errno.h>
#include	<stdarg.h>
#include	<stdlib.h>
#include	<strings.h>
#include	<malloc.h>
#include	<sys/stat.h>
#include	<sys/uio.h>
#include	<arpa/inet.h>
#include	<sys/select.h>
#include	<sys/ioctl.h>
#include	<sys/types.h>
#include	<sys/wait.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<sys/time.h>
#include	<sys/epoll.h>
#include	<signal.h>
#include	<wait.h>
#include	<net/if.h>
#include	<netinet/in.h>
#include	<syslog.h>
#include	<sys/types.h>
#include	<sys/ptrace.h>
#include	<sys/signalfd.h>
#include 	<pthread.h>
#include	"utils.h"
#include	"version.h"

sigset_t    __sigmask;
void	SigHandler(int sig);

extern int gIsDaemon;

static char  *__week[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static char  *__months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };


char	*days[7] = {
	"Pazar",
	"Pazartesi",
	"Salı",
	"Çarşamba",
	"Perşembe",
	"Cuma",
	"Cumartesi"
};
char	*months[12]={
	"Ocak",
	"Şubat",
	"Mart",
	"Nisan",
	"Mayıs",
	"Haziran", 
	"Temmuz",
	"Ağustos",
	"Eylül",
	"Ekim",
	"Kasım",
	"Aralık" 
};

void	DeletePid(int pid) {
	HandleEntry	*he, *n;
	for (he = (HandleEntry *) Sys.Handles->lh.f; he; he = n) {
		n = (HandleEntry *) he->le.n;
		if (he->handletype != PROCESS_HANDLE)
			continue;
		if (he->pid != pid)
			continue;
		LogPrint("Closing child handle\n");	
		//close(he->fhin);
		CloseHandle(Sys.Handles, he);
		LogPrint("Child handle closed\n");
	}
}
char    *timetohttptime(time_t *tt, char *d) {
	struct  tm      t;

	gmtime_r(tt, &t);
	sprintf(d, "%s, %02d %s %4d %02d:%02d:%02d GMT",
			__week[t.tm_wday],
			t.tm_mday,
			__months[t.tm_mon],
			t.tm_year+1900,
			t.tm_hour,
			t.tm_min,
			t.tm_sec
	);
	return d;
}

char	*GetFileExt(char *p, char *ext) {
	char	*d = NULL;
	*ext = 0;
	for ( ; *p; p++) {
		if (*p == '.') {
			d = p;
		}
	}
	if (d)
		strncpy(ext, d+1, 10);
	return ext;
}

char	*GetProgramName(char *p, char *ProgramName) {
	char	*pp;
	for (pp = p; *p; p++) {
		if (*p == '/')
			pp = p+1;
	}
	return strcpy(ProgramName, pp);
}

char	*GetProgramPath(char *p, char *ProgramPath) {
	char	*pp;
	int		i=0, j = 0;
	
	for (pp = p; *p; p++, i++) {
		if (*p == '/') {
			j = i;
		}
	}
	strncpy(ProgramPath, pp, j);
	ProgramPath[j] = 0;
	return ProgramPath;
}


int	DirExists(char *path) {
        DIR	*fh;
        fh = opendir(path);
        if (!fh)
                return 0;
        closedir(fh);
        return 1;
}

int	CreateDir(char *p) {
	char	tmp[256];
	char	*d = tmp;
	int	l=0;
	for ( ; *p; p++, d++, l++) {
		if (l && (*p == '/')) {
			*d = 0;
			if (!DirExists(tmp)) {
				mkdir(tmp, 0777);
				if (!DirExists(tmp))
					return -1;
			}
		}
		*d = *p;
	}
	*d = *p;
	if (!DirExists(tmp)) {
		mkdir(tmp, 0777);
		if (!DirExists(tmp))
			return -1;
	}
	return 0;
}

int	ParseParameters(int argc, char **argv, cmdline_param *clp) {
	int	i, j;
	for (i=0; clp->paramkey[0]; i++, clp++) {
		int	l = strlen(clp->paramkey);
		for (j=1; j<argc; j++) {
			if (strncmp(argv[j], clp->paramkey, l))
				continue;
			if (!sscanf(argv[j]+l, clp->readfmt, clp->paramptr))
				continue;
			break;
		}
	}
	return 0;
}

int	PrintParameters(cmdline_param *clp) {
	int	i;
	char	val[512];
	for (i=0; clp->paramkey[0]; i++, clp++) {
		if (clp->readfmt[1] == 's')
			sprintf(val, clp->readfmt, clp->paramptr);
		else
			sprintf(val, clp->readfmt, * (int *) clp->paramptr);
		LogPrint("%s%s\n", clp->paramkey, val);
	}
	return 0;
}

int	ParseParametersFromFile(char *FName, cmdline_param *clps) {
	int	i;
	cmdline_param *clp;
	char	buf[1024];
	FILE	*fp = fopen(FName, "rt");
	int	cnt;

	if (!fp) {
		//perror(FName);
		//LogPrint("Could not open file %s\n", FName);
		return 0;
	}
	for (cnt=0 ; fgets(buf, 1024, fp); ) {
		char	*p=buf;
		for (i=0; (i<1024) && *p; i++, p++)
			if ((*p == 13) || (*p == 10))
				break;
		*p = 0;
		if (!i)
			continue;
		for (clp=clps, i=0; clp->paramkey[0]; i++, clp++) {
			int	l = strlen(clp->paramkey);
			if (strncmp(buf, clp->paramkey, l))
				continue;
			if (!sscanf(buf+l, clp->readfmt, clp->paramptr))
				continue;
			if (clp->readfmt[1] == 's')
				sprintf(clp->oldvalue, clp->readfmt, clp->paramptr);
			else
				sprintf(clp->oldvalue, clp->readfmt, * (int *) clp->paramptr);
			cnt++;
			break;
		}
	}
	fclose(fp);
//	LogPrint("%d params parsed\n", cnt);
	return cnt;
}

int	ParamsChanged(cmdline_param *clp) {
	int	i;
	char	buf[512];
	int	rv = 0;
	LogPush("ParamsChanged");
	for (i=0; clp->paramkey[0]; i++, clp++) {
		if (clp->readfmt[1] == 's')
			sprintf(buf, clp->readfmt, clp->paramptr);
		else
			sprintf(buf, clp->readfmt, * (int *) clp->paramptr);
		if (strcmp(buf, clp->oldvalue)) {
			if (clp->oldvalue[0])
 				LogPrint("Ayar degisti [%s%s]->[%s%s]\n", clp->paramkey, clp->oldvalue, clp->paramkey, buf);
			else
				LogPrint("Ayar yapildi [%s%s]\n", clp->paramkey, buf);
			rv++;
		}
	}
	LogPop();
	return rv;
}

int	SaveParameters(char *FName, cmdline_param *clp) {
	int	i;
	FILE	*fp;

	LogPush("SaveParameters");
	if (!ParamsChanged(clp)) {
		goto l0;
	}
	fp = fopen(FName, "wt");
	if (!fp) {
		LogPrint("Ayarlar [%s] dosyasina kaydedilemedi. Hata[%d] = \"%s\"\n", FName, errno, strerror(errno));
		goto l0;
	}
	for (i=0; clp->paramkey[0]; i++, clp++) {
LogPrint("Recording %s\n", clp->paramkey);
		fprintf(fp, "%s", clp->paramkey);
		if (clp->readfmt[1] == 's')
			fprintf(fp, clp->readfmt, clp->paramptr);
		else
			fprintf(fp, clp->readfmt, * (int *) clp->paramptr);
		fprintf(fp, "\n");
	}
	fclose(fp);
	LogPrint("Ayarlar kaydedildi\n");
l0:
	LogPop();
	return 0;
}

static	int		LogInitialized = 0;
static	int		log_level = 0;
static	char	*log_prefix[32];
static	char	log_buf[1024] = "";

void	LogInit(void) {
	LogInitialized = 1;
}

void	LogClose(void) {
	LogInitialized = 0;
}

void	LogPush(char *s) {
	log_prefix[log_level++] = s;
}

void	LogPop(void) {
	if (log_level)
		log_level--;
}
void	LogPutPrefix(char *p) {
	int	i;
	if (!Sys.IsDaemon)
	p += sprintf(p, "%c[%d;%d;%dm", 27, 1, 31, 49);
//	p += sprintf(p, "%c[%d;%d;%dm", 27, 1, 36, 44);
	for (i=0; i<log_level; i++) {
		p += sprintf(p, "%s", log_prefix[i]);
		if (!Sys.IsDaemon)
		p += sprintf(p, "%c[%d;%d;%dm.", 27, 1, 32, 49);
		else
		p += sprintf(p, ".");
		if (!Sys.IsDaemon)
		p += sprintf(p, "%c[%d;%d;%dm", 27, 1, 31, 49);
//p += sprintf(p, "%c[%d;%d;%dm", 27, 1, 36, 44);
	}
	if (!Sys.IsDaemon)
	p += sprintf(p, "%c[0;;m", 27);
}

void	LogPrint(char *Str, ...) {
	va_list	ap;
	char	tmp_buf[1024];
	char	ca[3][8]={"", "", ""};
	char	*p, *sp;

	if (!LogInitialized)
		return;
	
	va_start(ap, Str);
	vsprintf(tmp_buf, Str, ap);
	va_end(ap);
	if (!*log_buf)
		LogPutPrefix(log_buf);
	for (sp = p = tmp_buf; *p; p++) {
		if (*p == '\n') {
			strncat(log_buf, sp, p-sp);
			if (!Sys.IsDaemon) {
				puts(log_buf);
				fflush(stdout);
			} else {
				syslog(LOG_NOTICE, "%s", log_buf);
			}
			sp = p+1;
			if (*(p+1))
				LogPutPrefix(log_buf);
			else
				*log_buf = 0;
		} else if (*p == '{') {
			int	a[3] = {-1, -1, -1}, j=0;
			char	*pp;
			for (pp = p+1; *pp; pp++) {
				if (*pp == '|')
					j++;
				else if (*pp >= '0' && *pp <= '9') {
					if (a[j] < 0)
						a[j] = 0;
					a[j] = a[j] * 10 + *pp-'0';
				} else if (*pp == '}')
					break;
				else
					continue;
			}
			if (*pp != '}' || j != 2)
				continue;
			strncat(log_buf, sp, p-sp);
			p = pp;
			sp = pp+1;

			if (a[0] >= 0)
				sprintf(ca[0], "%d", a[0]);
			else
				ca[0][0]=0;
			if (a[1] >= 0)
				sprintf(ca[1], "%d", a[1]+30);
			else
				ca[1][0]=0;
			if (a[2] >= 0)
				sprintf(ca[2], "%d", a[2]+40);
			else
				ca[2][0]=0;
			if (!gIsDaemon) {
				char	cz[32];
				sprintf(cz, "%c[%s;%s;%sm", 27, ca[0], ca[1], ca[2]);
				strcat(log_buf, cz);
			}
		}
	}
	if (sp+1 != p)
		strncat(log_buf, sp, p-sp);
}


llist_entry *get_entry(llist_hdr *hdr) {
	llist_entry *le;

	if (!(le = hdr->ff))
		goto l0;
	if ((hdr->ff = le->n))
		le->n->p = NULL;
	else
		hdr->lf = NULL;
	le->p = NULL;

	hdr->fc--;
	hdr->c++;
	if ((le->n = hdr->f)) 
		le->n->p = le;
	else
		hdr->l = le;
	hdr->f = le;
l0:
	return le;
}

int	del_entry(llist_hdr *hdr, llist_entry *he) {
	if (he->n)
		he->n->p = he->p;
	else
		hdr->l = he->p;

	if (he->p)
		he->p->n = he->n;
	else
		hdr->f = he->n;

	if ((he->n = hdr->ff))
		hdr->ff->p = he;
	else
		hdr->lf = he;
	he->p = NULL;
	hdr->ff = he;
	hdr->fc++;
	hdr->c--;
	return 0;
}

llist_entry *init_llist(int n, llist_hdr *hdr, int size) {
	llist_entry	*le, *res = NULL;
	int		i;

	if (!hdr)
		goto l0;

	res = le = calloc(size, n);
	
	if (!res)
		goto l0;
		
	hdr->fc = n;
	hdr->c = 0;
	hdr->ff = le;
	for (i=0; i<n; i++, le = (llist_entry *) (((char *) le) + size)) {
		if ((le->ix = i))
			le->p = (llist_entry *) (((char *) le) - size);
		else
			le->n = NULL;
		if (i < n-1)
			le->n = (llist_entry *) (((char *) le) + size);
		else {
			hdr->lf = le;
			le->n = NULL;
		}
	}
	hdr->f = hdr->l = NULL;
l0:
	return res;
}

e_writepool	*WritePoolInit(e_writepool *p, int MaxElems) {
	p->entries = init_llist(MaxElems, &p->lh, sizeof(e_writepool_entry));
	if (!p->entries) {
		return NULL;
	}
	return p;
}

void	WritePoolFree(e_writepool *p) {
	if (!p)
		return;
	if (!p->entries)
		free(p->entries);
	bzero(p, sizeof(e_writepool));
	return;
}

void	WPFree(HandleList *hl, HandleEntry *he) {
	e_writepool_entry *e, *ne = NULL;
	for (e = (e_writepool_entry *) he->wph.first; e; e=ne) {
		ne = e->next;
		if (e->prev) {
			e->prev->next = NULL;
			e->prev = NULL;
		}
		if (e->owner != he) {
			LogPrint("FUNNY THING HAPPENED!\n");
		}
		e->owner = NULL;
		del_entry(&hl->WP.lh, (llist_entry *) e);
	}
}

int	WPWrite(HandleList *hl, int type, HandleEntry *he, char *data, int len) {
	e_writepool_entry	*l, *e;
	if (!hl)
		return 1;
	if (!he)
		return 2;
	if (!he->inuse)
		return 3;
	l = (e_writepool_entry *) he->wph.last;
	if (l) {
;//		LogPrint("WP Prev exists!\n");
	}
	if (type == WPADD_STATIC_DATA) {
		e = (e_writepool_entry *) get_entry(&hl->WP.lh);
		if (!e)

			return 2;
		else
			;//LogPrint("WP: %d, %d\n", hl->WP.lh.fc, hl->WP.lh.c);
		e->type = type;
		if ((e->prev = l)) {
			l->next = e;
		} else
			he->wph.first = e;
		e->next = NULL;
		e->owner = he;
		e->ofs = 0;
		e->len = len;
		e->dp = (BYTE *) data;
		e->ttl = 0;
		he->wph.last = e;
		he->wph.TotalWrite += len;
		return 0;
	} else if (type == WPADD_DYNAMIC_DATA) {
		int	x;
		while (len > 0) {
			e = (e_writepool_entry *) get_entry(&hl->WP.lh);
			if (!e)
				return 2;
			else
				;//LogPrint("WP: %d, %d\n", hl->WP.lh.fc, hl->WP.lh.c);
			e->type = type;
			if ((e->prev = l)) {
				l->next = e;
			} else
				he->wph.first = e;
			if ((x = len) > WRITE_POOL_GRANULARITY)
				x = WRITE_POOL_GRANULARITY;
			e->next = NULL;
			he->wph.TotalWrite += x;
			e->owner = he;
			e->ofs = 0;
			e->len = x;
			memcpy(e->data, data, x);
			e->dp = NULL;
			e->ttl = 0;
			l = e;
			len -= x;
			data += x;
			he->wph.last = e;
		}
		return 0;
	}
	return 100;
}



int	WPDoWrite(HandleList *hl, HandleEntry *he) {
	struct	iovec	ovec[64];
	static	e_writepool_entry	*e, *pe, *penext;
	int	i, n, p;

	if (!hl)
		return 1;
	if (!he)
		return 1;
	e = he->wph.first;

	if (!e) {
		ClrWaitToWrite(hl, he);
;//		LogPrint("Nothing to write\n");
		return 0;
	}

	for (i=0, pe = e; pe; pe = pe->next, i++) {
		if (pe->type == WPADD_STATIC_DATA)
			ovec[i].iov_base = pe->dp + pe->ofs;
		else
			ovec[i].iov_base = pe->data + pe->ofs;
		ovec[i].iov_len = pe->len - pe->ofs;
	}

	n = writev(he->fhout, ovec, i);
;//	LogPrint("writev = %d\n", n);

	if (n < 0) {
		if (errno != EAGAIN)
			return 1;
	} else {
		he->wph.TotalWrite -= n;
	}

	for (i=0, p=0, pe = e; pe; pe = penext) {
		int	al = pe->len - pe->ofs;
		penext = pe->next;
		if (p > n)
			break;
		if ((al+p) <= n) {
			pe->len = pe->ofs = 0;
			pe->dp = NULL;
			if (pe->owner != he) {
				LogPrint("Should not occur %p != %p\n", pe->owner, he);
			}
			pe->owner = NULL;
			if (pe->prev)
				pe->prev->next = pe->next;
			else
				he->wph.first = pe->next;
			if (pe->next)
				pe->next->prev = NULL;
			else
				he->wph.last = NULL;
			pe->next = pe->prev = NULL;
			del_entry(&hl->WP.lh, (llist_entry *) pe);
;//			LogPrint("WP: %d, %d\n", hl->WP.lh.fc, hl->WP.lh.c);
		} else {
			pe->ofs += n - p;
			break;
		}
		p += al;
	}
	if (!he->wph.first) {
;//		LogPrint("No more vectors %d\n", he->wph.TotalWrite);

		ClrWaitToWrite(hl, he);
	} else {
;//		LogPrint("More vectors to write %d\n", he->wph.TotalWrite);

		WaitToWrite(hl, he);
	}
	return 0;
}

HandleList	*InitHandles(int MaxHandles, int MaxWPEntries, int timerinterval, timerhandler_t th) {
	HandleList	*hl;
	int		fd;
	int		i;
	HandleEntry	*he;

	LogPrint("InitHandles %d\n", MaxHandles);
	fd = epoll_create(MaxHandles);
	if (fd <= 0) {
		perror("epoll_create");
		return NULL;
	}
	LogPrint("Poll handle initialized %d\n", fd);
	hl = (HandleList *) calloc(sizeof(HandleList)+(sizeof(HandleEntry) + sizeof(struct epoll_event)
	)*MaxHandles, 1);

	if (!hl) {
		LogPrint("InitHandles1 %d\n", fd);
		close(fd);
		return NULL;
	}

	hl->MaxWPEntries = MaxWPEntries*MaxHandles;
	if (!WritePoolInit(&hl->WP, hl->MaxWPEntries)) {
		LogPrint("WritePoolInit %d\n", fd);
		close(fd);
		free(hl);
		return NULL;
	}

	hl->pollfd = fd;
	hl->pollcount = 0;
	hl->startidx = 1;
	hl->Handles = (HandleEntry * ) (((char *) hl) + sizeof(HandleList));
	hl->polls = (struct epoll_event *) (((char *) hl->Handles) + (sizeof(HandleEntry) * MaxHandles));
	hl->MaxHandles = MaxHandles;
	hl->TimerInterval = timerinterval;
	hl->TimerEvent = th;

	he = hl->Handles;
	hl->lh.ff = (llist_entry *) he;
	for (i=0; i<hl->MaxHandles; i++, he++) {
		if (i)
			he->le.p = (llist_entry *) &hl->Handles[i-1];
		if (i<hl->MaxHandles-1)
			he->le.n = (llist_entry *) &hl->Handles[i+1];
		he->le.ix = i;
	}
	hl->lh.fc = i;
	hl->lh.lf = (llist_entry *) he;
	hl->lh.f = NULL;
	hl->lh.l = NULL;
	return hl;
}
static	int	___idx = 0;

HandleEntry	*GetHandle(HandleList *hl) {
	HandleEntry	*he;


	if (!(he = (HandleEntry *) get_entry(&hl->lh)))
		return NULL;
	he->inuse = 1;
 	hl->totalcount++;
	he->__idx = ___idx++;
	return he;
}

void	CloseHandle(HandleList *hl, HandleEntry *he) {
	if (!hl) {
		LogPrint("CloseHandle no hl\n");
		return;
	}
	if (!he) {
		LogPrint("CloseHandle no he\n");
		return;
	}
	if (!he->inuse) {
		LogPrint("CloseHandle he inuse\n");
		return;
	}
	del_entry(&hl->lh, &he->le);
	he->inuse = 0;
	if (he->CallBack)
		(*he->CallBack)(hl, he, EVENT_FREE, NULL, 0, he->ExtraData);

	if (he->pid) {
		int	status;
		LogPrint("killing something %d\n", he->pid);
		kill(he->pid, 9	);
		waitpid(he->pid, &status, WNOHANG);
		LogPrint("killed something %d\n", he->pid);
	}
	//	fprintf(stderr, "CLOSING HANDLE l1\n"); fflush(stderr);
	
	if (he->fhin > 0) {
		//fprintf(stderr, "CLOSING HANDLE l1.0\n"); fflush(stderr);
		if (he->pollfd) {
			//fprintf(stderr, "CLOSING HANDLE l1.1\n"); fflush(stderr);
			epoll_ctl(he->pollfd, EPOLL_CTL_DEL, he->fhin, NULL);
		}
		//fprintf(stderr, "CLOSING HANDLE l1.1\n"); fflush(stderr);
	
		close(he->fhin);
	}
//fprintf(stderr, "CLOSING HANDLE l2\n"); fflush(stderr);	
	//LogPrint("CLOSING HANDLE l2\n");
	fflush(stdout);
	if (he->fhout > 0)
		close(he->fhout);
	//LogPrint("CLOSING HANDLE l3\n");
	
	WPFree(hl, he);
	//LogPrint("CLOSING HANDLE l4\n");
	
	if (hl->pollcount)
		hl->pollcount--;
	//LogPrint("CLOSING HANDLE l5\n");
			
	bzero(((char *) he)+sizeof(llist_entry), sizeof(HandleEntry)-sizeof(llist_entry));
	//LogPrint("DONE CLOSING HANDLE\n");
}

void	CloseHandles(HandleList *hl) {
	int	i;
	LogPush("CloseHandles");
	LogPrint("closing\n");
	for (i=hl->MaxHandles-1;i>=0; i--) {
		if (!hl->Handles[i].inuse)
			continue;
		if (hl->Handles[i].handletype == SERVER_HANDLE)
			continue;
		LogPrint("closing %s\n", hl->Handles[i].name);
		CloseHandle(hl, hl->Handles+i);
	}
	for (i=hl->MaxHandles-1;i>=0; i--) {
		if (!hl->Handles[i].inuse)
			continue;
		LogPrint("closing %s\n", hl->Handles[i].name);
		CloseHandle(hl, hl->Handles+i);
		LogPrint("closed2 one\n");
	}
	close(hl->pollfd);
	WritePoolFree(&hl->WP);
	free(hl);
	LogPrint("done\n");
	LogPop();
}

int     __SignalHandler(HandleList *hl, HandleEntry *he, int cmd, char *data, int len, char *ExtraData) {
	//LogPrint("_ProcessLineCB %s %d\n", he->name, cmd);
	switch (cmd) {
		case EVENT_CREATE: 
			LogPrint("Singal handler created!\n");			
		break;
		case EVENT_FREE: case EVENT_CLOSE:
			LogPrint("Singal handler cleared!\n");
		break;
		case EVENT_RAW: {
			struct signalfd_siginfo si; 
			ssize_t res;
			for ( ; (res = read (he->fhin, &si, sizeof(si))) == sizeof(si); ) {
				SigHandler(si.ssi_signo);
			}
		}
		break;
		default:
			LogPrint("EVENT (%d)\n", cmd);
	}
	return 0;  
}


HandleEntry	*AddSignal(HandleList *hl, void *cb) {
	HandleEntry	*he;
	int     k, pollres;
	
	LogPush("AddSignal");
	he = GetHandle(hl);
	if (!he) {
		LogPrint("No handle\n");
		LogPop();
		return NULL;
	}
	bzero(((char *) he)+sizeof(llist_entry), sizeof(HandleEntry)-sizeof(llist_entry));
//	he->ExtraData = ed;
	he->CallBack = cb;
	he->fhin = he->fhout = signalfd(-1, &__sigmask, 0);
	if (he->fhin < 0) {
		CloseHandle(hl, he);
		LogPop();
		return NULL;
	}
	fcntl(he->fhin, F_SETFL, O_NONBLOCK);

	strcpy(he->name, "SIGNAL");
	he->CallBack = cb;
	he->inuse = 1;
	he->Timeout = 0;
	he->EventTimeout = 0;
	he->handletype = SIGNAL_HANDLE;

	k = he->le.ix;
	hl->polls[k].events = EPOLLIN | EPOLLHUP | EPOLLERR; 
	hl->polls[k].data.ptr = he;
	he->pf = hl->polls+k;
	he->pollfd = hl->pollfd;
	if ((pollres = epoll_ctl(he->pollfd, EPOLL_CTL_ADD, he->fhin, he->pf)) < 0) {
		perror("epoll_ctl");
		LogPrint("epoll_ctl add failed\n");
		CloseHandle(hl, he);
		LogPop();
		return NULL;
	}
	if (cb)
		(*he->CallBack)(hl, he, EVENT_CREATE, NULL, 0, NULL);
	LogPrint("Signalcreated\n");
	LogPop();
	return he;
}

void	PollHandles(HandleList *hl) {
	int	i, /*k = 0,*/ bytes, bytestotal;
	struct	timeval	tvson;
	struct	timeval	tvilk;
	int	pollres, delta;
	int	timecheck = 0, timecheck2 = 0, timeadder = 0;
	struct	epoll_event *events, *e;
	HandleEntry	*he;
	char	buffer[4096];

	if (!hl) {
		LogPrint("PollHandles no hl\n");
		return;
	}
	events = calloc(sizeof(struct epoll_event), hl->MaxHandles);
	if (!events)
		return;
	//__ssync = sem_open("__ssync.main", O_CREAT, 0666, 0);
	//sem_init(&__ssync, 1, 1);
	pthread_mutex_init(&__ssync, NULL);
	gettimeofday(&tvilk, NULL);
	if (hl->TimerEvent)
		if ((*hl->TimerEvent)(hl, 0, tvilk));
/*		
	sem_post(&__ssync);
	for ( ; !Sys.DoStop; / *LogPrint("MAIN LOOP OUT!\n"),* / 
		sem_post(&__ssync)) {
		//) {
		if (!sem_wait(&__ssync)) {
			;// LogPrint("MAIN LOOP IN %d!\n", Sys.DoStop);
		}
*/		
	AddSignal(hl, __SignalHandler);
	for ( ; !Sys.DoStop; 

) {
		pthread_mutex_unlock(&__ssync);
		pollres = epoll_wait(hl->pollfd, events, hl->MaxHandles, (hl->TimerInterval+timeadder)/5000);
//		LogPrint("MAIN LOOP IN %d!\n", Sys.DoStop);
		
		pthread_mutex_lock(&__ssync);
		gettimeofday(&tvson, NULL);
		delta = (tvson.tv_sec - tvilk.tv_sec)*1000000;
		delta += (tvson.tv_usec - tvilk.tv_usec);
		tvilk = tvson;
		Sys.Now = tvson.tv_sec;
		Sys.uNow = tvson.tv_usec;
		Sys.tv = tvson;
		if (pollres < 0) {
			int	err = errno;
			perror("PollHandles.poll:");
			LogPrint("epoll_wait error %d\n", err);
			if (errno == EINTR) {
				LogPrint("Forgive interrupt\n", err);
				continue;
			}
			break;
		}

		timecheck+=delta;
		timecheck2 += delta;
		for (e=events, i=0; i<pollres; i++, e++) {
			he = (HandleEntry *) e->data.ptr;
			if (!he->inuse) {
				LogPrint("Handle not in use\n");
				continue;
			}
			if (!he->pf) {
				LogPrint("Handle has no corresponding poll data\n");
				continue;
			}
#ifdef	POLL_DEBUG
			LogPrint("H:%d, [", he->fhin);

			if (e->events & EPOLLIN)
				LogPrint("IN ");
			if (e->events & EPOLLOUT)
				LogPrint("OUT ");

			if (e->events & EPOLLERR)
				LogPrint("ERR ");

			if (e->events & EPOLLHUP)
				LogPrint("HUP ");


			if (e->events & EPOLLPRI)
				LogPrint("PRI ");

			if (e->events & EPOLLET)
				LogPrint("ET ");

			if (e->events & EPOLLONESHOT)
				LogPrint("ONESHOT ");
			LogPrint("]\n");
#endif
			if (e->events & (EPOLLHUP | EPOLLERR)) {
//				if (e->events & EPOLLHUP) 
//					if (he->handletype == PROCESS_HANDLE)
//						continue;
				if (!Sys.DoStop) {
					int       error = 0;
socklen_t errlen = sizeof(error);
if (getsockopt(he->fhin, SOL_SOCKET, SO_ERROR, (void *)&error, &errlen) == 0)
{
    printf("error = %s\n", strerror(error));
}
					LogPrint("Drop 2\n");

				}
				CloseHandle(hl, he);
				continue;
			}

			if (e->events & EPOLLOUT) {
				if (he->Connecting) {
					he->Writable = 1;
					he->Connecting = 0;
					if (he->handletype != PROCESS_HANDLE)
						if ((*he->CallBack)(hl, he, EVENT_CONNECT, NULL, 0, NULL)) {
							CloseHandle(hl, he);
						}
				} else {
					if (he->handletype != PROCESS_HANDLE)
						{
							he->Writable = 1;
							if ((*he->CallBack)(hl, he, EVENT_WRITE, NULL, 0, NULL)) {
								CloseHandle(hl, he);
							}
						}
					he->Writable = 1;
				}
			}
		}
		for (e=events, i=0; i<pollres; i++, e++) {
			he = (HandleEntry *) e->data.ptr;
			if (!he->inuse)
				continue;
			if (!he->pf)
				continue;

			if (e->events & (EPOLLHUP | EPOLLERR)) {
				continue;
			}

			if (e->events & EPOLLIN) {

					he->TimeoutValue = 0;
				
				if (he->handletype == SIGNAL_HANDLE) {
					if (he->CallBack)
						(he->CallBack)(hl, he, EVENT_RAW, NULL, 0, NULL);
				} else if (he->handletype == RAW_HANDLE) {
					if (he->CallBack)
						(he->CallBack)(hl, he, EVENT_RAW, NULL, 0, NULL);
				} else if (he->handletype == GPS_HANDLE) {
					if (he->CallBack)
						(he->CallBack)(hl, he, EVENT_GPS, NULL, 0, NULL);
				} else if (he->CallBack && (he->handletype == SERVER_HANDLE)) {
					(he->CallBack)(hl, he, EVENT_CONNECT, NULL, 0, NULL);
				} else if (he->CallBack && (he->handletype == UDP_HANDLE) ) {
					LogPrint("UDP IN %s\n", he->name);
					bytestotal = 0;
					if (ioctl(he->fhin, FIONREAD, &bytestotal)) {
						perror("IOCTL error");
						LogPrint("Drop IOCTL\n");
						CloseHandle(hl, he);
						continue;
					}
					while (bytestotal > 0) {
						int	*raddrlen = (int *) buffer;
						if (bytestotal >= 4096 - sizeof(struct sockaddr_in) - sizeof(int))
							bytes = 4096 - sizeof(struct sockaddr_in) - sizeof(int);
						else
							bytes = bytestotal;
						*raddrlen = sizeof(struct sockaddr_in);
						bytes = recvfrom(
								he->fhin,
								buffer+sizeof(struct sockaddr_in)+sizeof(int),
								bytes,
								0,
								(struct sockaddr *) (buffer+sizeof(int)),
								(socklen_t *) raddrlen
						);
						if (bytes <= 0) {
							break;
						} else
						#ifdef	POLL_DEBUG
							LogPrint("Read: %d bytes\n", bytes);
						#else
							;
						#endif
						if (he->CallBack)
							if ((*he->CallBack)(hl, he, EVENT_DATA, buffer, bytes+sizeof(int)+sizeof(struct sockaddr_in), NULL)) {
								CloseHandle(hl, he);
								break;
							}
						bytestotal -= bytes;

					}
				} else if (he->CallBack && (he->handletype == ETH_HANDLE) ) {
					static BYTE	rdata[512];
					static DWORD	rdlen;
					
				//	LogPrint("ETH IN %s\n", he->name);
					bytestotal = 0;
					if (ioctl(he->fhin, FIONREAD, &bytestotal)) {
						perror("IOCTL error");
						LogPrint("Drop IOCTL\n");
						CloseHandle(hl, he);
						continue;
					}
					while (bytestotal > 0) {
//						int	*raddrlen = (int *) buffer;
						if (bytestotal >= 4096)
							bytes = 4096;
						else
							bytes = bytestotal;
						rdlen = 6;
						bytes = recvfrom(
								he->fhin,
								buffer,
								bytes,
								0,
								(struct sockaddr *) rdata,
								(socklen_t *) &rdlen
						);
						if (bytes <= 0) {
							break;
						} else
						#ifdef	POLL_DEBUG
							LogPrint("Read: %d bytes\n", bytes);
						#else
							;
						#endif
						if (he->CallBack)
							if ((*he->CallBack)(hl, he, EVENT_DATA, buffer, bytes+sizeof(int)+sizeof(struct sockaddr_in), NULL)) {
								CloseHandle(hl, he);
								break;
							}
						bytestotal -= bytes;

					}
				} else {
					bytestotal = 0;
					if (he->handletype == DEV_HANDLE) {
						bytestotal = 8;
					} else
						if (ioctl(he->fhin, FIONREAD, &bytestotal)) {
						perror("IOCTL error");
						LogPrint("Drop IOCTL\n");
						CloseHandle(hl, he);
						continue;
					}
					if (he->handletype == PROCESS_HANDLE)
						;
					if (bytestotal <= 0) {
						if (he->handletype != COM_HANDLE ) {
							LogPrint("Drop 3\n");
							CloseHandle(hl, he);
						}
						continue;
					}
					while (bytestotal > 0) {
						if (bytestotal >= 4096)
							bytes = 4096;
						else
							bytes = bytestotal;
						bytes = read(he->fhin, buffer, bytes);
						if (bytes <= 0) {
							if (he->handletype == COM_HANDLE)
								break;
							LogPrint("Drop 4 %d %d\n", bytestotal, bytes);
							perror("RH");
							CloseHandle(hl, he);
							break;
						} else
						#ifdef	POLL_DEBUG
							LogPrint("Read: %d bytes\n", bytes);
						#else
							;
						#endif
						if (he->CallBack)
							if ((*he->CallBack)(hl, he, EVENT_DATA, buffer, bytes, NULL)) {
								CloseHandle(hl, he);
								break;
							}
						bytestotal -= bytes;
					}
				}
			}
		}
		if ((timecheck) >= (hl->TimerInterval+timeadder)) {
			delta = (hl->TimerInterval+timeadder);
			delta -= timecheck;
			if (delta < 0) {
				if (delta > -hl->TimerInterval)
					timeadder = delta;
			} else
				if (delta < hl->TimerInterval)
					timeadder = delta;
			/*if (timeadder >= (hl->TimerInterval/30))
				timeadder = 0;
*/

			if (hl->TimerEvent)
				if ((*hl->TimerEvent)(hl, timecheck, tvson))
					break;
			timecheck = 0;
		}
		if ((timecheck2) >= ((hl->TimerInterval+timeadder)/10)) {
			delta = timecheck2;
			{
				for (he = (HandleEntry *) hl->lh.l; he; he = (HandleEntry *) he->le.p) {
					if (!he->inuse)
						continue;
					if (!he->pf)
						continue;
					if (he->EventTimeout) {
						if (delta < 10000000)
							he->EventTimeoutValue += delta;

						if (he->EventTimeout*0.95 <= he->EventTimeoutValue) {

							if (he->CallBack) {
								(*he->CallBack)(hl, he, EVENT_TIMEOUT, NULL, 0, he->ExtraData);
							}
							he->EventTimeoutValue = 0;
						}
					}
					if (!he->Timeout)
						continue;
					if (delta < 10000000)
						he->TimeoutValue += delta;
					if (he->TimeoutValue >= he->Timeout) {
						CloseHandle(hl, he);
						continue;
					}
				}
			}
			timecheck2 = 0;
		}

	}
	LogPrint("OUT OF LOOP\n");
	fflush(stdout);
//	sem_close(&__ssync);
//	sem_destroy(&__ssync);
	pthread_mutex_destroy(&__ssync);
	LogPrint("PollHandles}\n");
}

int	HandleWrite(HandleList *hl, HandleEntry *he, char *buf, int a) {
	int	r;
	r = write(he->fhout, buf, a);
	if (r != a) {
		CloseHandle(hl, he);
		return 1;
	}
	return 0;
}

int	HandlePrint(HandleList *hl, HandleEntry *he, char *fmt, ...) {
	int	i;
	static	char	output[4096];
	va_list	ap;

	va_start(ap, fmt);
	i = vsprintf(output, fmt, ap);
	va_end(ap);
	return HandleWrite(hl, he, output, i);
}

int	WaitToWrite(HandleList *hl, HandleEntry *he) {
	if (!he->pf)
		return 0;
	if (he->pf->events & EPOLLOUT)
		return 0;
	he->pf->events |= EPOLLOUT;
	epoll_ctl(he->pollfd, EPOLL_CTL_MOD, he->fhin, he->pf);
	return 1;
}

int	ClrWaitToWrite(HandleList *hl, HandleEntry *he) {
	if (!(he->pf->events & EPOLLOUT))
		return 0;
	he->pf->events ^= EPOLLOUT;
	epoll_ctl(he->pollfd, EPOLL_CTL_MOD, he->fhin, he->pf);
	return 1;
}

int	GetMAC(char *intf, char *hwaddr) {
	struct ifreq	ifr;
	struct ifconf	ifc;
	char 			buf[1024];
	int 			sock;
	int				r = 1;
	struct ifreq	*it;
	struct ifreq	*end;
	int				i;

	sock  = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock == -1) {
		goto l0;
	}

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
		goto l1;
	}
	it = ifc.ifc_req;
	end = it + (ifc.ifc_len / sizeof(struct ifreq));

	for (; it != end; ++it) {
        	strcpy(ifr.ifr_name, it->ifr_name);
		if (intf) {
			if (strcmp(intf, ifr.ifr_name))
				continue;
		}
		if (ioctl(sock, SIOCGIFFLAGS, &ifr) != 0)
			continue;
		if (ifr.ifr_flags & IFF_LOOPBACK)
			continue;
                if (ioctl(sock, SIOCGIFHWADDR, &ifr))
			continue;
		r = 0;
		for (i=0; i<6; i++)
			sprintf(hwaddr+i*2, "%02X", ifr.ifr_hwaddr.sa_data[i]);
		break;
	}
l1:
	close(sock);
l0:
	return r;
}
void    sigchld_handler(int signum) {
	int     pid, status;
	while ((pid = waitpid(WAIT_ANY, &status, WNOHANG)) > 0) {
		LogPrint("Child process [%d] terminated with value [%d]\n", pid, status);
		DeletePid(pid);
		usleep(100);
	}
}

void	SigHandler(int sig) {
/*	 struct sigaction setup_action;
  sigset_t block_mask;

  sigemptyset (&block_mask);
  
  sigaddset (&block_mask, SIGINT);
  sigaddset (&block_mask, SIGQUIT);
  setup_action.sa_handler = catch_stop;
  setup_action.sa_mask = block_mask;
  setup_action.sa_flags = 0;
  sigaction (SIGTSTP, &setup_action, NULL);
  */
//	LogPrint("SIGNAL RECVD!\n");
fprintf(stderr, "SIGNAL RECVD %d %d!\n", sig, Sys.DoStop);	
	Sys.DoStop = 1;
	Sys.SignalId = sig;
	fprintf(stderr, "SIGNAL RECVD %d %d!\n", sig, Sys.DoStop);	
	fflush(stdout);
//	if (sig != SIGKILL)
//		raise(SIGKILL); /* EKREM degisecek sigint calismiyor */
}

void	SigKiller(int sig) {
	Sys.SignalId = sig;
}


int	LinMain(int argc, char **argv, int (*frun)(int, char **, int), void (*fcleanup)(int), timerhandler_t timer, cmdline_param *params) {
	int	a = 0;
	int	isfirstrun = 0;
	/*
	unsigned char	version[32] = {
		VERSION_MAJOR_INIT,
		'.',
		VERSION_MINOR_INIT,
		'-', 'D',
//		'-',
		BUILD_YEAR_CH0, BUILD_YEAR_CH1, BUILD_YEAR_CH2, BUILD_YEAR_CH3,
//		'-',
		BUILD_MONTH_CH0, BUILD_MONTH_CH1,
//		'-',
		BUILD_DAY_CH0, BUILD_DAY_CH1,
		'T',
		BUILD_HOUR_CH0, BUILD_HOUR_CH1,
//		':',
		BUILD_MIN_CH0, BUILD_MIN_CH1,
//		':',
		BUILD_SEC_CH0, BUILD_SEC_CH1,
		'\0'
	};
	strcpy(Sys.CompleteVersion, (char *) version);
	 */
	GetVersion(Sys.CompleteVersion); 
	GetProgramName(argv[0], Sys.ProgramName);
#ifdef	PI_SPECIFIC
	getRaspberryPiInformation(&Sys.RPI_Info);
#endif
	for (a = 1; a < argc; a++)
		if (strcmp(argv[a], "-va") == 0) {
			printf("%s_%s\n", Sys.AppName, Sys.CompleteVersion);
			return 0;
		} else if (strcmp(argv[a], "-v") == 0) {
			printf("%s\n", Sys.CompleteVersion);
			return 0;
		} else if (strcmp(argv[a], "-a") == 0) {
			printf("%s\n", Sys.AppName);
			return 0;
		}
	
	LogInit();

	bzero(&Sys.last_tm, sizeof(Sys.last_tm));

	GetMAC("eth0", Sys.MAC);	
	if (!Sys.MAC[0] || !strcmp("000000000000", Sys.MAC))
		GetMAC("venet0", Sys.MAC);
	if (!Sys.MAC[0] || !strcmp("000000000000", Sys.MAC))
		GetMAC("wlan0", Sys.MAC);
	if (!Sys.MAC[0] || !strcmp("000000000000", Sys.MAC))
		GetVZID(Sys.MAC);	
	GetCPUID(Sys.CPUID, &Sys.nProcessors);
	if (strcmp(Sys.CPUID, "1-1-1") == 0)
		strcpy(Sys.CPUID, Sys.MAC);
		
	{ 
		char	ProgramPath[512];
		GetProgramPath(argv[0], ProgramPath);
		chdir(ProgramPath);
//		LogPrint("WORKING DIR: %s\n", ProgramPath);
	}
	
	strcpy(Sys.ParamFileName, "conf");
	if (!DirExists(Sys.ParamFileName)) {
		isfirstrun = 1;

		if (CreateDir(Sys.ParamFileName)) {
			LogPrint("Ayar dizini olusturulamadi: %s\n", Sys.ParamFileName);
			goto l0;
		}
	}
	strcat(Sys.ParamFileName, "/");
	strcat(Sys.ParamFileName, Sys.ProgramName);
	strcat(Sys.ParamFileName, ".cfg");

	if (!ParseParametersFromFile(Sys.ParamFileName, params))
		isfirstrun = 1;
	ParseParameters(argc, argv, params);
	SaveParameters(Sys.ParamFileName, params);
	LogPush(Sys.ProgramName);
	//signal(SIGTERM, SigHandler);
	
	sigemptyset (&__sigmask);
	sigaddset(&__sigmask, SIGINT);
	sigaddset(&__sigmask, SIGTERM);

	sigprocmask (SIG_BLOCK, &__sigmask, NULL);
/*
	if (signal(SIGINT, (__sighandler_t) SigHandler) == SIG_ERR)
		LogPrint("Couldn't catch SIGINT\n");
*/		 
	{
		struct sigaction new_action, old_action;

		new_action.sa_handler = SigHandler;
		sigemptyset (&new_action.sa_mask);
		new_action.sa_flags = 0;
		//sigaction (SIGINT, NULL, &old_action);
		sigaction (SIGINT, &new_action,  &old_action);
//		sigaction (SIGUSR1, NULL, &old_action);
//		sigaction (SIGUSR1, &new_action, NULL);
	//	sigaction (SIGALRM, NULL, &old_action);
	//	sigaction (SIGALRM, &new_action, NULL);
	}
	 
/* 
	signal(SIGHUP, SigHandler);
	signal(SIGALRM, SigHandler);
	signal(SIGABRT, SigHandler);
	signal(SIGQUIT, SigHandler);
	signal(SIGUSR1, SigHandler);
	*/
	signal(SIGCHLD, sigchld_handler);
//	signal(SIGKILL, fcleanup);
	Sys.IsDaemon = gIsDaemon;
	if (Sys.IsDaemon) {
		if (daemon(1, 0) < 0) {
			LogPrint("Daemon error, going stand alone...\n");
			Sys.IsDaemon = 0;
		} else {
			;
		}
	}
	if (Sys.IsDaemon) {
		setlogmask(LOG_UPTO(LOG_NOTICE));
		openlog(Sys.ProgramName, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	}
	a = (*frun)(argc, argv, isfirstrun);
	if (a) {
		if ((Sys.Handles = InitHandles(
							Sys.MaxHandles,
							Sys.MaxHandles*Sys.WritePoolSize,
							Sys.Interval,
							timer
		))) {
			PollHandles(Sys.Handles);
			CloseHandles(Sys.Handles);
		}
	}
	LogPrint("Cleanup\n");
	Sys.DoStop = 1;
	(*fcleanup)(Sys.SignalId);
	LogPrint("Finished\n");
	if (Sys.IsDaemon) {
		closelog();
	}
l0:
	return a;
}


int     _ProcessLineCB(HandleList *hl, HandleEntry *he, int cmd, char *data, int len, char *ExtraData) {
	int		r;
	//LogPrint("_ProcessLineCB %s %d\n", he->name, cmd);
	switch (cmd) {
		case EVENT_CONNECT: {
			_processlinecontext	*lc = (_processlinecontext *) he->ExtraData;
			if (lc)
				return (*lc->cb)(hl, he, EVENT_CONNECT, data, len, ExtraData);
			else
				LogPrint("NO CALLBACK %s !!!\n", he->name);
		}
		break;
		case EVENT_FREE: {
			_processlinecontext	*lc = (_processlinecontext *) he->ExtraData;
			if (lc) {
				(*lc->cb)(hl, he, EVENT_FREE, data, len, ExtraData);
				free(lc);
				he->ExtraData = NULL;
			}
		}
		break;
		case EVENT_DATA: {
			_processlinecontext	*lc = (_processlinecontext *) he->ExtraData;
			data[len] = 0;
			//LogPrint("DAta %s\n", data);
			while (len && lc->zcnt) {

				if (*data == '\r' || *data == '\n')
					lc->zcnt--;
				else
					lc->zcnt = lc->ncnt;
				len--;
				data++;
			}

			if (lc) {
				for ( ; len; ) {
					if (*data == '#')
						if (!lc->p) {

							lc->state = -1-lc->state;
							goto l2;
						}
					if (lc->state < 0) {
						if (*data == 13 || *data == 10) {
							lc->state = -1-lc->state;
						}
						goto l2;
					}
					switch (lc->state) {
					case 4:
						if (*data == 13 || *data == 10)
							lc->state = 0;
						goto l2;
					case 0: // START
						if (*data == '{') {

							lc->parcnt++;
							lc->state = 1;
							goto l2;
						}
						goto l1;
					case 1:
						if (*data == '}') {
							if (lc->parcnt) {
								lc->parcnt--;
								if (lc->parcnt == 0) {
									lc->l[lc->p++] = '}';
									lc->l[lc->p] = 0;
									if ((r = (*lc->cb)(hl, he, EVENT_DATA, lc->l, lc->p, ExtraData)))
										return r;
									lc->p = 0;
									lc->state = 0;
									goto l1;
								}
								goto l2;
							} else
								goto l1;
						}
						if (*data == '{')
							lc->parcnt++;
						if (*data == '\"')
							lc->state = 2;
						goto l2;
					case 2:
						if (*data == '\"')
							lc->state = 1;
						else if (*data == '\\')
							lc->state = 3;
						goto l2;
					case 3:
						lc->state = 2;
						goto l2;
					}
l2:
					lc->l[lc->p++] = *data;
l1:
					data++;
					len--;
				}
			}
		}
		break;
		case EVENT_EXECUTE: {
			_processlinecontext	*lc = (_processlinecontext *) he->ExtraData;
			if (lc)
				execvp(lc->command, lc->params);
		}
		break;
		case EVENT_WRITE:
			WPDoWrite(hl, he);
		break;
		default:
			LogPrint("EVENT (%d)\n", cmd);
	}
	return 0;
}

char	*SkipSpace(char *l) {
	while (*l == 13 || *l == 10 || *l == 9 || *l == 32 || *l == ',')
		l++;
	return l;
}

char	*SkipStr(char *s) {
	while (*s) {
		if (*s == '\"') {
			return ++s;
		} else if (*s == '\\') {
			s++;
			if (*s)
				s++;
			continue;
		}
		s++;
	}
	return s;
}

char	*ParseStr(char *d, char *s) {
	while (*s) {
		if (*s == '\"') {
			*d = 0;
			return ++s;
		} else if (*s == '\\') {
			s++;
			switch (*s) {
			case 'n': *(d++) = '\n'; break;
			case 'r': *(d++) = '\r'; break;
			case 't': *(d++) = '\t'; break;
			default: *(d++) = '\n'; break;
			}
			if (*s)
				s++;
			continue;
		}
		*(d++) = *(s++);
	}
	return s;
}
char	*ParsePar(char *s) {
	int	c = 1;
	while (*s) {
		if (*s == '}') {
			c--;
			if (c == 0) {
				*s = ' ';
				return s;
			}
			s++;
		} else if (*s == '\"') {
			s = SkipStr(s+1);
		} else if (*s == '{') {
			c++;
			s++;
		} else
			s++;
	}
	return s;
}
char	*ParsePar2(char *s) {
	int	c = 1;
	while (*s) {
		if (*s == '}') {
			c--;
			if (c == 0) {
				return ++s;
			}
			s++;
		} else if (*s == '\"') {
			s = SkipStr(s+1);
		} else if (*s == '{') {
			c++;
			s++;
		} else
			s++;
	}
	return s;
}
char	*GetJSONTok(char *l, char **name, char **value) {
l0:
	l = SkipSpace(l);
	switch (*l) {
	case 0:
		return NULL;
	case '\"':
		*name = ++l;
		l = ParseStr(*name, l);
		break;
	case '{':
		*(l++) = ' ';
		ParsePar(l);
		goto l0;
	default:
		*name = l;
		while (*l) {
			if (*l == ':' || *l == 32 || *l == 13 || *l == 10 || *l == 9)
				break;
			l++;
		}

		if (*l != ':') {
			*l = 0;
			l = SkipSpace(l+1);
		}
		if (*l != ':')
			return NULL;
		*l = 0;
		l++;
	}
	l = SkipSpace(l);
	switch (*l) {
		case 0:
			return NULL;
		case '\"':
			*value = ++l;
			return ParseStr(*value, l);
		case '{':
			*value = l;
			l = ParsePar2(l+1);
			*l = 0;
			return ++l;
	}
	return l;
}



/* public API - prototypes - TODO: doxygen*/


/* code */
#define SHA1_K0 0x5a827999
#define SHA1_K20 0x6ed9eba1
#define SHA1_K40 0x8f1bbcdc
#define SHA1_K60 0xca62c1d6

const uint8_t sha1InitState[] = {
  0x01,0x23,0x45,0x67, // H0
  0x89,0xab,0xcd,0xef, // H1
  0xfe,0xdc,0xba,0x98, // H2
  0x76,0x54,0x32,0x10, // H3
  0xf0,0xe1,0xd2,0xc3  // H4
};

void sha1_init(sha1nfo *s) {
  memcpy(s->state.b,sha1InitState,HASH_LENGTH);
  s->byteCount = 0;
  s->bufferOffset = 0;
}

uint32_t sha1_rol32(uint32_t number, uint8_t bits) {
  return ((number << bits) | (number >> (32-bits)));
}

void sha1_hashBlock(sha1nfo *s) {
  uint8_t i;
  uint32_t a,b,c,d,e,t;

  a=s->state.w[0];
  b=s->state.w[1];
  c=s->state.w[2];
  d=s->state.w[3];
  e=s->state.w[4];
  for (i=0; i<80; i++) {
    if (i>=16) {
      t = s->buffer.w[(i+13)&15] ^ s->buffer.w[(i+8)&15] ^ s->buffer.w[(i+2)&15] ^ s->buffer.w[i&15];
      s->buffer.w[i&15] = sha1_rol32(t,1);
    }
    if (i<20) {
      t = (d ^ (b & (c ^ d))) + SHA1_K0;
    } else if (i<40) {
      t = (b ^ c ^ d) + SHA1_K20;
    } else if (i<60) {
      t = ((b & c) | (d & (b | c))) + SHA1_K40;
    } else {
      t = (b ^ c ^ d) + SHA1_K60;
    }
    t+=sha1_rol32(a,5) + e + s->buffer.w[i&15];
    e=d;
    d=c;
    c=sha1_rol32(b,30);
    b=a;
    a=t;
  }
  s->state.w[0] += a;
  s->state.w[1] += b;
  s->state.w[2] += c;
  s->state.w[3] += d;
  s->state.w[4] += e;
}

void sha1_addUncounted(sha1nfo *s, uint8_t data) {
  s->buffer.b[s->bufferOffset ^ 3] = data;
  s->bufferOffset++;
  if (s->bufferOffset == BLOCK_LENGTH) {
    sha1_hashBlock(s);
    s->bufferOffset = 0;
  }
}

void sha1_writebyte(sha1nfo *s, uint8_t data) {
  ++s->byteCount;
  sha1_addUncounted(s, data);
}

void sha1_write(sha1nfo *s, const char *data, size_t len) {
	for (;len--;) sha1_writebyte(s, (uint8_t) *data++);
}

void sha1_pad(sha1nfo *s) {
  // Implement SHA-1 padding (fips180-2 Â§5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  sha1_addUncounted(s, 0x80);
  while (s->bufferOffset != 56) sha1_addUncounted(s, 0x00);

  // Append length in the last 8 bytes
  sha1_addUncounted(s, 0); // We're only using 32 bit lengths
  sha1_addUncounted(s, 0); // But SHA-1 supports 64 bit lengths
  sha1_addUncounted(s, 0); // So zero pad the top bits
  sha1_addUncounted(s, s->byteCount >> 29); // Shifting to multiply by 8
  sha1_addUncounted(s, s->byteCount >> 21); // as SHA-1 supports bitstreams as well as
  sha1_addUncounted(s, s->byteCount >> 13); // byte.
  sha1_addUncounted(s, s->byteCount >> 5);
  sha1_addUncounted(s, s->byteCount << 3);
}

uint8_t* sha1_result(sha1nfo *s) {
  int i;
  // Pad to complete the last block
  sha1_pad(s);

  // Swap byte order back
  for (i=0; i<5; i++) {
    uint32_t a,b;
    a=s->state.w[i];
    b=a<<24;
    b|=(a<<8) & 0x00ff0000;
    b|=(a>>8) & 0x0000ff00;
    b|=a>>24;
    s->state.w[i]=b;
  }

  // Return pointer to hash (20 characters)
  return s->state.b;
}

#define HMAC_IPAD 0x36
#define HMAC_OPAD 0x5c

void sha1_initHmac(sha1nfo *s, const uint8_t* key, int keyLength) {
  uint8_t i;
  memset(s->keyBuffer, 0, BLOCK_LENGTH);
  if (keyLength > BLOCK_LENGTH) {
    // Hash long keys
    sha1_init(s);
    for (;keyLength--;) sha1_writebyte(s, *key++);
    memcpy(s->keyBuffer, sha1_result(s), HASH_LENGTH);
  } else {
    // Block length keys are used as is
    memcpy(s->keyBuffer, key, keyLength);
  }
  // Start inner hash
  sha1_init(s);
  for (i=0; i<BLOCK_LENGTH; i++) {
    sha1_writebyte(s, s->keyBuffer[i] ^ HMAC_IPAD);
  }
}

uint8_t* sha1_resultHmac(sha1nfo *s) {
  uint8_t i;
  // Complete inner hash
  memcpy(s->innerHash,sha1_result(s),HASH_LENGTH);
  // Calculate outer hash
  sha1_init(s);
  for (i=0; i<BLOCK_LENGTH; i++) sha1_writebyte(s, s->keyBuffer[i] ^ HMAC_OPAD);
  for (i=0; i<HASH_LENGTH; i++) sha1_writebyte(s, s->innerHash[i]);
  return sha1_result(s);
}



/* aaaack but it's fast and const should make it shared text page. */
static const unsigned char pr2six[256] =
{
    /* ASCII table */
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

int Base64decode_len(const char *bufcoded)
{
    int nbytesdecoded;
    register const unsigned char *bufin;
    register int nprbytes;

    bufin = (const unsigned char *) bufcoded;
    while (pr2six[*(bufin++)] <= 63);

    nprbytes = (bufin - (const unsigned char *) bufcoded) - 1;
    nbytesdecoded = ((nprbytes + 3) / 4) * 3;

    return nbytesdecoded + 1;
}

int Base64decode(char *bufplain, const char *bufcoded)
{
    int nbytesdecoded;
    register const unsigned char *bufin;
    register unsigned char *bufout;
    register int nprbytes;

    bufin = (const unsigned char *) bufcoded;
    while (pr2six[*(bufin++)] <= 63);
    nprbytes = (bufin - (const unsigned char *) bufcoded) - 1;
    nbytesdecoded = ((nprbytes + 3) / 4) * 3;

    bufout = (unsigned char *) bufplain;
    bufin = (const unsigned char *) bufcoded;

    while (nprbytes > 4) {
    *(bufout++) =
        (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
    *(bufout++) =
        (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
    *(bufout++) =
        (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
    bufin += 4;
    nprbytes -= 4;
    }

    /* Note: (nprbytes == 1) would be an error, so just ingore that case */
    if (nprbytes > 1) {
    *(bufout++) =
        (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
    }
    if (nprbytes > 2) {
    *(bufout++) =
        (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
    }
    if (nprbytes > 3) {
    *(bufout++) =
        (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
    }

    *(bufout++) = '\0';
    nbytesdecoded -= (4 - nprbytes) & 3;
    return nbytesdecoded;
}

static const char basis_64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int Base64encode_len(int len)
{
    return ((len + 2) / 3 * 4) + 1;
}

int Base64encode(char *encoded, const char *string, int len)
{
    int i;
    char *p;

    p = encoded;
    for (i = 0; i < len - 2; i += 3) {
    *p++ = basis_64[(string[i] >> 2) & 0x3F];
    *p++ = basis_64[((string[i] & 0x3) << 4) |
                    ((int) (string[i + 1] & 0xF0) >> 4)];
    *p++ = basis_64[((string[i + 1] & 0xF) << 2) |
                    ((int) (string[i + 2] & 0xC0) >> 6)];
    *p++ = basis_64[string[i + 2] & 0x3F];
    }
    if (i < len) {
    *p++ = basis_64[(string[i] >> 2) & 0x3F];
    if (i == (len - 1)) {
        *p++ = basis_64[((string[i] & 0x3) << 4)];
        *p++ = '=';
    }
    else {
        *p++ = basis_64[((string[i] & 0x3) << 4) |
                        ((int) (string[i + 1] & 0xF0) >> 4)];
        *p++ = basis_64[((string[i + 1] & 0xF) << 2)];
    }
    *p++ = '=';
    }

    *p++ = '\0';
    return p - encoded;
}


char *repl_str(const char *str, const char *old, const char *new) {

	/* Adjust each of the below values to suit your needs. */

	/* Increment positions cache size initially by this number. */
	size_t cache_sz_inc = 16;
	/* Thereafter, each time capacity needs to be increased,
	 * multiply the increment by this factor. */
	const size_t cache_sz_inc_factor = 3;
	/* But never increment capacity by more than this number. */
	const size_t cache_sz_inc_max = 1048576;

	char *pret, *ret = NULL;
	const char *pstr2, *pstr = str;
	size_t i, count = 0;
	char *pos_cache = NULL;
	size_t cache_sz = 0;
	size_t cpylen, orglen, retlen, newlen, oldlen = strlen(old);

	/* Find all matches and cache their positions. */
	while ((pstr2 = strstr(pstr, old)) != NULL) {
		count++;

		/* Increase the cache size when necessary. */
		if (cache_sz < count) {
			cache_sz += cache_sz_inc;
			pos_cache = realloc(pos_cache, sizeof(*pos_cache) * cache_sz);
			if (pos_cache == NULL) {
				goto end_repl_str;
			}
			cache_sz_inc *= cache_sz_inc_factor;
			if (cache_sz_inc > cache_sz_inc_max) {
				cache_sz_inc = cache_sz_inc_max;
			}
		}

		pos_cache[count-1] = pstr2 - str;
		pstr = pstr2 + oldlen;
	}

	orglen = pstr - str + strlen(pstr);

	/* Allocate memory for the post-replacement string. */
	if (count > 0) {
		newlen = strlen(new);
		retlen = orglen + (newlen - oldlen) * count;
	} else	retlen = orglen;
	ret = malloc(retlen + 1);
	if (ret == NULL) {
		goto end_repl_str;
	}

	if (count == 0) {
		/* If no matches, then just duplicate the string. */
		strcpy(ret, str);
	} else {
		/* Otherwise, duplicate the string whilst performing
		 * the replacements using the position cache. */
		pret = ret;
		memcpy(pret, str, pos_cache[0]);
		pret += pos_cache[0];
		for (i = 0; i < count; i++) {
			memcpy(pret, new, newlen);
			pret += newlen;
			pstr = str + pos_cache[i] + oldlen;
			cpylen = (i == count-1 ? orglen : pos_cache[i+1]) - pos_cache[i] - oldlen;
			memcpy(pret, pstr, cpylen);
			pret += cpylen;
		}
		ret[retlen] = '\0';
	}

end_repl_str:
	/* Free the cache and return the post-replacement string,
	 * which will be NULL in the event of an error. */
	free(pos_cache);
	return ret;
}

void	GetDevIP(char *dev, char *ip) {
 int fd;
 struct ifreq ifr;

	*ip = 0;
	
 fd = socket(AF_INET, SOCK_DGRAM, 0);
if (fd <= 0)
	return;
 /* I want to get an IPv4 IP address */
 ifr.ifr_addr.sa_family = AF_INET;

 /* I want IP address attached to "eth0" */
 strncpy(ifr.ifr_name, dev, IFNAMSIZ-1);

 ioctl(fd, SIOCGIFADDR, &ifr);

 close(fd);

 sprintf(ip, "%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

 return;
}
int	InterfaceStatus(char *dev, char *ip, char *operstate, int *carrier) {
	char	fname[64];
	int		rv = 0;
	
	char	oldip[32];
	char	oldoperstate[32];
	int		oldcarrier;
	
	oldcarrier = *carrier;
	strcpy(oldip, ip);
	strcpy(oldoperstate, operstate);
	
	FILE	*fp;
	sprintf(fname, "/sys/class/net/%s/carrier", dev);
	if ((fp = fopen(fname, "rt"))) {
		fscanf(fp, "%d", carrier);
		fclose(fp);
	} else {
		*carrier = 0;
	}
	sprintf(fname, "/sys/class/net/%s/operstate", dev);
	if ((fp = fopen(fname, "rt"))) {
		fscanf(fp, "%s", operstate);
		fclose(fp);
	} else {
		*operstate = 0;
	}
	GetDevIP(dev, ip);
	if (!*ip)
		strcpy(ip, " ");
	if (*carrier != oldcarrier)
		rv |= 1;
	if (strcmp(operstate, oldoperstate))
		rv |= 2;
	if (strcmp(ip, oldip))
		rv |= 4;
	return rv;
}

int	CheckNet(char *dev, char *ip) {
	char	fname[64];
	char	state[64] = "unknown";
	int		carrier = 0;
	int		rv = 0;
	
	FILE	*fp;
	sprintf(fname, "/sys/class/net/%s/carrier", dev);
	if ((fp = fopen(fname, "rt"))) {
		fscanf(fp, "%d", &carrier);
		fclose(fp);
	} else {
		carrier = 0;
	}
	sprintf(fname, "/sys/class/net/%s/operstate", dev);
	if ((fp = fopen(fname, "rt"))) {
		fscanf(fp, "%s", state);
		fclose(fp);
	}
	if (carrier) {
		rv |= 1;		
	} 
	if (!strcmp(state, "up"))
		rv |= 4;
	if (!strcmp(state, "down"))
		rv |= 2;
//	LogPrint("[ %10s ]:%2d %10s, %4d\n", dev, rv, state, carrier);
	
	if (ip != NULL)  {
		if ((rv & 5) == 5) {
			GetDevIP(dev, ip);
		} else
			*ip = 0;
	}
	if (!*ip)
		strcpy(ip, " ");
	return rv;
}

double	CheckTemp(void) {
//	char	state[64] = "unknown";
	int		carrier = 0;
//	int		rv = 0;
	
	FILE	*fp;
	if ((fp = fopen("/sys/class/thermal/thermal_zone0/temp", "rt"))) {
		fscanf(fp, "%d", &carrier);
		fclose(fp);
	}
	
	
	return carrier / 1000.0;
}

// This works on both linux and MacOSX (and any BSD kernel).
int	detect_gdb(void) {
/*	static int	isCheckedAlready = 0;
	if (!isCheckedAlready) {
		if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0)
			underDebugger = 1;
		usleep(100000);
		ptrace(PTRACE_DETACH, 0, 0, 0);
		isCheckedAlready = 1;
	}
*/
	underDebugger = 1;
	return underDebugger == 1;
}


void	GetCPUID(char *CPUID, int *processors) {
	FILE	*fp;
	char	buf[4096];
	char	Hardware[32] = "1";
	char	Revision[32] = "1";
	char	Serial[32] = "1";
	int		processor = 0;
	
//	uname(&unam);
	if (!(fp = fopen("/proc/cpuinfo", "rb"))) {
		LogPrint("Could not open file\n");
		goto l0;
	}
	buf[sizeof(buf)-1] = 0;
	while (fgets(buf, sizeof(buf)-2, fp)) {
		if (sscanf(buf, "Hardware%*[ \n\t]:%s\n", Hardware) == 1)
			;
		if (sscanf(buf, "Revision%*[ \n\t]:%s\n", Revision) == 1)
			;
		if (sscanf(buf, "Serial%*[ \n\t]: %*[0]%s\n", Serial) == 1)
			;
		if (sscanf(buf, "processor%*[ \n\t]: %d\n", &processor) == 1)
			;
	}
	fclose(fp);
	*processors = ++processor;
l0:
	sprintf(CPUID, "%s-%s-%s", Hardware, Revision, Serial);;
}

void	GetVZID(char *p) {
	FILE	*fp = fopen("/proc/vz/veinfo", "rb");
	if (fp) {
		fscanf(fp, "%s", p);
		fclose(fp);		
	} else
		*p = 0;
}
