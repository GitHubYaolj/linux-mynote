/*
 * fs/sysfs/symlink.c - operations for initializing and mounting sysfs
 *
 * Copyright (c) 2001-3 Patrick Mochel
 * Copyright (c) 2007 SUSE Linux Products GmbH
 * Copyright (c) 2007 Tejun Heo <teheo@suse.de>
 *
 * This file is released under the GPLv2.
 *
 * Please see Documentation/filesystems/sysfs.txt for more information.
 */

#define DEBUG 

#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/pagemap.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/magic.h>

#include "sysfs.h"


static struct vfsmount *sysfs_mount;
struct super_block * sysfs_sb = NULL;
struct kmem_cache *sysfs_dir_cachep;

static const struct super_operations sysfs_ops = {
	.statfs		= simple_statfs,
	.drop_inode	= generic_delete_inode,
	.delete_inode	= sysfs_delete_inode,
};

struct sysfs_dirent sysfs_root = {
	.s_name		= "",
	.s_count	= ATOMIC_INIT(1),
	.s_flags	= SYSFS_DIR,
	.s_mode		= S_IFDIR | S_IRWXU | S_IRUGO | S_IXUGO,
	.s_ino		= 1,
};

static int sysfs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct inode *inode;
	struct dentry *root;

	sb->s_blocksize = PAGE_CACHE_SIZE;
	sb->s_blocksize_bits = PAGE_CACHE_SHIFT;
	sb->s_magic = SYSFS_MAGIC;
	sb->s_op = &sysfs_ops;  // 
	sb->s_time_gran = 1;
	sysfs_sb = sb;

	/* get root inode, initialize and unlock it */
	mutex_lock(&sysfs_mutex);
	inode = sysfs_get_inode(&sysfs_root); //sysfs_init_inode()
	mutex_unlock(&sysfs_mutex);
	if (!inode) {
		pr_debug("sysfs: could not get root inode\n");
		return -ENOMEM;
	}

	/* instantiate and link root dentry */
    //inode �� dentry����һһ��Ӧ�ģ�����inode����һ��dentry
	root = d_alloc_root(inode); //��/����ΰ��ġ�/��ԭ������������
	if (!root) {
		pr_debug("%s: could not get root dentry!\n",__func__);
		iput(inode);
		return -ENOMEM;
	}
	root->d_fsdata = &sysfs_root;
	sb->s_root = root;//һ������sysfs���ӵ�sb->s_root,��vsfmount->mnt_mountpoint = sb->s_root
	return 0;
}

static int sysfs_get_sb(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data, struct vfsmount *mnt)
{
	return get_sb_single(fs_type, flags, data, sysfs_fill_super, mnt);
}

static struct file_system_type sysfs_fs_type = {
	.name		= "sysfs",
	.get_sb		= sysfs_get_sb,
	.kill_sb	= kill_anon_super,
};

int __init sysfs_init(void)
{
	int err = -ENOMEM;

	sysfs_dir_cachep = kmem_cache_create("sysfs_dir_cache",
					      sizeof(struct sysfs_dirent),
					      0, 0, NULL);//�ִ�����һ�黺��
	if (!sysfs_dir_cachep)
		goto out;

	err = sysfs_inode_init();//����bdi_init()  backing_dev_info ��̨�豸��Ϣ�ṹ��
	if (err)
		goto out_err;
    //��������ļ�ϵͳ��������һ����file_systems��ȫ���ļ�ϵͳ����������
	err = register_filesystem(&sysfs_fs_type);//sruct file_system_type �ļ�ϵͳ����ṹ��
	if (!err) {
		sysfs_mount = kern_mount(&sysfs_fs_type);// vfs_kern_mount() ����һ���ڴ�Ȼ���ʼ�� 
		//kern_mount()��Ҫ��ɹ��ص㡢�����顢��Ŀ¼�������ڵ�Ĵ����ͳ�ʼ�����������Կ�����һ��ԭ�Ӳ���
		if (IS_ERR(sysfs_mount)) {
			printk(KERN_ERR "sysfs: could not mount!\n");
			err = PTR_ERR(sysfs_mount);
			sysfs_mount = NULL;
			unregister_filesystem(&sysfs_fs_type);
			goto out_err;
		}
	} else
		goto out_err;
out:
	return err;
out_err:
	kmem_cache_destroy(sysfs_dir_cachep);
	sysfs_dir_cachep = NULL;
	goto out;
}

#undef sysfs_get
struct sysfs_dirent *sysfs_get(struct sysfs_dirent *sd)
{
	return __sysfs_get(sd);
}
EXPORT_SYMBOL_GPL(sysfs_get);

#undef sysfs_put
void sysfs_put(struct sysfs_dirent *sd)
{
	__sysfs_put(sd);
}
EXPORT_SYMBOL_GPL(sysfs_put);
