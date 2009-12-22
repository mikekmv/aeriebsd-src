/*
 * Copyright (c) 2009 Michael Shalayeff
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* pic16f818/9 running ablecom's fw monitoring power supply */

/*
 * sample dump:
 * iic0: addr 0x38
 * 00=01 01=01 04=03 05=0a 07=f1 08=01 09=21 0a=5a 0b=5a 0c=01 0d=4b 0e=25
 * 0f=25 17=03 18=01 19=07 1b=c7 1c=41 1d=42 1e=4c 1f=45 20=43 21=4f 22=4d
 * 23=c8 24=53 25=50 26=35 27=30 28=32 29=2d 2a=32 2b=53 2c=c8 2d=50 2e=57
 * 2f=53 30=2d 31=30 32=30 33=34 34=39 35=c3 36=33 37=2e 38=30 39=cf 3a=50
 * 3b=30 3c=30 3d=34 3e=39 3f=30 40=36 41=33 42=31 43=4c 44=32 45=38 46=39
 * 47=33 48=32 49=c0 4a=c0 4b=c1 4f=2f 51=02 52=18 53=df 54=07 55=f4 56=01
 * 57=58 58=02 59=64 5a=64 5b=5a 5d=8c 5f=b4 61=08 62=01 63=2f 64=3f 65=10
 * 66=0e 67=58 68=12 69=35 6a=a0 6c=9c 6d=01 6e=02 6f=0d 70=eb 71=05 72=81
 * 73=f4 74=01 75=19 77=19 79=32 7b=64 7d=d0 7e=07 7f=01 80=02 81=0d 82=15
 * 83=db 84=02 85=4a 86=01 87=0d 89=10 8b=32 8d=f4 8e=01 8f=08 90=52 91=01
 * 92=02 93=0d 94=0f 95=e1 96=03 97=f4 98=01 99=14 9b=19 9d=32 9f=f4 a0=01
 * a1=30 a2=75 a3=01 a4=02 a5=0d a6=89 a7=67 a8=04 a9=b0 aa=04 ab=30 ad=3c
 * af=78 b1=e8 b2=03 b3=58 b4=98 b5=01 b6=82 b7=0d b8=6c b9=04 ba=05 bb=b0
 * bc=04 bd=3c bf=3c c1=78 c5=e8 c6=03
 * words
 * 00=0101 01=0101 02=0000 03=0000 04=0303 05=0a0a 06=0000 07=f1f1
 * 08=0101 09=2222 0a=5959 0b=5b5b 0c=0101 0d=4b4b 0e=2525 0f=2525
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/sensors.h>

#include <dev/i2c/i2cvar.h>

/* registers */
#define	ABLECOM_FAN0	0x0a
#define	ABLECOM_FAN1	0x0b
#define	ABLECOM_STATUS	0x0c
#define	ABLECOM_PSMODEL	0x24
#define	ABLECOM_FWVER	0x2d

#define	ABLECOM_SENS_STATUS	0
#define	ABLECOM_SENS_FAN0	1
#define	ABLECOM_SENS_FAN1	2
#define	ABLECOM_NUM_SENSORS	3

struct ablecom_softc {
	struct device	sc_dev;

	i2c_tag_t	sc_tag;
	i2c_addr_t	sc_addr;
	struct ksensor  sc_sensor[ABLECOM_NUM_SENSORS];
	struct ksensordev sc_sensordev;
};

struct cfdriver ablecom_cd = {
	NULL, "ablecom", DV_DULL
};

int	ablecommatch(struct device *, void *, void *);
void	ablecomattach(struct device *, struct device *, void *);

const struct cfattach ablecom_ca = {
	sizeof(struct ablecom_softc), ablecommatch, ablecomattach
};

u_int8_t ablecom_read(struct ablecom_softc *, u_int8_t);
void	ablecom_refresh(void *);

int
ablecommatch(struct device *parent, void *match, void *aux)
{
	struct i2c_attach_args *ia = aux;

	if (strcmp(ia->ia_name, "ablecom"))
		return 0;

	return 1;
}

