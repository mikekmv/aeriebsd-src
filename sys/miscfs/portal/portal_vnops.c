/*
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software donated to Berkeley by
 * Jan-Simon Pendry.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	from: Id: portal_vnops.c,v 1.4 1992/05/30 10:05:24 jsp Exp
 *	@(#)portal_vnops.c	8.8 (Berkeley) 1/21/94
 */

/*
 * Portal Filesystem
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/filedesc.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/malloc.h>
#include <sys/namei.h>
#include <sys/mbuf.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/un.h>
#include <sys/unpcb.h>
#include <sys/syscallargs.h>
#include <miscfs/portal/portal.h>

static int portal_fileid = PORTAL_ROOTFILEID+1;

static void	portal_closefd(struct proc *, int);
static int	portal_connect(struct socket *, struct socket *);


int	portal_badop(void *);

int	portal_lookup(void *);
int	portal_open(void *);
int	portal_getattr(void *);
int	portal_setattr(void *);
int	portal_link(void *);
int	portal_symlink(void *);
int	portal_readdir(void *);
int	portal_inactive(void *);
int	portal_reclaim(void *);
int	portal_print(void *);
int	portal_pathconf(void *);
int	portal_poll(void *);

int (**portal_vnodeop_p)(void *);
struct vnodeopv_entry_desc portal_vnodeop_entries[] = {
	{ &vop_default_desc, eopnotsupp },
	{ &vop_lookup_desc, portal_lookup },		/* lookup */
	{ &vop_create_desc, eopnotsupp },		/* create */
	{ &vop_mknod_desc, eopnotsupp },		/* mknod */
	{ &vop_open_desc, portal_open },		/* open */
	{ &vop_close_desc, nullop },			/* close */
	{ &vop_access_desc, nullop },			/* access */
	{ &vop_getattr_desc, portal_getattr },		/* getattr */
	{ &vop_setattr_desc, portal_setattr },		/* setattr */
	{ &vop_read_desc, eopnotsupp },			/* read */
	{ &vop_write_desc, eopnotsupp },		/* write */
	{ &vop_ioctl_desc, (int (*)(void *))enoioctl },	/* ioctl */
	{ &vop_poll_desc, portal_poll },		/* poll */
	{ &vop_revoke_desc, vop_generic_revoke },	/* revoke */
	{ &vop_fsync_desc, nullop },			/* fsync */
	{ &vop_remove_desc, eopnotsupp },		/* remove */
	{ &vop_link_desc, portal_link },		/* link */
	{ &vop_rename_desc, eopnotsupp },		/* rename */
	{ &vop_mkdir_desc, eopnotsupp },		/* mkdir */
	{ &vop_rmdir_desc, eopnotsupp },		/* rmdir */
	{ &vop_symlink_desc, portal_symlink },		/* symlink */
	{ &vop_readdir_desc, portal_readdir },		/* readdir */
	{ &vop_abortop_desc, vop_generic_abortop },	/* abortop */
	{ &vop_inactive_desc, portal_inactive },	/* inactive */
	{ &vop_reclaim_desc, portal_reclaim },		/* reclaim */
	{ &vop_lock_desc, vop_generic_lock },		/* lock */
	{ &vop_unlock_desc, vop_generic_unlock },	/* unlock */
	{ &vop_bmap_desc, portal_badop },		/* bmap */
	{ &vop_strategy_desc, portal_badop },		/* strategy */
	{ &vop_print_desc, portal_print },		/* print */
	{ &vop_islocked_desc, vop_generic_islocked },	/* islocked */
	{ &vop_pathconf_desc, portal_pathconf },	/* pathconf */
	{ &vop_advlock_desc, eopnotsupp },		/* advlock */
	{ &vop_bwrite_desc, eopnotsupp },		/* bwrite */
	{ NULL, NULL }
};
struct vnodeopv_desc portal_vnodeop_opv_desc =
	{ &portal_vnodeop_p, portal_vnodeop_entries };

static void
portal_closefd(struct proc *p, int fd)
{
	struct sys_close_args /* {
		syscallarg(int) fd;
	} */ ua;
	register_t retval[2];
	int error;

	SCARG(&ua, fd) = fd;
	error = sys_close(p, &ua, retval);
	/*
	 * We should never get an error, and there isn't anything
	 * we could do if we got one, so just print a message.
	 */
	if (error)
		printf("portal_closefd: error = %d\n", error);
}

/*
 * vp is the current namei directory
 * cnp is the name to locate in that directory...
 */
