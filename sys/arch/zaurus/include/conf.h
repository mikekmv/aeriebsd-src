
#ifndef _ZAURUS_CONF_H
#define	_ZAURUS_CONF_H

#include <sys/conf.h>

/*
 * ZAURUS specific device includes go in here
 */

#define CONF_HAVE_APM
#define	CONF_HAVE_USB
#define	CONF_HAVE_WSCONS

#include <arm/conf.h>

#endif	/* _ZAURUS_CONF_H */
