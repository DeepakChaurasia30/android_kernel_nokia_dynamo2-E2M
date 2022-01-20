/*
 *
 * FocalTech fts TouchScreen driver.
 *
 * Copyright (c) 2010-2015, Focaltech Ltd. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

 /*******************************************************************************
*
* File Name: Focaltech_ex_fun.c
*
* Author: Xu YongFeng
*
* Created: 2015-01-29
*
* Modify by mshl on 2015-10-26
*
* Abstract:
*
* Reference:
*
*******************************************************************************/

/*******************************************************************************
* 1.Included header files
*******************************************************************************/
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
//#include <mach/irqs.h>
#include <linux/irq.h>

#include <linux/syscalls.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/string.h>

#include <linux/mount.h>
#include <linux/netdevice.h>
#include <linux/proc_fs.h>
#include "focaltech_core.h"
#include "test_lib.h"
#include <fih/hwid.h> //SW8-DH-Merger-2006000-00+
#ifdef CONFIG_FIH_PROJECT_E2M
#include <linux/vmalloc.h>
#endif
/*******************************************************************************
* Private constant and macro definitions using #define
*******************************************************************************/
/*create apk debug channel*/
#define PROC_UPGRADE			0
#define PROC_READ_REGISTER		1
#define PROC_WRITE_REGISTER	2
#define PROC_AUTOCLB			4
#define PROC_UPGRADE_INFO		5
#define PROC_WRITE_DATA		6
#define PROC_READ_DATA			7
#define PROC_SET_TEST_FLAG				8
#define PROC_NAME	"ftxxxx-debug"

#define WRITE_BUF_SIZE		512
#define READ_BUF_SIZE		512
#define FTXXXX_INI_FILENAME_CONFIG_GIS "TP_Firmware_GIS"
#define FTXXXX_INI_FILENAME_CONFIG_Ofilm "TP_Firmware_Ofilm"
#define FTXXXX_INI_FILENAME_CONFIG_VZ1 "TP_Firmware_VZ1" //SW8-DH-Merge-2006000+
#define FTXXXX_INI_CONFIG_FILEPATH "/system/etc/"
#define FTXXXX_INI_CONFIG_NAME_GIS "Conf_MultipleTest_GIS.ini"
#define FTXXXX_INI_CONFIG_NAME_Ofilm "Conf_MultipleTest_Ofilm.ini"
#define FTXXXX_INI_CONFIG_NAME_VZ1 "Conf_MultipleTest_VZ1.ini" //SW8-DH-Merge-2006000+
#define FTXXXX_TEST_DATA_FILE "/data/fts_testdata.csv"
#define DEBUG
/*******************************************************************************
* Private enumerations, structures and unions using typedef
*******************************************************************************/


/*******************************************************************************
* Static variables
*******************************************************************************/
static unsigned char proc_operate_mode = PROC_UPGRADE;
static struct proc_dir_entry *fts_proc_entry;
bool ResultSelfTest = false;
/*******************************************************************************
* Global variable or extern global variabls/functions
*******************************************************************************/
#if GTP_ESD_PROTECT
int apk_debug_flag = 0;
#endif
/*******************************************************************************
* Static function prototypes
*******************************************************************************/

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
/*interface of write proc*/
/************************************************************************
*   Name: fts_debug_write
*  Brief:interface of write proc
* Input: file point, data buf, data len, no use
* Output: no
* Return: data len
***********************************************************************/
static ssize_t fts_debug_write(struct file *filp, const char __user *buff, size_t count, loff_t *ppos)
{
	unsigned char writebuf[WRITE_BUF_SIZE];
	int buflen = count;
	int writelen = 0;
	int ret = 0;

	if (copy_from_user(&writebuf, buff, buflen)) {
		dev_err(&fts_i2c_client->dev, "%s:copy from user error\n", __func__);
		return -EFAULT;
	}
	proc_operate_mode = writebuf[0];

	switch (proc_operate_mode) {
	case PROC_UPGRADE:
		{
			char upgrade_file_path[128];
			memset(upgrade_file_path, 0, sizeof(upgrade_file_path));
			sprintf(upgrade_file_path, "%s", writebuf + 1);
			upgrade_file_path[buflen-1] = '\0';
			FTS_DBG("%s\n", upgrade_file_path);
			disable_irq(fts_i2c_client->irq);
			#if GTP_ESD_PROTECT
			apk_debug_flag = 1;
			#endif

			ret = fts_ctpm_fw_upgrade_with_app_file(fts_i2c_client, upgrade_file_path);
			#if GTP_ESD_PROTECT
			apk_debug_flag = 0;
			#endif
			enable_irq(fts_i2c_client->irq);
			if (ret < 0) {
				dev_err(&fts_i2c_client->dev, "%s:upgrade failed.\n", __func__);
				return ret;
			}
		}
		break;
	case PROC_READ_REGISTER:
		writelen = 1;
		ret = fts_i2c_write(fts_i2c_client, writebuf + 1, writelen);
		if (ret < 0) {
			printk("BBox::UEC;7::2\n");
			dev_err(&fts_i2c_client->dev, "%s:write iic error\n", __func__);
			return ret;
		}
		break;
	case PROC_WRITE_REGISTER:
		writelen = 2;
		ret = fts_i2c_write(fts_i2c_client, writebuf + 1, writelen);
		if (ret < 0) {
			printk("BBox::UEC;7::2\n");
			dev_err(&fts_i2c_client->dev, "%s:write iic error\n", __func__);
			return ret;
		}
		break;
	case PROC_AUTOCLB:
		FTS_DBG("%s: autoclb\n", __func__);
		fts_ctpm_auto_clb(fts_i2c_client);
		break;
	case PROC_READ_DATA:
	case PROC_WRITE_DATA:
		writelen = count - 1;
		ret = fts_i2c_write(fts_i2c_client, writebuf + 1, writelen);
		if (ret < 0) {
			printk("BBox::UEC;7::2\n");
			dev_err(&fts_i2c_client->dev, "%s:write iic error\n", __func__);
			return ret;
		}
		break;
	default:
		break;
	}


	return count;
}

