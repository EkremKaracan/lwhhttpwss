#include	<string.h>
#include	"version.h"

void	GetVersion(char *p) {
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
	strcpy(p, (char *) version);
}