.include <bsd.own.mk>

SUBDIR=	ac accton acpidump adduser amd apm apmd arp \
	authpf bgpctl bgpd chroot config cron dev_mkdb \
	dhcpd dhcrelay dvmrpctl dvmrpd edquota eeprom faithd fdformat \
	ftp-proxy gpioctl hostapd hotplugd icbd ifstated inetd iostat \
	kgmon kvm_mkdb lpr mailwrapper map-mbone memconfig mopd mrinfo \
	mrouted mtrace mtree ndp netgroup_mkdb ntpd openssl ospfctl ospfd \
	pcidump popa3d portmap ppp pppd pppoe procmap pstat pwd_mkdb \
	quot quotaon rarpd rbootd rdate rdconfig relayctl relayd repquota \
	rip6query ripctl ripd rmt route6d rpc.bootparamd rpc.lockd \
	rpc.statd rtadvd rtsold rwhod sa sasyncd sensorsd sliplogin \
	slstats snmpctl snmpd spamdb spray syslogc syslogd sysmerge \
	tcpdrop tcpdump timed tokenadm tokeninit traceroute traceroute6 \
	trpt usbdevs user vipw watchdogd wsconscfg wsfontload wsmoused \
	zdump zic ztsscale

.if (${AFS:L} == "yes")
#SUBDIR+=afs
.endif

.if (${YP:L} == "yes")
SUBDIR+=ypbind yppoll ypset ypserv
.endif

.include <bsd.subdir.mk>

