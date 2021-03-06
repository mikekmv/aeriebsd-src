/*
 * Copyright (c) 2006 Can Erkin Acar <canacar@openbsd.org>
 * Copyright (c) 2005 Marco Peereboom <marco@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/signalvar.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/malloc.h>
#include <sys/kernel.h>

#include <machine/bus.h>

#include <dev/acpi/acpireg.h>
#include <dev/acpi/acpivar.h>
#include <dev/acpi/acpidev.h>
#include <dev/acpi/amltypes.h>
#include <dev/acpi/dsdt.h>

#include <sys/sensors.h>

#define KTOC(k)			((k - 2732) / 10)
#define ACPITZ_MAX_AC		(10)
#define ACPITZ_TMP_RETRY	(3)

struct acpitz_softc {
	struct device		sc_dev;

	struct acpi_softc	*sc_acpi;
	struct aml_node		*sc_devnode;

	int			sc_tmp;
	int			sc_crt;
	int			sc_hot;
	int			sc_ac[ACPITZ_MAX_AC];
	int			sc_ac_stat[ACPITZ_MAX_AC];
	int			sc_pse;
	int			sc_psv;
	int			sc_tc1;
	int			sc_tc2;
	int			sc_lasttmp;
	struct ksensor		sc_sens;
	struct ksensordev	sc_sensdev;
};

int	acpitz_match(struct device *, void *, void *);
void	acpitz_attach(struct device *, struct device *, void *);

struct cfattach acpitz_ca = {
	sizeof(struct acpitz_softc), acpitz_match, acpitz_attach
};

struct cfdriver acpitz_cd = {
	NULL, "acpitz", DV_DULL
};

void	acpitz_monitor(struct acpitz_softc *);
void	acpitz_refresh(void *);
int	acpitz_notify(struct aml_node *, int, void *);
int	acpitz_gettempreading(struct acpitz_softc *, char *);
int	acpitz_getreading(struct acpitz_softc *, char *);
int	acpitz_setfan(struct acpitz_softc *, int, char *);
#if 0
int	acpitz_setcpu(struct acpitz_softc *, int);
#endif

int
acpitz_match(struct device *parent, void *match, void *aux)
{
	struct acpi_attach_args	*aa = aux;
	struct cfdata		*cf = match;

	/* sanity */
	if (aa->aaa_name == NULL ||
	    strcmp(aa->aaa_name, cf->cf_driver->cd_name) != 0 ||
	    aa->aaa_table != NULL)
		return (0);

	if (aa->aaa_node->value->type != AML_OBJTYPE_THERMZONE)
		return (0);

	return (1);
}

void
acpitz_attach(struct device *parent, struct device *self, void *aux)
{
	struct acpitz_softc	*sc = (struct acpitz_softc *)self;
	struct acpi_attach_args	*aa = aux;
	int			i;
	char			name[8];

	sc->sc_acpi = (struct acpi_softc *)parent;
	sc->sc_devnode = aa->aaa_node;

	sc->sc_lasttmp = -1;
	if ((sc->sc_tmp = acpitz_gettempreading(sc, "_TMP")) == -1) {
		printf(": failed to read _TMP\n");
		return;
	}

	if ((sc->sc_crt = acpitz_gettempreading(sc, "_CRT")) == -1)
		printf(": no critical temperature defined\n");
	else
		printf(": critical temperature %d degC\n", KTOC(sc->sc_crt));

	for (i = 0; i < ACPITZ_MAX_AC; i++) {
		snprintf(name, sizeof name, "_AC%d", i);
		sc->sc_ac[i] = acpitz_gettempreading(sc, name);
		sc->sc_ac_stat[0] = -1;
	}

	sc->sc_hot = acpitz_gettempreading(sc, "_HOT");
	sc->sc_tc1 = acpitz_getreading(sc, "_TC1");
	sc->sc_tc2 = acpitz_getreading(sc, "_TC2");
	sc->sc_psv = acpitz_gettempreading(sc, "_PSV");
	dnprintf(10, "%s: _HOT: %d _TC1: %d _TC2: %d _PSV: %d _TMP: %d "
	    "_CRT: %d\n", DEVNAME(sc), sc->sc_hot, sc->sc_tc1, sc->sc_tc2,
	    sc->sc_psv, sc->sc_tmp, sc->sc_crt);

	/* get _PSL */

	strlcpy(sc->sc_sensdev.xname, DEVNAME(sc),
	    sizeof(sc->sc_sensdev.xname));
	strlcpy(sc->sc_sens.desc, "zone temperature",
	    sizeof(sc->sc_sens.desc));
	sc->sc_sens.type = SENSOR_TEMP;
	sensor_attach(&sc->sc_sensdev, &sc->sc_sens);
	sensordev_install(&sc->sc_sensdev);
	sc->sc_sens.value = 0;

	aml_register_notify(sc->sc_devnode, NULL,
	    acpitz_notify, sc, ACPIDEV_POLL);
}