/*interface of read proc*/
/************************************************************************
*   Name: fts_debug_read
*  Brief:interface of read proc
* Input: point to the data, no use, no use, read len, no use, no use
* Output: page point to data
* Return: read char number
***********************************************************************/
static ssize_t fts_debug_read(struct file *filp, char __user *buff, size_t count, loff_t *ppos)
{
	int ret = 0;
	int num_read_chars = 0;
	int readlen = 0;
	u8 regvalue = 0x00, regaddr = 0x00;
	unsigned char buf[READ_BUF_SIZE];

	switch (proc_operate_mode) {
	case PROC_UPGRADE:
		//after calling fts_debug_write to upgrade
		regaddr = 0xA6;
		ret = fts_read_reg(fts_i2c_client, regaddr, &regvalue);
		if (ret < 0){
			printk("[focal] %s \n", __func__);
			printk("BBox::UEC;7::1\n");
			num_read_chars = sprintf(buf, "%s", "get fw version failed.\n");
		}
		else
			num_read_chars = sprintf(buf, "current fw version:0x%02x\n", regvalue);
		break;
	case PROC_READ_REGISTER:
		readlen = 1;
		ret = fts_i2c_read(fts_i2c_client, NULL, 0, buf, readlen);
		if (ret < 0) {
			printk("BBox::UEC;7::1\n");
			dev_err(&fts_i2c_client->dev, "%s:read iic error\n", __func__);
			return ret;
		}
		num_read_chars = 1;
		break;
	case PROC_READ_DATA:
		readlen = count;
		ret = fts_i2c_read(fts_i2c_client, NULL, 0, buf, readlen);
		if (ret < 0) {
			printk("BBox::UEC;7::1\n");
			dev_err(&fts_i2c_client->dev, "%s:read iic error\n", __func__);
			return ret;
		}

		num_read_chars = readlen;
		break;
	case PROC_WRITE_DATA:
		break;
	default:
		break;
	}

	if (copy_to_user(buff, buf, num_read_chars)) {
		dev_err(&fts_i2c_client->dev, "%s:copy to user error\n", __func__);
		return -EFAULT;
	}

	return num_read_chars;
}
static const struct file_operations fts_proc_fops = {
		.owner = THIS_MODULE,
		.read = fts_debug_read,
		.write = fts_debug_write,

};
#else
/*interface of write proc*/
/************************************************************************
*   Name: fts_debug_write
*  Brief:interface of write proc
* Input: file point, data buf, data len, no use
* Output: no
* Return: data len
***********************************************************************/
static int fts_debug_write(struct file *filp,
	const char __user *buff, unsigned long len, void *data)
{
	unsigned char writebuf[WRITE_BUF_SIZE];
	int buflen = len;
	int writelen = 0;
	int ret = 0;


	if (copy_from_user(&writebuf, buff, buflen)) {
		dev_err(&fts_i2c_client->dev, "%s:copy from user error\n", __func__);
		return -EFAULT;
	}
	proc_operate_mode = writebuf[0];

	switch (proc_operate_mode) {

	case PROC_UPGRADE:
		{
			char upgrade_file_path[128];
			memset(upgrade_file_path, 0, sizeof(upgrade_file_path));
			sprintf(upgrade_file_path, "%s", writebuf + 1);
			upgrade_file_path[buflen-1] = '\0';
			FTS_DBG("%s\n", upgrade_file_path);
			disable_irq(fts_i2c_client->irq);
			#if GTP_ESD_PROTECT
				apk_debug_flag = 1;
			#endif
			ret = fts_ctpm_fw_upgrade_with_app_file(fts_i2c_client, upgrade_file_path);
			#if GTP_ESD_PROTECT
				apk_debug_flag = 0;
			#endif
			enable_irq(fts_i2c_client->irq);
			if (ret < 0) {
				dev_err(&fts_i2c_client->dev, "%s:upgrade failed.\n", __func__);
				return ret;
			}
		}
		break;
	case PROC_READ_REGISTER:
		writelen = 1;
		ret = fts_i2c_write(fts_i2c_client, writebuf + 1, writelen);
		if (ret < 0) {
			printk("BBox::UEC;7::2\n");
			dev_err(&fts_i2c_client->dev, "%s:write iic error\n", __func__);
			return ret;
		}
		break;
	case PROC_WRITE_REGISTER:
		writelen = 2;
		ret = fts_i2c_write(fts_i2c_client, writebuf + 1, writelen);
		if (ret < 0) {
			printk("BBox::UEC;7::2\n");
			dev_err(&fts_i2c_client->dev, "%s:write iic error\n", __func__);
			return ret;
		}
		break;
	case PROC_AUTOCLB:
		FTS_DBG("%s: autoclb\n", __func__);
		fts_ctpm_auto_clb(fts_i2c_client);
		break;
	case PROC_READ_DATA:
	case PROC_WRITE_DATA:
		writelen = len - 1;
		ret = fts_i2c_write(fts_i2c_client, writebuf + 1, writelen);
		if (ret < 0) {
			printk("BBox::UEC;7::2\n");
			dev_err(&fts_i2c_client->dev, "%s:write iic error\n", __func__);
			return ret;
		}
		break;
	default:
		break;
	}


	return len;
}

