/*
 * Copyright (c) 2009 Michael Shalayeff
 * Copyright (c) 2009 Pedro Martelletto
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
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/mount.h>
#include <sys/vnode.h>

#include <milfs/milfs_param.h>
#include <milfs/milfs_types.h>
#include <milfs/milfs_proto.h>
#include <milfs/milfs_extern.h>

int (**milfs_vnodeop_p)(void *v);

struct vnodeopv_entry_desc milfs_vnodeop_entries[] = {
	{ &vop_default_desc, eopnotsupp },
	{ &vop_lookup_desc, milfs_lookup },		/* lookup */
	{ &vop_create_desc, milfs_create },		/* create */
	{ &vop_mknod_desc, milfs_mknod },		/* mknod */
	{ &vop_open_desc, milfs_open },			/* open */
	{ &vop_close_desc, milfs_close },		/* close */
	{ &vop_access_desc, milfs_access },		/* access */
	{ &vop_getattr_desc, milfs_getattr },		/* getattr */
	{ &vop_setattr_desc, milfs_setattr },		/* setattr */
	{ &vop_read_desc, milfs_read },			/* read */
	{ &vop_write_desc, milfs_write },		/* write */
	{ &vop_ioctl_desc, milfs_ioctl },		/* ioctl */
	{ &vop_poll_desc, milfs_poll },			/* poll */
	{ &vop_kqfilter_desc, milfs_kqfilter },		/* kqfilter */
	{ &vop_revoke_desc, milfs_revoke },		/* revoke */
	{ &vop_fsync_desc, milfs_fsync },		/* fsync */
	{ &vop_remove_desc, milfs_remove },		/* remove */
	{ &vop_link_desc, milfs_link },			/* link */
	{ &vop_rename_desc, milfs_rename },		/* rename */
	{ &vop_mkdir_desc, milfs_mkdir },		/* mkdir */
	{ &vop_rmdir_desc, milfs_rmdir },		/* rmdir */
	{ &vop_symlink_desc, milfs_symlink },		/* symlink */
	{ &vop_readdir_desc, milfs_readdir },		/* readdir */
	{ &vop_readlink_desc, milfs_readlink },		/* readlink */
	{ &vop_abortop_desc, milfs_abortop },		/* abortop */
	{ &vop_inactive_desc, milfs_inactive },		/* inactive */
	{ &vop_reclaim_desc, milfs_reclaim },		/* reclaim */
	{ &vop_lock_desc, milfs_lock },			/* lock */
	{ &vop_unlock_desc, milfs_unlock },		/* unlock */
	{ &vop_bmap_desc, milfs_bmap },			/* bmap */
	{ &vop_strategy_desc, milfs_strategy },		/* strategy */
	{ &vop_print_desc, milfs_print },		/* print */
	{ &vop_islocked_desc, milfs_islocked },		/* islocked */
	{ &vop_pathconf_desc, milfs_pathconf },		/* pathconf */
	{ &vop_advlock_desc, milfs_advlock },		/* advlock */
	{ &vop_reallocblks_desc, milfs_reallocblks },	/* reallocblks */
	{ &vop_bwrite_desc, milfs_bwrite },		/* bwrite */
	{ NULL, NULL }
};

struct vnodeopv_desc milfs_vnodeop_opv_desc = {
	&milfs_vnodeop_p,
	milfs_vnodeop_entries
};

int milfs_lookup(void *v) {
  printf("milfs_lookup called!\n");
  return (EINVAL);
}

int milfs_create(void *v) {
  printf("milfs_create called!\n");
  return (EINVAL);
}

int milfs_mknod(void *v) {
  printf("milfs_mknod called!\n");
  return (EINVAL);
}

int milfs_open(void *v) {
  printf("milfs_open called!\n");
  return (EINVAL);
}

int milfs_close(void *v) {
  printf("milfs_close called!\n");
  return (EINVAL);
}

int milfs_access(void *v) {
  printf("milfs_access called!\n");
  return (EINVAL);
}

int milfs_getattr(void *v) {
  printf("milfs_getattr called!\n");
  return (EINVAL);
}

int milfs_setattr(void *v) {
  printf("milfs_setattr called!\n");
  return (EINVAL);
}

int milfs_read(void *v) {
  printf("milfs_read called!\n");
  return (EINVAL);
}

int milfs_write(void *v) {
  printf("milfs_write called!\n");
  return (EINVAL);
}

int milfs_ioctl(void *v) {
  printf("milfs_ioctl called!\n");
  return (EINVAL);
}

int milfs_poll(void *v) {
  printf("milfs_poll called!\n");
  return (EINVAL);
}

int milfs_kqfilter(void *v) {
  printf("milfs_kqfilter called!\n");
  return (EINVAL);
}

int milfs_revoke(void *v) {
  printf("milfs_revoke called!\n");
  return (EINVAL);
}

int milfs_fsync(void *v) {
  printf("milfs_fsync called!\n");
  return (EINVAL);
}

int milfs_remove(void *v) {
  printf("milfs_remove called!\n");
  return (EINVAL);
}

int milfs_link(void *v) {
  printf("milfs_link called!\n");
  return (EINVAL);
}

int milfs_rename(void *v) {
  printf("milfs_rename called!\n");
  return (EINVAL);
}

int milfs_mkdir(void *v) {
  printf("milfs_mkdir called!\n");
  return (EINVAL);
}

int milfs_rmdir(void *v) {
  printf("milfs_rmdir called!\n");
  return (EINVAL);
}

int milfs_symlink(void *v) {
  printf("milfs_symlink called!\n");
  return (EINVAL);
}

int milfs_readdir(void *v) {
  printf("milfs_readdir called!\n");
  return (EINVAL);
}

int milfs_readlink(void *v) {
  printf("milfs_readlink called!\n");
  return (EINVAL);
}

int milfs_abortop(void *v) {
  printf("milfs_abortop called!\n");
  return (EINVAL);
}

int milfs_inactive(void *v) {
  printf("milfs_inactive called!\n");
  return (EINVAL);
}

int milfs_reclaim(void *v) {
  printf("milfs_reclaim called!\n");
  return (EINVAL);
}

int milfs_lock(void *v) {
  printf("milfs_lock called!\n");
  return (EINVAL);
}

int milfs_unlock(void *v) {
  printf("milfs_unlock called!\n");
  return (EINVAL);
}

int milfs_bmap(void *v) {
  printf("milfs_bmap called!\n");
  return (EINVAL);
}

int milfs_strategy(void *v) {
  printf("milfs_strategy called!\n");
  return (EINVAL);
}

int milfs_print(void *v) {
  printf("milfs_print called!\n");
  return (EINVAL);
}

int milfs_islocked(void *v) {
  printf("milfs_islocked called!\n");
  return (EINVAL);
}

int milfs_pathconf(void *v) {
  printf("milfs_pathconf called!\n");
  return (EINVAL);
}

int milfs_advlock(void *v) {
  printf("milfs_advlock called!\n");
  return (EINVAL);
}

int milfs_reallocblks(void *v) {
  printf("milfs_reallocblks called!\n");
  return (EINVAL);
}

int milfs_bwrite(void *v) {
  printf("milfs_bwrite called!\n");
  return (EINVAL);
}