#if 0
int
acpitz_setcpu(struct acpitz_softc *sc, int perc)
{
	struct aml_value res0, *ref;
	int x;

	if (aml_evalname(sc->sc_acpi, sc->sc_devnode, "_PSL", 0, NULL, &res0)) {
		printf("%s: _PSL failed\n", DEVNAME(sc));
		goto out;
	}
	if (res0.type != AML_OBJTYPE_PACKAGE) {
		printf("%s: not a package\n", DEVNAME(sc));
		goto out;
	}
	for (x = 0; x < res0.length; x++) {
		if (res0.v_package[x]->type != AML_OBJTYPE_OBJREF) {
			printf("%s: _PSL[%d] not a object ref\n",
			    DEVNAME(sc), x);
			continue;
		}
		ref = res0.v_package[x]->v_objref.ref;
		if (ref->type != AML_OBJTYPE_PROCESSOR)
			printf("%s: _PSL[%d] not a CPU\n", DEVNAME(sc), x);
	}
 out:
	aml_freevalue(&res0);
	return (0);
}
#endif

int
acpitz_setfan(struct acpitz_softc *sc, int i, char *method)
{
	struct aml_value	res0, res1, res2, *ref;
	char			name[8];
	int			rv = 1, x, y;

	dnprintf(20, "%s: acpitz_setfan(%d, %s)\n", DEVNAME(sc), i, method);

	snprintf(name, sizeof name, "_AL%d", i);
	if (aml_evalname(sc->sc_acpi, sc->sc_devnode, name, 0, NULL, &res0)) {
		dnprintf(20, "%s: %s failed\n", DEVNAME(sc), name);
		goto out;
	}

	if (res0.type != AML_OBJTYPE_PACKAGE) {
		printf("%s: %s not a package\n", DEVNAME(sc), name);
		goto out;
	}

	for (x = 0; x < res0.length; x++) {
		if (res0.v_package[x]->type != AML_OBJTYPE_OBJREF) {
			printf("%s: %s[%d] not a object ref\n", DEVNAME(sc),
			    name, x);
			continue;
		}
		ref = res0.v_package[x]->v_objref.ref;
		if (aml_evalname(sc->sc_acpi, ref->node, "_PR0",0 , NULL,
		    &res1)) {
			printf("%s: %s[%d] _PR0 failed\n", DEVNAME(sc),
			    name, x);
			aml_freevalue(&res1);
			continue;
		}
		if (res1.type != AML_OBJTYPE_PACKAGE) {
			printf("%s: %s[%d] _PR0 not a package\n", DEVNAME(sc),
			    name, x);
			aml_freevalue(&res1);
			continue;
		}
		for (y = 0; y < res1.length; y++) {
			if (res1.v_package[y]->type != AML_OBJTYPE_OBJREF) {
				printf("%s: %s[%d.%d] _PR0 not a package\n",
				    DEVNAME(sc), name, x, y);
				continue;
			}
			ref = res1.v_package[y]->v_objref.ref;
			if (aml_evalname(sc->sc_acpi, ref->node, method, 0,
			    NULL, NULL))
				printf("%s: %s[%d.%d] %s fails\n",
				    DEVNAME(sc), name, x, y, method);

			/* save off status of fan */
			if (aml_evalname(sc->sc_acpi, ref->node, "_STA", 0,
			    NULL, &res2))
				printf("%s: %s[%d.%d] _STA fails\n",
				    DEVNAME(sc), name, x, y);
			else {
				sc->sc_ac_stat[i] = aml_val2int(&res2);
				aml_freevalue(&res2);
			}
		}
		aml_freevalue(&res1);
	}
	rv = 0;
out:
	aml_freevalue(&res0);
	return (rv);
}

