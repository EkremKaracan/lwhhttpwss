/*
	Copyright (C) 2015+ Ekrem Karacan
	ekrem.karacan@gmail.com
*/
#ifndef utils_h
#define	utils_h

#include	<netinet/in.h>
#include	<time.h>
#include	<stdint.h>
#include	<signal.h>
#include	<semaphore.h>
#ifdef	PI_SPECIFIC
#include	<raspberry_pi_revision.h>
#endif
typedef	uint8_t		BYTE;
typedef	uint16_t	WORD;
typedef	uint32_t	DWORD;
typedef int32_t		int32;
typedef	uint64_t	QWORD;

#define	diff(a, b)	((a) >= (b) || (a) <= -(b))

#include	"version.h"

// { filesdirsnames.c
char    *timetohttptime(time_t *tt, char *d);
char	*GetFileExt(char *p, char *ext);
char	*GetProgramName(char *p, char *ProgramName);
char	*GetProgramPath(char *p, char *ProgramPath);
int		DirExists(char *path);
int		CreateDir(char *p);

// } filesdirsnames.c

// { params.c
typedef	struct {
	char	paramkey[32];
	char	readfmt[16];
	char	*paramptr;
	char	oldvalue[256];
} cmdline_param;
int	ParseParameters(int argc, char **argv, cmdline_param *clp);
int	ParseParametersFromFile(char *FName, cmdline_param *clp);
int	SaveParameters(char *FName, cmdline_param *clp);
int	PrintParameters(cmdline_param *clp);
// } params.c

// { log.c
void	LogInit(void);
void	LogClose(void);
void	LogPush(char *s);
void	LogPop(void);
void	LogPutPrefix(char *p);
void	LogPrint(char *Str, ...);
// } log.c

// { llist.c
typedef struct st_llist_entry {
	struct st_llist_entry *n, *p;
	int	ix, filla;
} llist_entry;
typedef	struct st_llist_hdr {
	struct st_llist_entry *f, *l, *ff, *lf;
	int	fc, c;
} llist_hdr;
llist_entry	*get_entry(llist_hdr *hdr);
int		del_entry(llist_hdr *hdr, llist_entry *he);
llist_entry *init_llist(int n, llist_hdr *hdr, int size);
// } llist.c

// { writepool.c
struct stHandleList;
struct stHandleEntry;
#define	WRITE_POOL_GRANULARITY	1480
#define	WPADD_STATIC_DATA	1
#define	WPADD_DYNAMIC_DATA	2
typedef	struct e_writepool_entry_st {
	llist_entry	le;
	struct		stHandleEntry	*owner;
	struct e_writepool_entry_st	*next, *prev;
	int		type;
	DWORD		ofs;
	DWORD		len;
	DWORD		ttl;
	BYTE		*dp;
	BYTE		data[WRITE_POOL_GRANULARITY];
} e_writepool_entry;

typedef	struct {
	llist_hdr	lh;
	llist_entry	*entries;
} e_writepool;

typedef	struct {
	DWORD		TotalWrite;
	e_writepool_entry	*first;
	e_writepool_entry	*last;
} e_writepool_hdr;
// } writepool.c

// { handles.c
typedef int (*timerhandler_t)(void *p, int delta, struct timeval tv);
typedef	struct stHandleEntry {
	llist_entry	le;
	DWORD	__idx;
	struct	sockaddr_in	SockAddr;
	int	pollfd;
	e_writepool_hdr		wph;
	char	IpAddress[32];
	char	RemoteAddress[32];
	WORD	rPort;
	DWORD	rIP;
	char	name[512];
	int	Connecting;
	int	Writable;
	int	inuse;
	int	fhin, fhout, pid;
	int	handletype;
	int	TimeoutValue;
	int	Timeout;
	int	EventTimeoutValue;
	int	EventTimeout;
#define	SERVER_HANDLE	0
#define	CLIENT_HANDLE	1
#define	PROCESS_HANDLE	2
#define	COM_HANDLE	3
#define	UDP_HANDLE	4
#define	TS_HANDLE	5
#define	FILE_HANDLE	6
#define	DEV_HANDLE	10
#define	GPS_HANDLE	11
#define	RAW_HANDLE	12
#define	ETH_HANDLE	13
#define	SIGNAL_HANDLE	14
	struct	epoll_event	*pf;
	int	(*CallBack)(struct stHandleList *, struct stHandleEntry *he, int, char *, int, char *);
	char	*ExtraData;
	char	*ExtraData2;
	int	writecnt;
	struct	stHandleEntry	*Parent;
#define	EVENT_CONNECT	0
#define	EVENT_CLOSE	1
#define	EVENT_ERROR	2
#define	EVENT_TIMEOUT	3
#define	EVENT_TIMER	4
#define	EVENT_FREE	5
#define	EVENT_DATA	6
#define	EVENT_RUN	7
#define	EVENT_CREATE	8
#define	EVENT_EXECUTE	16
#define	EVENT_WRITE	32
#define	EVENT_GPS	64
#define	EVENT_RAW	128
#define	EVENT_DNS	256

} HandleEntry;

typedef	struct stHandleList {
	llist_hdr	lh;
	unsigned long	totalcount;
	int		MaxHandles;
	e_writepool	WP;
	int		startidx;
	int		TimerInterval;
	timerhandler_t	TimerEvent;
	int		pollfd;
	int		pollcount;
	int		MaxWPEntries;
	HandleEntry	*Handles;
	struct	epoll_event	*polls;
} HandleList;