/*interface of read proc*/
/************************************************************************
*   Name: fts_debug_read
*  Brief:interface of read proc
* Input: point to the data, no use, no use, read len, no use, no use
* Output: page point to data
* Return: read char number
***********************************************************************/
static int fts_debug_read( char *page, char **start,
	off_t off, int count, int *eof, void *data )
{
	int ret = 0;
	unsigned char buf[READ_BUF_SIZE];
	int num_read_chars = 0;
	int readlen = 0;
	u8 regvalue = 0x00, regaddr = 0x00;

	switch (proc_operate_mode) {
	case PROC_UPGRADE:
		//after calling fts_debug_write to upgrade
		regaddr = 0xA6;
		ret = fts_read_reg(fts_i2c_client, regaddr, &regvalue);
		if (ret < 0){
			printk("[focal] %s \n", __func__);
			printk("BBox::UEC;7::1\n");
			num_read_chars = sprintf(buf, "%s", "get fw version failed.\n");
		}
		else
			num_read_chars = sprintf(buf, "current fw version:0x%02x\n", regvalue);
		break;
	case PROC_READ_REGISTER:
		readlen = 1;
		ret = fts_i2c_read(fts_i2c_client, NULL, 0, buf, readlen);
		if (ret < 0) {
			printk("BBox::UEC;7::1\n");
			dev_err(&fts_i2c_client->dev, "%s:read iic error\n", __func__);
			return ret;
		}
		num_read_chars = 1;
		break;
	case PROC_READ_DATA:
		readlen = count;
		ret = fts_i2c_read(fts_i2c_client, NULL, 0, buf, readlen);
		if (ret < 0) {
			printk("BBox::UEC;7::1\n");
			dev_err(&fts_i2c_client->dev, "%s:read iic error\n", __func__);
			return ret;
		}

		num_read_chars = readlen;
		break;
	case PROC_WRITE_DATA:
		break;
	default:
		break;
	}

	memcpy(page, buf, num_read_chars);
	return num_read_chars;
}
#endif
/************************************************************************
* Name: fts_create_apk_debug_channel
* Brief:  create apk debug channel
* Input: i2c info
* Output: no
* Return: success =0
***********************************************************************/
int fts_create_apk_debug_channel(struct i2c_client * client)
{
	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
		fts_proc_entry = proc_create(PROC_NAME, 0777, NULL, &fts_proc_fops);
	#else
		fts_proc_entry = create_proc_entry(PROC_NAME, 0777, NULL);
	#endif
	if (NULL == fts_proc_entry)
	{
		dev_err(&client->dev, "Couldn't create proc entry!\n");

		return -ENOMEM;
	}
	else
	{
		dev_info(&client->dev, "Create proc entry success!\n");

		#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0))
			fts_proc_entry->write_proc = fts_debug_write;
			fts_proc_entry->read_proc = fts_debug_read;
		#endif
	}
	return 0;
}
/************************************************************************
* Name: fts_release_apk_debug_channel
* Brief:  release apk debug channel
* Input: no
* Output: no
* Return: no
***********************************************************************/
void fts_release_apk_debug_channel(void)
{

	if (fts_proc_entry)
		#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
			proc_remove(fts_proc_entry);
		#else
			remove_proc_entry(NULL, fts_proc_entry);
		#endif
}
/*
int ft6x06_write_reg(struct i2c_client *client, u8 regaddr, u8 regvalue)
{
	unsigned char buf[2] = {0};
	buf[0] = regaddr;
	buf[1] = regvalue;

	return ft6x06_i2c_Write(client, buf, sizeof(buf));
}


int ft6x06_read_reg(struct i2c_client *client, u8 regaddr, u8 *regvalue)
{
	return ft6x06_i2c_Read(client, &regaddr, 1, regvalue, 1);
}
*/
int focal_i2c_Read(unsigned char *writebuf,
		    int writelen, unsigned char *readbuf, int readlen)
{
	int ret,retry_cnt;

	if(NULL == fts_i2c_client)
	{
		printk("i2c_adapter = NULL in function:%s\n", __func__);
		return -1;
	}

	if (writelen > 0) {//write and read
		struct i2c_msg msgs[] = {
			{
			 .addr = fts_i2c_client->addr,
			 .flags = 0,
			 .len = writelen,
			 .buf = writebuf,
			 },
			{
			 .addr = fts_i2c_client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};

		for (retry_cnt = 0; retry_cnt < 3; retry_cnt++) {
		ret = i2c_transfer(fts_i2c_client->adapter, msgs, 2);
		    if (ret < 0){
		        printk("%s: i2c read erro, retry cnt %d \n", __func__,retry_cnt);
		        msleep(20);
		    }
		else
		        break;
		}
		if (ret < 0)
			printk("%s: i2c read error.\n", __func__);
	} else {//read only
		struct i2c_msg msgs[] = {
			{
			 .addr = fts_i2c_client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};

		for (retry_cnt = 0; retry_cnt < 3; retry_cnt++) {
		ret = i2c_transfer(fts_i2c_client->adapter, msgs, 1);
		    if (ret < 0){
		        printk("%s: i2c read erro, retry cnt %d \n", __func__,retry_cnt);
		        msleep(20);
		    }
		else
		        break;
		}
		if (ret < 0)
			printk("%s:i2c read error.\n", __func__);
	}
	return ret;
}

int focal_i2c_Write(unsigned char *writebuf, int writelen)
{
	int ret,retry_cnt;

	struct i2c_msg msg[] = {
		{
		 .addr = fts_i2c_client->addr,
		 .flags = 0,
		 .len = writelen,
		 .buf = writebuf,
		 },
	};

	if(NULL == fts_i2c_client)
	{
		printk("i2c_adapter = NULL in function:%s\n", __func__);
		return -1;
	}
	else if(NULL == writebuf)
	{
		printk("writebuf = NULL in function:%s\n", __func__);
		return -1;
	}
	else if(writelen<=0)
	{
		printk("writelen <= 0 in function:%s\n", __func__);
		return -1;
	}

		for (retry_cnt = 0; retry_cnt < 3; retry_cnt++) {
        	    ret = i2c_transfer(fts_i2c_client->adapter, msg, 1);//write data only
		    if (ret < 0){
		        printk("%s: i2c write erro, retry cnt %d \n", __func__,retry_cnt);
		        msleep(20);
		    }
		else
		        break;
		}
	if (ret < 0)
		printk("%s i2c write error.\n", __func__);

	return ret;
}


/************************************************************************
* Name: fts_tpfwver_show
* Brief:  show tp fw vwersion
* Input: device, device attribute, char buf
* Output: no
* Return: char number
***********************************************************************/
static ssize_t fts_tpfwver_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t num_read_chars = 0;
	u8 fwver = 0;
	mutex_lock(&fts_input_dev->mutex);
	if (fts_read_reg(fts_i2c_client, FTS_REG_FW_VER, &fwver) < 0)
	{
		mutex_unlock(&fts_input_dev->mutex);
		printk("BBox::UEC;7::5\n");
		printk( "%s \n", __func__);
		return -1;
	}

	if (fwver == 255)
		num_read_chars = snprintf(buf, 128,"get tp fw version fail!\n");
	else
	{
		num_read_chars = snprintf(buf, 128, "%02X\n", fwver);
	}

	mutex_unlock(&fts_input_dev->mutex);

	return num_read_chars;
}


static ssize_t ftsdeviceid_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t num_read_chars = 0;
	u8 deviceid = 0;
	mutex_lock(&fts_input_dev->mutex);
	if (fts_read_reg(fts_i2c_client, FTS_REG_FW_VENDOR_ID, &deviceid) < 0)
	{
		mutex_unlock(&fts_input_dev->mutex);
		return -1;
	}

	mutex_unlock(&fts_input_dev->mutex);
	if (deviceid == 0)
		num_read_chars = snprintf(buf, 128,"get tp device id fail!\n");
	else
	{
		num_read_chars = snprintf(buf, 128, "%02X\n", deviceid);
	}
	return num_read_chars;
}