void
acpitz_refresh(void *arg)
{
	struct acpitz_softc	*sc = arg;
	int			i, perc;

	dnprintf(30, "%s: %s: refresh\n", DEVNAME(sc),
	    sc->sc_devnode->name);

	/* get _TMP and debounce the value */
	if (-1 == (sc->sc_tmp = acpitz_gettempreading(sc, "_TMP"))) {
		printf("%s: %s: failed to read temp\n", DEVNAME(sc),
		    sc->sc_devnode->name);
		return;
	}
	/* critical trip points */
	if (sc->sc_crt != -1 && sc->sc_crt <= sc->sc_tmp) {
		/* do critical shutdown */
		printf("%s: Critical temperature, shutting down\n",
		    DEVNAME(sc));
		psignal(initproc, SIGUSR2);
	}
	if (sc->sc_hot != -1 && sc->sc_hot <= sc->sc_tmp) {
		printf("%s: _HOT temperature\n", DEVNAME(sc));
		/* XXX go to S4, until then cool as hard as we can */
	}

	/* passive cooling */
	if (sc->sc_lasttmp != -1 && sc->sc_tc1 != -1 && sc->sc_tc2 != -1 &&
	    sc->sc_psv != -1) {
	    	dnprintf(30, "%s: passive cooling: lasttmp: %d tc1: %d "
		    "tc2: %d psv: %d\n", DEVNAME(sc), sc->sc_lasttmp,
		    sc->sc_tc1, sc->sc_tc2, sc->sc_psv);
		if (sc->sc_psv <= sc->sc_tmp) {
			sc->sc_pse = 1;
			perc = sc->sc_tc1 * (sc->sc_tmp - sc->sc_lasttmp) +
			    sc->sc_tc2 * (sc->sc_tmp - sc->sc_psv);
			perc /= 10;
			if (perc < 0)
				perc = 0;
			else if (perc > 100)
				perc = 100;
			/* printf("_TZ perc = %d\n", perc); */
		} else if (sc->sc_pse)
			sc->sc_pse = 0;
	}
	sc->sc_lasttmp = sc->sc_tmp;

	/* active cooling */
	for (i = 0; i < ACPITZ_MAX_AC; i++) {
		if (sc->sc_ac[i] != -1 && sc->sc_ac[i] <= sc->sc_tmp) {
			/* turn on fan i */
			if (sc->sc_ac_stat[i] <= 0)
				acpitz_setfan(sc, i, "_ON_");
		} else if (sc->sc_ac[i] != -1) {
			/* turn off fan i */
			if (sc->sc_ac_stat[i] > 0)
				acpitz_setfan(sc, i, "_OFF");
		}
	}
	sc->sc_sens.value = sc->sc_tmp * 100000;
}

int
acpitz_getreading(struct acpitz_softc *sc, char *name)
{
	struct aml_value	res;
	int			rv = -1;

	if (aml_evalname(sc->sc_acpi, sc->sc_devnode, name, 0, NULL, &res)) {
		dnprintf(10, "%s: acpitz_getreading: no %s\n", DEVNAME(sc),
		    name);
		goto out;
	}
	rv = aml_val2int(&res);
out:
	aml_freevalue(&res);
	return (rv);
}

int
acpitz_gettempreading(struct acpitz_softc *sc, char *name)
{
	int			rv = -1, tmp = -1, i;

	for (i = 0; i < ACPITZ_TMP_RETRY; i++) {
		tmp = acpitz_getreading(sc, name);
		if (tmp == -1)
			goto out;
		if (KTOC(tmp) > 0) {
			rv = tmp;
			break;
		} else {
			dnprintf(20, "%s: %d invalid reading on %s, "
			    "debouncing\n", DEVNAME(sc), tmp, name);
		}

		/* debounce value */
		if (cold)
			delay(1000000);
		else
			while (tsleep(sc, PWAIT, "tzsleep", hz) !=
			    EWOULDBLOCK);
	}
	if (i >= ACPITZ_TMP_RETRY) {
		printf("%s: %s: failed to read %s\n", DEVNAME(sc),
		    sc->sc_devnode->name, name);
		goto out;
	}
 out:
	dnprintf(30, "%s: name: %s tmp: %dK => %dC, rv: %d\n", DEVNAME(sc),
	    name, tmp, KTOC(tmp), rv);
	return (rv);
}

int
acpitz_notify(struct aml_node *node, int notify_type, void *arg)
{
	struct acpitz_softc	*sc = arg;
	int			crt;

	dnprintf(10, "%s notify: %.2x %s\n", DEVNAME(sc), notify_type,
	    sc->sc_devnode->name);

	switch (notify_type) {
	case 0x80:	/* hardware notifications */
		break;
	case 0x81:	/* operating Points changed */
		sc->sc_psv = acpitz_gettempreading(sc, "_PSV");
		crt = sc->sc_crt;
		sc->sc_crt = acpitz_gettempreading(sc, "_CRT");
		if (crt != sc->sc_crt)
			printf("%s: new critical temperature: %u degC",
			    DEVNAME(sc), KTOC(sc->sc_crt));
		break;
	case 0x82:	/* re-evaluate thermal device list */
		break;
	default:
		break;
	}

	acpitz_refresh(sc);
	return (0);
}