int
portal_lookup(void *v)
{
	struct vop_lookup_args *ap = v;
	struct componentname *cnp = ap->a_cnp;
	struct vnode **vpp = ap->a_vpp;
	struct vnode *dvp = ap->a_dvp;
	char *pname = cnp->cn_nameptr;
	struct proc *p = cnp->cn_proc;
	struct portalnode *pt;
	int error;
	struct vnode *fvp = 0;
	char *path;
	int size;

	*vpp = NULLVP;

	if (cnp->cn_nameiop == DELETE || cnp->cn_nameiop == RENAME)
		return (EROFS);

	if (cnp->cn_namelen == 1 && *pname == '.') {
		*vpp = dvp;
		VREF(dvp);
		return (0);
	}

	error = getnewvnode(VT_PORTAL, dvp->v_mount, portal_vnodeop_p, &fvp);
	if (error)
		goto bad;
	fvp->v_type = VREG;
	fvp->v_data = malloc(sizeof(struct portalnode), M_TEMP, M_WAITOK);

	pt = VTOPORTAL(fvp);
	/*
	 * Save all of the remaining pathname and
	 * advance the namei next pointer to the end
	 * of the string.
	 */
	for (size = 0, path = pname; *path; path++)
		size++;
	cnp->cn_consume = size - cnp->cn_namelen;

	pt->pt_arg = malloc(size+1, M_TEMP, M_WAITOK);
	pt->pt_size = size+1;
	bcopy(pname, pt->pt_arg, pt->pt_size);
	pt->pt_fileid = portal_fileid++;

	*vpp = fvp;
	VOP_LOCK(fvp, LK_EXCLUSIVE, p);
	/*
	 * As we are the last component of the path name, fix up
	 * the locking on the directory node.
	 */
	if ((cnp->cn_flags & LOCKPARENT) == 0) {
		VOP_UNLOCK(dvp, 0, p);
		cnp->cn_flags |= PDIRUNLOCK;
	}
	return (0);

bad:;
	if (fvp)
		vrele(fvp);
	return (error);
}

static int
portal_connect(struct socket *so, struct socket *so2)
{
	/* from unp_connect, bypassing the namei stuff... */
	struct socket *so3;
	struct unpcb *unp2;
	struct unpcb *unp3;

	if (so2 == 0)
		return (ECONNREFUSED);

	if (so->so_type != so2->so_type)
		return (EPROTOTYPE);

	if ((so2->so_options & SO_ACCEPTCONN) == 0)
		return (ECONNREFUSED);

	if ((so3 = sonewconn(so2, 0)) == 0)
		return (ECONNREFUSED);

	unp2 = sotounpcb(so2);
	unp3 = sotounpcb(so3);
	if (unp2->unp_addr)
		unp3->unp_addr = m_copy(unp2->unp_addr, 0, (int)M_COPYALL);

	so2 = so3;


	return (unp_connect2(so, so2));
}