//SW4-HL-TouchPanel-ImplementFirmwareUpgradeAndGetFirmwareVersion-00+{_20151116
void touch_tpfwver_read(char *fw_ver)
{
	u8 fwver = 0;
	u8 fwimver = 0;
	u8 deviceid = 0;
	fts_read_reg(fts_i2c_client, FTS_REG_FW_VENDOR_ID, &deviceid);
	pr_err("[focal] deviceid %x .\n", deviceid);

	mutex_lock(&fts_input_dev->mutex);

	if (fts_read_reg(fts_i2c_client, FTS_REG_FW_VER, &fwver) < 0)
	{
		goto fts_read_reg_fail;
	}
	/*if (fwver == 255)
	{
		snprintf(fw_ver, 128,"get tp fw version fail!\n");
	}
	else
	{
		snprintf(fw_ver, 128, "%02X\n", fwver);
	}*/
	if (!fwver)
	{
		fwver = 00;
	}
	if (fih_hwid_fetch(FIH_HWID_PRJ) == FIH_PRJ_VZ1)
	    fwimver = fts_ctpm_get_app_file_ver(fts_i2c_client, FTXXXX_INI_FILENAME_CONFIG_VZ1);
	else if(deviceid==0x3c)
	    fwimver = fts_ctpm_get_app_file_ver(fts_i2c_client, FTXXXX_INI_FILENAME_CONFIG_GIS);
	else if(deviceid==0x51)
	    fwimver = fts_ctpm_get_app_file_ver(fts_i2c_client, FTXXXX_INI_FILENAME_CONFIG_Ofilm);
	else{
	    pr_err("[focal] deviceid %x , not 0x3c 0x51, load 0x51\n", deviceid);
	    printk("BBox::UEC;7::6\n");
	    printk("[focal] deviceid %x , not 0x3c 0x51, load 0x51\n", deviceid);
	    fwimver = fts_ctpm_get_app_file_ver(fts_i2c_client, FTXXXX_INI_FILENAME_CONFIG_Ofilm);
	}
	if (!fwimver)
	{
		fwimver = 00;
	}

	snprintf(fw_ver, 128, "FocalTech-V%02X_%02X\n", fwver, fwimver);

fts_read_reg_fail:
	mutex_unlock(&fts_input_dev->mutex);
}

void touch_tpfwimver_read(char *fw_ver)
{
	u8 fwver = 0;

	u8 deviceid = 0;
	fts_read_reg(fts_i2c_client, FTS_REG_FW_VENDOR_ID, &deviceid);
		pr_err("[focal] deviceid %x .\n", deviceid);

	mutex_lock(&fts_input_dev->mutex);
	if (fih_hwid_fetch(FIH_HWID_PRJ) == FIH_PRJ_VZ1)
	    fwver = fts_ctpm_get_app_file_ver(fts_i2c_client, FTXXXX_INI_FILENAME_CONFIG_VZ1);
	else if(deviceid==0x3c)
	    fwver = fts_ctpm_get_app_file_ver(fts_i2c_client, FTXXXX_INI_FILENAME_CONFIG_GIS);
	else if(deviceid==0x51)
	    fwver = fts_ctpm_get_app_file_ver(fts_i2c_client, FTXXXX_INI_FILENAME_CONFIG_Ofilm);
	else{
	    pr_err("[focal] deviceid %x , not 0x3c 0x51, load 0x51\n", deviceid);
	    printk("BBox::UEC;7::6\n");
	    printk("[focal] deviceid %x , not 0x3c 0x51, load 0x51\n", deviceid);
	    fwver = fts_ctpm_get_app_file_ver(fts_i2c_client, FTXXXX_INI_FILENAME_CONFIG_Ofilm);
	}
	snprintf(fw_ver, 128, "%02X\n", fwver);

	mutex_unlock(&fts_input_dev->mutex);
}