HandleList	*InitHandles(int MaxHandles, int MaxWPEntries, int timerinterval, timerhandler_t th);
void		CloseHandle(HandleList *hl, HandleEntry *he);
void		CloseHandles(HandleList *hl);
HandleEntry	*GetHandle(HandleList *hl);
void	PollHandles(HandleList *hl);
int		HandleWrite(HandleList *hl, HandleEntry *he, char *buf, int a);
int		HandlePrint(HandleList *hl, HandleEntry *he, char *fmt, ...);
int		ClrWaitToWrite(HandleList *hl, HandleEntry *he);
int		WaitToWrite(HandleList *hl, HandleEntry *he);
// } handles.c

// { writepool.c
int	WPWrite(HandleList *hl, int type, HandleEntry *he, char *data, int len);
int	WPDoWrite(HandleList *hl, HandleEntry *he);
// } writepool.c

// { init.c
int	LinMain(int argc, char **argv, int (*frun)(int, char **, int), void (*fcleanup)(int), timerhandler_t timer, cmdline_param *params);
// } init.c

typedef	struct {
	struct	tm		last_tm; // = {0, 0, 0, 0, 0, 0};
	struct	tm		cur_tm;
	int				IsDaemon; // = 0;
	volatile sig_atomic_t			DoStop; // = 0;
	int				SignalId;
	time_t			Now;
	suseconds_t		uNow;
	struct	timeval	tv;
	char			ProgramName[128];  // = "";
	char			AppName[128];
	char			ParamFileName[1024]; // = "";
	char			CompleteVersion[32]; // = "";
	int				MaxHandles;
	int				Interval;
	int				WritePoolSize;
	int				UpdateStatus;
	char			MAC[32]; // = "";
	char			CPUID[32];
	int				nProcessors;
	DWORD			downtotal, downcount;
	char			downtable[64];
	HandleList		*Handles;
#ifdef  PI_SPECIFIC
	RASPBERRY_PI_INFO_T	
					RPI_Info;
#endif
} eGlobals;

eGlobals	Sys;

#ifdef	main_c

	HandleList	*Handles = NULL;
	int		_led = 0;
	int		_blink = 0;
	int		_blinkc = 0;
	int		gIsFirstRun = 1;
#else
	struct	tm	__olt;
	struct	tm	__lt;
	extern	HandleList	*Handles;

	extern	int		gIsFirstRun;
#endif


#define MARKERCOLOR	RGB16(0x20, 0xF0, 0x20)
//#define	BORDERCOLOR	RGB16(0xC0, 0xff, 0xFF)
//#define	BORDERCOLOR	RGB16(0x60, 0x20, 0x20)

#define	BORDERCOLOR	RGB16(0x10, 0x20, 0x10)
//#define	HEADERFONTCOLOR	RGB16(0, 0, 0x80)
//#define	HEADERCOLOR	RGB16(0xFF, 0xFF, 0xc0)
#define	HEADERCOLOR	RGB16(0, 0, 0)
#define	HEADERFONTCOLOR	RGB16(0xFF, 0xFF, 0xFF)
void	SendSSIn(void);
void	SendSSOut(void);
#define	MAX(a, b)	((a) > (b) ? (a) : b)
#define	MIN(a, b)	((a) < (b) ? (a) : b)


typedef	struct {
	int	(*cb)(HandleList *, HandleEntry *, int, char *, int, char *);
	char	*command;
	char	**params;
	char	l[1024];
	int		p;
	int		parcnt;
	int		qcnt;
	int		state;
	int		ncnt, zcnt;
	int		max;
} _processlinecontext;
int     _ProcessLineCB(HandleList *hl, HandleEntry *he, int cmd, char *data, int len, char *ExtraData);
char	*GetJSONTok(char *l, char **name, char **value);


#define HASH_LENGTH 20
#define BLOCK_LENGTH 64

union _buffer {
	uint8_t b[BLOCK_LENGTH];
	uint32_t w[BLOCK_LENGTH/4];
};

union _state {
	uint8_t b[HASH_LENGTH];
	uint32_t w[HASH_LENGTH/4];
};

typedef struct sha1nfo {
	union _buffer buffer;
	uint8_t bufferOffset;
	union _state state;
	uint32_t byteCount;
	uint8_t keyBuffer[BLOCK_LENGTH];
	uint8_t innerHash[HASH_LENGTH];
} sha1nfo;

void sha1_init(sha1nfo *s);
/**
 */
void sha1_writebyte(sha1nfo *s, uint8_t data);
/**
 */
void sha1_write(sha1nfo *s, const char *data, size_t len);
/**
 */
uint8_t* sha1_result(sha1nfo *s);
/**
 */
void sha1_initHmac(sha1nfo *s, const uint8_t* key, int keyLength);
/**
 */
uint8_t* sha1_resultHmac(sha1nfo *s);


int Base64encode(char *encoded, const char *string, int len);
int Base64encode_len(int len);
int Base64decode(char *bufplain, const char *bufcoded);
int Base64decode_len(const char *bufcoded);

char *repl_str(const char *str, const char *old, const char *new);
int	InterfaceStatus(char *dev, char *ip, char *operstate, int *carrier);
int	CheckNet(char *dev, char *ip);
double	CheckTemp(void);
int	detect_gdb(void);

#ifndef	utils_c
	extern	pthread_mutex_t	__ssync;
	extern	int	underDebugger;
	extern	int	gIsDaemon;
#else
	int	underDebugger = 0;
	pthread_mutex_t		__ssync;
	int	gIsDaemon = 0;
#endif

extern	char	*days[7];
extern	char	*months[12];

void	GetCPUID(char *CPUID, int *processors);
void	GetVZID(char *p);
#endif