u_int8_t
ablecom_read(struct ablecom_softc *sc, u_int8_t reg)
{
	u_int8_t v;

	iic_exec(sc->sc_tag, I2C_OP_READ_WITH_STOP,
	    sc->sc_addr, &reg, sizeof reg, &v, sizeof v, 0);

	return v;
}

void
ablecomattach(struct device *parent, struct device *self, void *aux)
{
	struct ablecom_softc *sc = (struct ablecom_softc *)self;
	struct i2c_attach_args *ia = aux;

	if (!sensor_task_register(sc, ablecom_refresh, 5)) {
		printf(": failed to register update task\n");
		return;
	}

	sc->sc_tag = ia->ia_tag;
	sc->sc_addr = ia->ia_addr;

	/* print out PS model and fw version */
	printf(": %c%c%c%c%c%c%c%c fw %c%c%c%c%c%c%c%c\n",
	    ablecom_read(sc, ABLECOM_PSMODEL + 0),
	    ablecom_read(sc, ABLECOM_PSMODEL + 1),
	    ablecom_read(sc, ABLECOM_PSMODEL + 2),
	    ablecom_read(sc, ABLECOM_PSMODEL + 3),
	    ablecom_read(sc, ABLECOM_PSMODEL + 4),
	    ablecom_read(sc, ABLECOM_PSMODEL + 5),
	    ablecom_read(sc, ABLECOM_PSMODEL + 6),
	    ablecom_read(sc, ABLECOM_PSMODEL + 7),
	    ablecom_read(sc, ABLECOM_FWVER + 0),
	    ablecom_read(sc, ABLECOM_FWVER + 1),
	    ablecom_read(sc, ABLECOM_FWVER + 2),
	    ablecom_read(sc, ABLECOM_FWVER + 3),
	    ablecom_read(sc, ABLECOM_FWVER + 4),
	    ablecom_read(sc, ABLECOM_FWVER + 5),
	    ablecom_read(sc, ABLECOM_FWVER + 6),
	    ablecom_read(sc, ABLECOM_FWVER + 7));

	strlcpy(sc->sc_sensordev.xname, sc->sc_dev.dv_xname,
	    sizeof(sc->sc_sensordev.xname));

	sc->sc_sensor[ABLECOM_SENS_STATUS].type = SENSOR_INDICATOR;
	strlcpy(sc->sc_sensor[ABLECOM_SENS_STATUS].desc, "Status",
	    sizeof(sc->sc_sensor[ABLECOM_SENS_STATUS].desc));

	sc->sc_sensor[ABLECOM_SENS_FAN0].type = SENSOR_FANRPM;
	sc->sc_sensor[ABLECOM_SENS_FAN1].type = SENSOR_FANRPM;

	sensor_attach(&sc->sc_sensordev, &sc->sc_sensor[0]);
	sensor_attach(&sc->sc_sensordev, &sc->sc_sensor[1]);
	sensor_attach(&sc->sc_sensordev, &sc->sc_sensor[2]);
	sensordev_install(&sc->sc_sensordev);
}

void
ablecom_refresh(void *v)
{
	struct ablecom_softc *sc = v;
	u_int8_t val;

	iic_acquire_bus(sc->sc_tag, 0);

	val = ablecom_read(sc, ABLECOM_STATUS);
	sc->sc_sensor[ABLECOM_SENS_STATUS].value = val & 1;
	sc->sc_sensor[ABLECOM_SENS_STATUS].flags &= ~SENSOR_FINVALID;

	val = ablecom_read(sc, ABLECOM_FAN0);
	sc->sc_sensor[ABLECOM_SENS_FAN0].value = 3816 * val * 30 / 1000;
	sc->sc_sensor[ABLECOM_SENS_FAN0].flags &= ~SENSOR_FINVALID;

	val = ablecom_read(sc, ABLECOM_FAN1);
	sc->sc_sensor[ABLECOM_SENS_FAN1].value = 3816 * val * 30 / 1000;
	sc->sc_sensor[ABLECOM_SENS_FAN1].flags &= ~SENSOR_FINVALID;

	iic_release_bus(sc->sc_tag, 0);
}