void touch_vendor_read(char *buf)
{
	sprintf(buf, "%s\n","FocalTech");
}
//SW4-HL-TouchPanel-ImplementFirmwareUpgradeAndGetFirmwareVersion-00+}_20151116

/************************************************************************
* Name: fts_tpfwver_store
* Brief:  no
* Input: device, device attribute, char buf, char count
* Output: no
* Return: EPERM
***********************************************************************/
static ssize_t fts_tpfwver_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	/* place holder for future use */
	return -EPERM;
}
/************************************************************************
* Name: fts_tpdriver_version_show
* Brief:  show tp fw vwersion
* Input: device, device attribute, char buf
* Output: no
* Return: char number
***********************************************************************/
static ssize_t fts_tpdriver_version_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t num_read_chars = 0;

	mutex_lock(&fts_input_dev->mutex);

	num_read_chars = snprintf(buf, 128,"%s \n", FTS_DRIVER_INFO);

	mutex_unlock(&fts_input_dev->mutex);

	return num_read_chars;
}
/************************************************************************
* Name: fts_tpdriver_version_store
* Brief:  no
* Input: device, device attribute, char buf, char count
* Output: no
* Return: EPERM
***********************************************************************/
static ssize_t fts_tpdriver_version_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	/* place holder for future use */
	return -EPERM;
}
/************************************************************************
* Name: fts_tprwreg_show
* Brief:  no
* Input: device, device attribute, char buf
* Output: no
* Return: EPERM
***********************************************************************/
static ssize_t fts_tprwreg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	/* place holder for future use */
	return -EPERM;
}
/************************************************************************
* Name: fts_tprwreg_store
* Brief:  read/write register
* Input: device, device attribute, char buf, char count
* Output: print register value
* Return: char count
***********************************************************************/
static ssize_t fts_tprwreg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	ssize_t num_read_chars = 0;
	int retval;
	long unsigned int wmreg=0;
	u8 regaddr=0xff,regvalue=0xff;
	u8 valbuf[5]={0};

	memset(valbuf, 0, sizeof(valbuf));
	mutex_lock(&fts_input_dev->mutex);
	num_read_chars = count - 1;
	if (num_read_chars != 2)
	{
		if (num_read_chars != 4)
		{
			dev_err(dev, "please input 2 or 4 character\n");
			goto error_return;
		}
	}
	memcpy(valbuf, buf, num_read_chars);
	retval = kstrtoul(valbuf, 16, &wmreg);
	if (0 != retval)
	{
		dev_err(dev, "%s() - ERROR: Could not convert the given input to a number. The given input was: \"%s\"\n", __FUNCTION__, buf);
		goto error_return;
	}
	if (2 == num_read_chars)
	{
		/*read register*/
		regaddr = wmreg;
		printk("[focal][test](0x%02x)\n", regaddr);
		if (fts_read_reg(client, regaddr, &regvalue) < 0)
			printk("[Focal] %s : Could not read the register(0x%02x)\n", __func__, regaddr);
		else
			printk("[Focal] %s : the register(0x%02x) is 0x%02x\n", __func__, regaddr, regvalue);
	}
	else
	{
		regaddr = wmreg>>8;
		regvalue = wmreg;
		if (fts_write_reg(client, regaddr, regvalue)<0){
			printk("BBox::UEC;7::2\n");
			dev_err(dev, "[Focal] %s : Could not write the register(0x%02x)\n", __func__, regaddr);
		}
		else
			dev_dbg(dev, "[Focal] %s : Write 0x%02x into register(0x%02x) successful\n", __func__, regvalue, regaddr);
	}
	error_return:
	mutex_unlock(&fts_input_dev->mutex);

	return count;
}
/************************************************************************
* Name: fts_fwupdate_show
* Brief:  no
* Input: device, device attribute, char buf
* Output: no
* Return: EPERM
***********************************************************************/
static ssize_t fts_fwupdate_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	/* place holder for future use */
	return -EPERM;
}

/************************************************************************
* Name: fts_fwupdate_store
* Brief:  upgrade from *.i
* Input: device, device attribute, char buf, char count
* Output: no
* Return: char count
***********************************************************************/
static ssize_t fts_fwupdate_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u8 uc_host_fm_ver;
	int i_ret;
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	mutex_lock(&fts_input_dev->mutex);

	disable_irq(client->irq);
	#if GTP_ESD_PROTECT
		apk_debug_flag = 1;
	#endif

	i_ret = fts_ctpm_fw_upgrade_with_i_file(client);
	if (i_ret == 0)
	{
		msleep(300);
		uc_host_fm_ver = fts_ctpm_get_i_file_ver();
		dev_dbg(dev, "%s [FTS] upgrade to new version 0x%x\n", __func__, uc_host_fm_ver);
	}
	else
	{
		dev_err(dev, "%s ERROR:[FTS] upgrade failed ret=%d.\n", __func__, i_ret);
	}

	#if GTP_ESD_PROTECT
		apk_debug_flag = 0;
	#endif
	enable_irq(client->irq);
	mutex_unlock(&fts_input_dev->mutex);

	return count;
}
/************************************************************************
* Name: fts_fwupgradeapp_show
* Brief:  no
* Input: device, device attribute, char buf
* Output: no
* Return: EPERM
***********************************************************************/
static ssize_t fts_fwupgradeapp_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	/* place holder for future use */
	return -EPERM;
}