int
portal_open(void *v)
{
	struct vop_open_args *ap = v;
	struct socket *so = 0;
	struct portalnode *pt;
	struct proc *p = ap->a_p;
	struct vnode *vp = ap->a_vp;
	int s;
	struct uio auio;
	struct iovec aiov[2];
	int res;
	struct mbuf *cm = 0;
	struct cmsghdr *cmsg;
	int newfds;
	int *ip;
	int fd;
	int error;
	int len;
	struct portalmount *fmp;
	struct file *fp;
	struct portal_cred pcred;

	/*
	 * Nothing to do when opening the root node.
	 */
	if (vp->v_flag & VROOT)
		return (0);

	/*
	 * Can't be opened unless the caller is set up
	 * to deal with the side effects.  Check for this
	 * by testing whether the p_dupfd has been set.
	 */
	if (p->p_dupfd >= 0)
		return (ENODEV);

	pt = VTOPORTAL(vp);
	fmp = VFSTOPORTAL(vp->v_mount);

	/*
	 * Create a new socket.
	 */
	error = socreate(AF_UNIX, &so, SOCK_STREAM, 0);
	if (error)
		goto bad;

	/*
	 * Reserve some buffer space
	 */
	res = pt->pt_size + sizeof(pcred) + 512;	/* XXX */
	error = soreserve(so, res, res);
	if (error)
		goto bad;

	/*
	 * Kick off connection
	 */
	s = splsoftnet();
	error = portal_connect(so, (struct socket *)fmp->pm_server->f_data);
	splx(s);
	if (error)
		goto bad;

	/*
	 * Wait for connection to complete
	 */
	/*
	 * XXX: Since the mount point is holding a reference on the
	 * underlying server socket, it is not easy to find out whether
	 * the server process is still running.  To handle this problem
	 * we loop waiting for the new socket to be connected (something
	 * which will only happen if the server is still running) or for
	 * the reference count on the server socket to drop to 1, which
	 * will happen if the server dies.  Sleep for 5 second intervals
	 * and keep polling the reference count.   XXX.
	 */
	s = splsoftnet();
	while ((so->so_state & SS_ISCONNECTING) && so->so_error == 0) {
		if (fmp->pm_server->f_count == 1) {
			error = ECONNREFUSED;
			splx(s);
			goto bad;
		}
		(void) tsleep(&so->so_timeo, PSOCK, "portalcon", 5 * hz);
	}
	splx(s);

	if (so->so_error) {
		error = so->so_error;
		goto bad;
	}
		
	/*
	 * Set miscellaneous flags
	 */
	so->so_rcv.sb_timeo = 0;
	so->so_snd.sb_timeo = 0;
	so->so_rcv.sb_flags |= SB_NOINTR;
	so->so_snd.sb_flags |= SB_NOINTR;


	pcred.pcr_flag = ap->a_mode;
	pcred.pcr_uid = ap->a_cred->cr_uid;
	pcred.pcr_gid = ap->a_cred->cr_gid;
	pcred.pcr_ngroups = ap->a_cred->cr_ngroups;
	bcopy(ap->a_cred->cr_groups, pcred.pcr_groups, NGROUPS * sizeof(gid_t));
	aiov[0].iov_base = &pcred;
	aiov[0].iov_len = sizeof(pcred);
	aiov[1].iov_base = pt->pt_arg;
	aiov[1].iov_len = pt->pt_size;
	auio.uio_iov = aiov;
	auio.uio_iovcnt = 2;
	auio.uio_rw = UIO_WRITE;
	auio.uio_segflg = UIO_SYSSPACE;
	auio.uio_procp = p;
	auio.uio_offset = 0;
	auio.uio_resid = aiov[0].iov_len + aiov[1].iov_len;

	error = sosend(so, (struct mbuf *) 0, &auio,
			(struct mbuf *) 0, (struct mbuf *) 0, 0);
	if (error)
		goto bad;

	len = auio.uio_resid = sizeof(int);
	do {
		struct mbuf *m = 0;
		int flags = MSG_WAITALL;
		fdpunlock(p->p_fd);
		error = soreceive(so, (struct mbuf **) 0, &auio,
					&m, &cm, &flags);
		fdplock(p->p_fd);
		if (error)
			goto bad;

		/*
		 * Grab an error code from the mbuf.
		 */
		if (m) {
			m = m_pullup(m, sizeof(int));	/* Needed? */
			if (m) {
				error = *(mtod(m, int *));
				m_freem(m);
			} else {
				error = EINVAL;
			}
		} else {
			if (cm == 0) {
				error = ECONNRESET;	 /* XXX */
#ifdef notdef
				break;
#endif
			}
		}
	} while (cm == 0 && auio.uio_resid == len && !error);

	if (cm == 0)
		goto bad;

	if (auio.uio_resid) {
		error = 0;
#ifdef notdef
		error = EMSGSIZE;
		goto bad;
#endif
	}

	/*
	 * XXX: Break apart the control message, and retrieve the
	 * received file descriptor.  Note that more than one descriptor
	 * may have been received, or that the rights chain may have more
	 * than a single mbuf in it.  What to do?
	 */
	cmsg = mtod(cm, struct cmsghdr *);
	newfds = (cmsg->cmsg_len - sizeof(*cmsg)) / sizeof (int);
	if (newfds == 0) {
		error = ECONNREFUSED;
		goto bad;
	}
	/*
	 * At this point the rights message consists of a control message
	 * header, followed by a data region containing a vector of
	 * integer file descriptors.  The fds were allocated by the action
	 * of receiving the control message.
	 */
	ip = (int *)(cmsg + 1);
	fd = *ip++;
	if (newfds > 1) {
		/*
		 * Close extra fds.
		 */
		int i;
		printf("portal_open: %d extra fds\n", newfds - 1);
		for (i = 1; i < newfds; i++) {
			portal_closefd(p, *ip);
			ip++;
		}
	}

	/*
	 * Check that the mode the file is being opened for is a subset 
	 * of the mode of the existing descriptor.
	 */
	if ((fp = fd_getfile(p->p_fd, fd)) == NULL) {
		error = EBADF;
		goto bad;
	}
	if (((ap->a_mode & (FREAD|FWRITE)) | fp->f_flag) != fp->f_flag) {
		portal_closefd(p, fd);
		error = EACCES;
		goto bad;
	}

	/*
	 * Save the dup fd in the proc structure then return the
	 * special error code (ENXIO) which causes magic things to
	 * happen in vn_open.  The whole concept is, well, hmmm.
	 */
	p->p_dupfd = fd;
	error = ENXIO;

bad:;
	/*
	 * And discard the control message.
	 */
	if (cm) { 
		m_freem(cm);
	}

	if (so) {
		soshutdown(so, 2);
		soclose(so);
	}
	return (error);
}