/************************************************************************
* Name: fts_fwupgradeapp_store
* Brief:  upgrade from app.bin
* Input: device, device attribute, char buf, char count
* Output: no
* Return: char count
***********************************************************************/
static ssize_t fts_fwupgradeapp_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	char fwname[128];
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	memset(fwname, 0, sizeof(fwname));
	sprintf(fwname, "%s", buf);
	fwname[count-1] = '\0';

	mutex_lock(&fts_input_dev->mutex);

	disable_irq(client->irq);
	#if GTP_ESD_PROTECT
				apk_debug_flag = 1;
			#endif
	fts_ctpm_fw_upgrade_with_app_file(client, fwname);
	#if GTP_ESD_PROTECT
				apk_debug_flag = 0;
			#endif
	enable_irq(client->irq);

	mutex_unlock(&fts_input_dev->mutex);
	return count;
}

static ssize_t fts_fwupgradeapp_store_fih(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	char fwname[128];
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	memset(fwname, 0, sizeof(fwname));
	sprintf(fwname, "%s", buf);
	fwname[count-1] = '\0';

	mutex_lock(&fts_input_dev->mutex);

	disable_irq(client->irq);
	#if GTP_ESD_PROTECT
				apk_debug_flag = 1;
			#endif
	fts_ctpm_fw_upgrade_with_app_file_fih(client, fwname);
	#if GTP_ESD_PROTECT
				apk_debug_flag = 0;
			#endif
	enable_irq(client->irq);

	mutex_unlock(&fts_input_dev->mutex);
	return count;
}

//SW4-HL-TouchPanel-ImplementFirmwareUpgradeAndGetFirmwareVersion-01+{_20151124
static int fw_upgrade_flag = 0;
int gForceUpgrade = 0;
void touch_fwupgrade(int input)
{
	u8 deviceid = 0;
	mutex_lock(&fts_input_dev->mutex);
	disable_irq(fts_i2c_client->irq);
	fts_read_reg(fts_i2c_client, FTS_REG_FW_VENDOR_ID, &deviceid);
		pr_err("[focal] deviceid %x .\n", deviceid);
	if (input > 1)
	{
		gForceUpgrade = 1;
	}
	else
	{
		gForceUpgrade = 0;
	}
	if (fih_hwid_fetch(FIH_HWID_PRJ) == FIH_PRJ_VZ1)
	    fw_upgrade_flag = fts_ctpm_fw_upgrade_with_app_file(fts_i2c_client, FTXXXX_INI_FILENAME_CONFIG_VZ1);
	else if(deviceid==0x3c)
	    fw_upgrade_flag = fts_ctpm_fw_upgrade_with_app_file(fts_i2c_client, FTXXXX_INI_FILENAME_CONFIG_GIS);
	else if(deviceid==0x51)
	    fw_upgrade_flag = fts_ctpm_fw_upgrade_with_app_file(fts_i2c_client, FTXXXX_INI_FILENAME_CONFIG_Ofilm);
	else{
	    pr_err("[focal] deviceid %x , not 0x3c 0x51, load 0x51\n", deviceid);
	    printk("BBox::UEC;7::6\n");
	    printk("[focal] deviceid %x , not 0x3c 0x51, load 0x51\n", deviceid);
	    fw_upgrade_flag = fts_ctpm_fw_upgrade_with_app_file(fts_i2c_client, FTXXXX_INI_FILENAME_CONFIG_Ofilm);
	}
	enable_irq(fts_i2c_client->irq);

	mutex_unlock(&fts_input_dev->mutex);
}
EXPORT_SYMBOL(gForceUpgrade);

void touch_fwupgrade_read(char *buf)
{
	sprintf(buf, "%d\n", fw_upgrade_flag);
}
//SW4-HL-TouchPanel-ImplementFirmwareUpgradeAndGetFirmwareVersion-00+}_20151124

/************************************************************************
* Name: fts_ftsgetprojectcode_show
* Brief:  no
* Input: device, device attribute, char buf
* Output: no
* Return: EPERM
***********************************************************************/
static ssize_t fts_getprojectcode_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return -EPERM;
}
/************************************************************************
* Name: fts_ftsgetprojectcode_store
* Brief:  no
* Input: device, device attribute, char buf, char count
* Output: no
* Return: EPERM
***********************************************************************/
static ssize_t fts_getprojectcode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	/* place holder for future use */
	return -EPERM;
}

// FIHTDC@Ivan - For self test - S
// Get the ini file size for prepare memory.
static int fts_GetInISize(char *config_name)
{
	struct file *pfile = NULL;
	struct inode *inode = NULL;
	//unsigned long magic;
	off_t fsize = 0;
	char filepath[128];
	memset(filepath, 0, sizeof(filepath));

	sprintf(filepath, "%s%s", FTXXXX_INI_CONFIG_FILEPATH, config_name);

	if (NULL == pfile)
		pfile = filp_open(filepath, O_RDONLY, 0);

	if (IS_ERR(pfile)) {
		pr_err("[focal] error occured while opening file %s.\n", filepath);
		return -EIO;
	}

	inode = pfile->f_dentry->d_inode;
	//magic = inode->i_sb->s_magic;
	fsize = inode->i_size;
	filp_close(pfile, NULL);

	return fsize;
}

// Read the config.ini data
static int fts_ReadInIData(char *config_name, char *config_buf)
{
	struct file *pfile = NULL;
	struct inode *inode = NULL;
	//unsigned long magic;
	off_t fsize = 0;
	char filepath[128];
	loff_t pos = 0;
	mm_segment_t old_fs;

	memset(filepath, 0, sizeof(filepath));
	sprintf(filepath, "%s%s", FTXXXX_INI_CONFIG_FILEPATH, config_name);
	if (NULL == pfile)
		pfile = filp_open(filepath, O_RDONLY, 0);
	if (IS_ERR(pfile)) {
		pr_err("[focal] error occured while opening file %s.\n", filepath);
		return -EIO;
	}

	inode = pfile->f_dentry->d_inode;
	//magic = inode->i_sb->s_magic;
	fsize = inode->i_size;
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	vfs_read(pfile, config_buf, fsize, &pos);
	filp_close(pfile, NULL);
	set_fs(old_fs);

	return 0;
}

// Save the test data into file.
static int fts_SaveTestData(char *file_name, char *data_buf, int iLen)
{
	struct file *pfile = NULL;

	char filepath[128];
	loff_t pos;
	mm_segment_t old_fs;

	memset(filepath, 0, sizeof(filepath));
	sprintf(filepath, "%s", file_name);
	if (NULL == pfile)
		pfile = filp_open(filepath, O_CREAT|O_RDWR, 0600);
	if (IS_ERR(pfile)) {
		pr_err("[focal] error occured while opening file %s.\n", filepath);
		return -EIO;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	vfs_write(pfile, data_buf, iLen, &pos);
	filp_close(pfile, NULL);
	set_fs(old_fs);

	return 0;
}

//Read and initial test parameters.
static int fts_get_testparam_from_ini(char *config_name)
{
	char *filedata = NULL;

	int inisize = fts_GetInISize(config_name);

	pr_info("[focal] inisize = %d \n ", inisize);
	if (inisize <= 0) {
		pr_err("[focal] %s ERROR:Get firmware size failed\n", __func__);
		return -EIO;
	}

	filedata = kmalloc(inisize + 1, GFP_ATOMIC);
	if (filedata == NULL){
		filedata = vmalloc(inisize + 1);
		pr_err("[focal] %s() - ERROR: kmalloc fail, do vmalloc\n", __func__);
	}
	if (fts_ReadInIData(config_name, filedata)) {
		pr_err("[focal] %s() - ERROR: request_firmware failed\n", __func__);
		kfree(filedata);
		return -EIO;
	} else {
		pr_info("[focal] fts_ReadInIData successful\n");
	}

	set_param_data(filedata);
		if (is_vmalloc_addr(filedata))
			vfree(filedata);
		else
			kfree(filedata);
	return 0;
}
// FIHTDC@Ivan - For self test - E
/************************************************************************
* Name: fts_ftsgetprojectcode_show
* Brief:  no
* Input: device, device attribute, char buf
* Output: no
* Return: EPERM
***********************************************************************/
static ssize_t fts_selftest_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t num_read_chars = 0;
	if(ResultSelfTest)
		num_read_chars = snprintf(buf, 100, "SelfTest Result: 0\n");
	else
		num_read_chars = snprintf(buf, 100, "SelfTest Result: 1\n");
	return num_read_chars;
}
/************************************************************************
* Name: fts_ftsgetprojectcode_store
* Brief:  no
* Input: device, device attribute, char buf, char count
* Output: no
* Return: EPERM
***********************************************************************/
static ssize_t fts_selftest_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	char *testdata = NULL;
	int iTestDataLen=0;
	char fwname[128];
	memset(fwname, 0, sizeof(fwname));
	sprintf(fwname, "%s", buf);
	fwname[count-1] = '\0';

	testdata = kmalloc(1024*80, GFP_ATOMIC);
	if (testdata == NULL){
		testdata = vmalloc(1024*80);
		pr_err("[focal] %s() - ERROR: kmalloc fail, do vmalloc\n", __func__);
	}
	if(NULL == testdata)
	{
		printk("[focal] kmalloc failed in function:%s\n", __func__);
		return count;
	}

	mutex_lock(&fts_input_dev->mutex);
	init_i2c_write_func(focal_i2c_Write);
	init_i2c_read_func(focal_i2c_Read);

	if( fts_get_testparam_from_ini(fwname) < 0)
	{
		printk("[focal] get testparam from ini failure\n");
	}
	else
	{
		if(start_test_tp() == true)
		{
			ResultSelfTest = true;
			printk("[focal] self test result = true\n");
		}
		else
		{
			ResultSelfTest = false;
			printk("[focal] self test result = false\n");
		}
		if (strnstr(saved_command_line, "androidboot.mode=2", strlen(saved_command_line))) {
		//Get test data
		iTestDataLen = get_test_data(testdata);
		fts_SaveTestData(FTXXXX_TEST_DATA_FILE, testdata, iTestDataLen);
		free_test_param_data();
		}
	}
		if (is_vmalloc_addr(testdata))
			vfree(testdata);
		else
			kfree(testdata);
	mutex_unlock(&fts_input_dev->mutex);
	return count;
}