int
portal_getattr(void *v)
{
	struct vop_getattr_args *ap = v;
	struct vnode *vp = ap->a_vp;
	struct vattr *vap = ap->a_vap;

	bzero(vap, sizeof(*vap));
	vattr_null(vap);
	vap->va_uid = 0;
	vap->va_gid = 0;
	vap->va_fsid = vp->v_mount->mnt_stat.f_fsid.val[0];
	vap->va_size = DEV_BSIZE;
	vap->va_blocksize = DEV_BSIZE;
	getnanotime(&vap->va_atime);
	vap->va_mtime = vap->va_atime;
	vap->va_ctime = vap->va_atime;
	vap->va_gen = 0;
	vap->va_flags = 0;
	vap->va_rdev = 0;
	/* vap->va_qbytes = 0; */
	vap->va_bytes = 0;
	/* vap->va_qsize = 0; */
	if (vp->v_flag & VROOT) {
		vap->va_type = VDIR;
		vap->va_mode = S_IRUSR|S_IWUSR|S_IXUSR|
				S_IRGRP|S_IWGRP|S_IXGRP|
				S_IROTH|S_IWOTH|S_IXOTH;
		vap->va_nlink = 2;
		vap->va_fileid = 2;
	} else {
		vap->va_type = VREG;
		vap->va_mode = S_IRUSR|S_IWUSR|
				S_IRGRP|S_IWGRP|
				S_IROTH|S_IWOTH;
		vap->va_nlink = 1;
		vap->va_fileid = VTOPORTAL(vp)->pt_fileid;
	}
	return (0);
}

int
portal_setattr(void *v)
{
	struct vop_setattr_args *ap = v;

	/*
	 * Can't mess with the root vnode
	 */
	if (ap->a_vp->v_flag & VROOT)
		return (EACCES);

	if (ap->a_vap->va_flags != VNOVAL)
		return (EOPNOTSUPP);

	return (0);
}

/*
 * Fake readdir, just return empty directory.
 * It is hard to deal with '.' and '..' so don't bother.
 */
/*ARGSUSED*/
int
portal_readdir(void *v)
{
 	return (0);
}

/*ARGSUSED*/
int
portal_inactive(void *v)
{
 	struct vop_inactive_args *ap = v;

	VOP_UNLOCK(ap->a_vp, 0, ap->a_p);
	return (0);
}

int
portal_reclaim(void *v)
{
	struct vop_reclaim_args *ap = v;
	struct portalnode *pt = VTOPORTAL(ap->a_vp);

	if (pt->pt_arg) {
		free(pt->pt_arg, M_TEMP);
		pt->pt_arg = 0;
	}
	free(ap->a_vp->v_data, M_TEMP);
	ap->a_vp->v_data = 0;

	return (0);
}

/*
 * Return POSIX pathconf information applicable to special devices.
 */
int
portal_pathconf(void *v)
{
	struct vop_pathconf_args *ap = v;

	switch (ap->a_name) {
	case _PC_LINK_MAX:
		*ap->a_retval = LINK_MAX;
		return (0);
	case _PC_MAX_CANON:
		*ap->a_retval = MAX_CANON;
		return (0);
	case _PC_MAX_INPUT:
		*ap->a_retval = MAX_INPUT;
		return (0);
	case _PC_PIPE_BUF:
		*ap->a_retval = PIPE_BUF;
		return (0);
	case _PC_CHOWN_RESTRICTED:
		*ap->a_retval = 1;
		return (0);
	case _PC_VDISABLE:
		*ap->a_retval = _POSIX_VDISABLE;
		return (0);
	default:
		return (EINVAL);
	}
	/* NOTREACHED */
}

/*
 * Print out the contents of a Portal vnode.
 */
/* ARGSUSED */
int
portal_print(void *v)
{
	printf("tag VT_PORTAL, portal vnode\n");
	return (0);
}

int
portal_link(void *v) 
{
	struct vop_link_args *ap = v;
 
	VOP_ABORTOP(ap->a_dvp, ap->a_cnp);
	vput(ap->a_dvp);
	return (EROFS);
}

int
portal_symlink(void *v)
{
	struct vop_symlink_args *ap = v;
  
	VOP_ABORTOP(ap->a_dvp, ap->a_cnp);
	vput(ap->a_dvp);
	return (EROFS);
}

int
portal_badop(void *v)
{
	panic ("portal: bad op");
	return (0);
}

int
portal_poll(void *v)
{
	struct vop_poll_args *ap = v;

	return (ap->a_events & (POLLIN | POLLOUT | POLLRDNORM | POLLWRNORM));
}