//SW4-HL-TouchPanel-SelfTest-00+{_20151201
void touch_selftest(void)
{
	char *testdata = NULL;
	int iTestDataLen=0;
char *INI_CONFIG_NAME;
	u8 deviceid = 0;
	fts_read_reg(fts_i2c_client, FTS_REG_FW_VENDOR_ID, &deviceid);
		pr_err("[focal] deviceid %x .\n", deviceid);


	testdata = kmalloc(1024*80, GFP_ATOMIC);
	if (testdata == NULL){
		testdata = vmalloc(1024*80);
		pr_err("[focal] %s() - ERROR: kmalloc fail, do vmalloc\n", __func__);
	}
	if(NULL == testdata)
	{
		printk("[focal] kmalloc failed in function:%s\n", __func__);
		return;
	}

	mutex_lock(&fts_input_dev->mutex);
	init_i2c_write_func(focal_i2c_Write);
	init_i2c_read_func(focal_i2c_Read);

	if (fih_hwid_fetch(FIH_HWID_PRJ) == FIH_PRJ_VZ1)
	    INI_CONFIG_NAME = FTXXXX_INI_CONFIG_NAME_VZ1;
	else if(deviceid==0x3c)
	    INI_CONFIG_NAME = FTXXXX_INI_CONFIG_NAME_GIS;
	else if(deviceid==0x51)
	    INI_CONFIG_NAME = FTXXXX_INI_CONFIG_NAME_Ofilm;
	else{
	    pr_err("[focal] deviceid %x , not 0x3c 0x51, load 0x51\n", deviceid);
	    printk("BBox::UEC;7::6\n");
	    printk("[focal] deviceid %x , not 0x3c 0x51, load 0x51\n", deviceid);
	    INI_CONFIG_NAME = FTXXXX_INI_CONFIG_NAME_Ofilm;
	}
	if( fts_get_testparam_from_ini(INI_CONFIG_NAME) < 0)
	{
		printk("[focal] get testparam from ini failure\n");
	}
	else
	{
		if(start_test_tp() == true)
		{
			ResultSelfTest = true;
			printk("[focal] self test result = true\n");
		}
		else
		{
			ResultSelfTest = false;
			printk("[focal] self test result = false\n");
		}
		if (strnstr(saved_command_line, "androidboot.mode=2", strlen(saved_command_line))) {
		//Get test data
		    iTestDataLen = get_test_data(testdata);
		    fts_SaveTestData(FTXXXX_TEST_DATA_FILE, testdata, iTestDataLen);
		    free_test_param_data();
		}
	}

		if (is_vmalloc_addr(testdata))
			vfree(testdata);
		else
			kfree(testdata);
	mutex_unlock(&fts_input_dev->mutex);
}
//SW4-HL-TouchPanel-SelfTest-00+}_20151201

int selftest_result_read(void)
{
	int num_read_chars = 0;
	if(ResultSelfTest)
		num_read_chars = 0;
	else
		num_read_chars = 1;
	return num_read_chars;
}

/****************************************/
/* sysfs */
/*get the fw version
*example:cat ftstpfwver
*/
static DEVICE_ATTR(ftstpfwver, S_IRUGO|S_IWUSR, fts_tpfwver_show, fts_tpfwver_store);
static DEVICE_ATTR(ftsdeviceid, S_IRUGO|S_IWUSR, ftsdeviceid_show, NULL);
static DEVICE_ATTR(ftstpdriverver, S_IRUGO|S_IWUSR, fts_tpdriver_version_show, fts_tpdriver_version_store);
/*upgrade from *.i
*example: echo 1 > ftsfwupdate
*/
static DEVICE_ATTR(ftsfwupdate, S_IRUGO|S_IWUSR, fts_fwupdate_show, fts_fwupdate_store);
/*read and write register
*read example: echo 88 > ftstprwreg ---read register 0x88
*write example:echo 8807 > ftstprwreg ---write 0x07 into register 0x88
*
*note:the number of input must be 2 or 4.if it not enough,please fill in the 0.
*/
static DEVICE_ATTR(ftsfwupdate_fih, S_IRUGO|S_IWUSR, fts_fwupgradeapp_show, fts_fwupgradeapp_store_fih);
// defult TP FW path is /system/etc/firmware/

static DEVICE_ATTR(ftstprwreg, S_IRUGO|S_IWUSR, fts_tprwreg_show, fts_tprwreg_store);
/*upgrade from app.bin
*example:echo "*_app.bin" > ftsfwupgradeapp
*/
static DEVICE_ATTR(ftsfwupgradeapp, S_IRUGO|S_IWUSR, fts_fwupgradeapp_show, fts_fwupgradeapp_store);
static DEVICE_ATTR(ftsgetprojectcode, S_IRUGO|S_IWUSR, fts_getprojectcode_show, fts_getprojectcode_store);
static DEVICE_ATTR(ftsselftest, S_IRUGO|S_IWUSR, fts_selftest_show, fts_selftest_store);


/*add your attr in here*/
static struct attribute *fts_attributes[] = {
	&dev_attr_ftstpfwver.attr,
	&dev_attr_ftsdeviceid.attr,
	&dev_attr_ftstpdriverver.attr,
	&dev_attr_ftsfwupdate.attr,
	&dev_attr_ftsfwupdate_fih.attr,
	&dev_attr_ftstprwreg.attr,
	&dev_attr_ftsfwupgradeapp.attr,
	&dev_attr_ftsgetprojectcode.attr,
	&dev_attr_ftsselftest.attr,
	NULL
};

static struct attribute_group fts_attribute_group = {
	.attrs = fts_attributes
};

/************************************************************************
* Name: fts_create_sysfs
* Brief:  create sysfs for debug
* Input: i2c info
* Output: no
* Return: success =0
***********************************************************************/
int fts_create_sysfs(struct i2c_client * client)
{
	int err;

	err = sysfs_create_group(&client->dev.kobj, &fts_attribute_group);
	if (0 != err)
	{
		dev_err(&client->dev, "%s() - ERROR: sysfs_create_group() failed.\n", __func__);
		sysfs_remove_group(&client->dev.kobj, &fts_attribute_group);
		return -EIO;
	}
	else
	{
		pr_info("fts:%s() - sysfs_create_group() succeeded.\n",__func__);
	}
	return err;
}
/************************************************************************
* Name: fts_remove_sysfs
* Brief:  remove sys
* Input: i2c info
* Output: no
* Return: no
***********************************************************************/
int fts_remove_sysfs(struct i2c_client * client)
{
	sysfs_remove_group(&client->dev.kobj, &fts_attribute_group);
	return 0;
}
