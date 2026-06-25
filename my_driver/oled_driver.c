#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

#include "oled.h"
#include "font.h"

#define DEV_MAJOR	0
#define DEV_NAME	"oled"
#define NUM_KEYS	4
#define I2C3_BUS_NUM	2	/* imx6ull.dtsi alias: i2c2 = &i2c3 */

/* oled.c 导出的调试变量, 用于诊断 I2C 错误 */
extern int oled_i2c_first_error;
extern uint8_t oled_i2c_first_err_cmd;
extern uint8_t oled_i2c_first_err_data;

/* ─── 驱动私有数据 ──────────────────────────────────────────── */
struct oled_priv {
	struct i2c_adapter *i2c_adap;
	struct work_struct display_work;	/* 中断下半部: 延迟更新 OLED */
};

static int major;
static struct class *class_oled;
/* oled_i2c_client 在 oled.c 中定义, oled.h 声明为 extern, remove 中直接使用 */

bool isInitialized = false;

/* ─── work: 在进程上下文中更新 OLED 显示 ──────────────────────── */
static void oled_display(struct oled_priv *priv)
{
	
}

/* ─── work 回调: 进程上下文中执行 OLED 刷新 ───────────────────── */
static void display_work_func(struct work_struct *ws)
{
	//ws本质是oled_priv结构体中display_work成员的地址。这个宏的本质是利用display_work成员在oled_priv结构体中的偏移量，计算出oled_priv结构体的起始地址，从而获取到整个oled_priv结构体的指针。这样就可以在work回调函数中访问oled_priv结构体中的其他成员变量了。
	//oled_priv是包含display_work的结构体，container_of宏根据display_work的地址计算出oled_priv的地址
	//display_work是当前工作队列的句柄
	struct oled_priv *priv =
		container_of(ws, struct oled_priv, display_work);

	oled_display(priv);
}

/* ─── file_operations ────────────────────────────────────────── */
static int key_oled_open(struct inode *inode, struct file *filp)
{
	if (isInitialized)
		oled_clear();
	return 0;
}

static ssize_t key_oled_read(struct file *filp, char __user *buf,
			     size_t cnt, loff_t *offt)
{
	return 0;
}

static ssize_t key_oled_write(struct file *filp, const char __user *buf,
			      size_t cnt, loff_t *offt)
{
	char *kbuf, *p, *seg;
	char *comma1, *comma2;
	int x, y;

	if (cnt == 0)
		return 0;

	kbuf = kzalloc(cnt + 1, GFP_KERNEL);
	if (!kbuf)
		return -ENOMEM;

	if (copy_from_user(kbuf, buf, cnt)) {
		kfree(kbuf);
		return -EFAULT;
	}
	kbuf[cnt] = '\0';

	/* 去掉末尾的换行符 */
	while (cnt > 0 && (kbuf[cnt - 1] == '\n' || kbuf[cnt - 1] == '\r'))
		kbuf[--cnt] = '\0';

	/* 按 ';' 分割, 逐段解析 "X,Y,string" */
	p = kbuf;
	while ((seg = strsep(&p, ";")) != NULL) {
		if (*seg == '\0')
			continue;

		comma1 = strchr(seg, ',');
		if (!comma1)
			continue;
		*comma1 = '\0';

		comma2 = strchr(comma1 + 1, ',');
		if (!comma2) {
			*comma1 = ',';
			continue;
		}
		*comma2 = '\0';

		if (kstrtoint(seg, 10, &x) != 0 ||
		    kstrtoint(comma1 + 1, 10, &y) != 0) {
			*comma1 = ',';
			*comma2 = ',';
			continue;
		}

		oled_show_string((uint16_t)x, (uint16_t)y, comma2 + 1);

		*comma1 = ',';
		*comma2 = ',';
	}

	kfree(kbuf);
	return cnt;
}

static int key_oled_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static struct file_operations oled_fops = {
	.owner   = THIS_MODULE,
	.open    = key_oled_open,
	.read    = key_oled_read,
	.write   = key_oled_write,
	.release = key_oled_release,
};

/* ─── probe ──────────────────────────────────────────────────── */
static int key_oled_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct oled_priv *priv;
	struct i2c_board_info info;
	struct i2c_client *client;

	dev_info(dev, "oled_probe\n");

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) return -ENOMEM;
	platform_set_drvdata(pdev, priv);

	/* 初始化 work, 确保注册中断前 work 已就绪 */
	//这一步主要是初始化工作队列，中断函数不能使用包含可能睡眠的函数，必须使用工作队列确保非阻塞
	//display_work_func是工作队列回调，当中断发生时，schedule_work(&priv->display_work)会将display_work_func函数添加到系统的工作队列中，等待内核调度器在适当的时间执行它
	INIT_WORK(&priv->display_work, display_work_func);

	/* 获取 I2C3 适配器 */
	priv->i2c_adap = i2c_get_adapter(I2C3_BUS_NUM);
	if (!priv->i2c_adap) {
		dev_err(dev, "i2c_get_adapter(%d) failed, imx-fire-i2c3.dtbo loaded?\n",
			I2C3_BUS_NUM);
		return -EPROBE_DEFER;
	}

	/* 注册 I2C 客户端 (标准 Linux 做法, type 不与任何内核驱动匹配) */
	memset(&info, 0, sizeof(info));
	strlcpy(info.type, "my_ssd1306", I2C_NAME_SIZE);
	info.addr = OLED_I2C_ADDR_7BIT;
	client = i2c_new_device(priv->i2c_adap, &info);
	if (!client) {
		dev_err(dev, "i2c_new_device failed\n");
		i2c_put_adapter(priv->i2c_adap);
		return -ENODEV;
	}
	oled_i2c_client = client;

	/* 初始化 OLED 并显示启动信息 */
	dev_info(dev, "I2C adapter nr=%d name=%s\n",
		 priv->i2c_adap->nr, priv->i2c_adap->name);
	oled_init();
	dev_info(dev, "oled_init done, i2c_err=%d cmd=0x%02x data=0x%02x\n",
		 oled_i2c_first_error, oled_i2c_first_err_cmd, oled_i2c_first_err_data);
	oled_clear();
	oled_show_string(0, 0, "KEY-OLED Ready");
	oled_show_string(0, 2, "K1 K2 K3 K4");
	oled_show_string(0, 4, "Press any key~");
	isInitialized = true;

	/* 注册字符设备 */
	major = register_chrdev(DEV_MAJOR, DEV_NAME, &oled_fops);
	dev_info(dev, "key_oled major=%d\n", major);

	class_oled = class_create(THIS_MODULE, "oled_class");
	device_create(class_oled, NULL, MKDEV(major, 0), NULL, "oled");

	dev_info(dev, "probe done, /dev/oled ready\n");
	return 0;
}

/* ─── remove ─────────────────────────────────────────────────── */
static int key_oled_remove(struct platform_device *pdev)
{
	struct oled_priv *priv = platform_get_drvdata(pdev);

	/* 取消并等待 work 完成, 防止 work 访问已释放的 I2C 资源 */
	cancel_work_sync(&priv->display_work);

	oled_clear();

	device_destroy(class_oled, MKDEV(major, 0));
	class_destroy(class_oled);
	unregister_chrdev(major, DEV_NAME);

	/* 注销 I2C 客户端 + 释放 adapter */
	if (oled_i2c_client) {
		i2c_unregister_device(oled_i2c_client);
		oled_i2c_client = NULL;
	}
	if (priv && priv->i2c_adap) {
		i2c_put_adapter(priv->i2c_adap);
		priv->i2c_adap = NULL;
	}

	dev_info(&pdev->dev, "key_oled_remove done\n");
	return 0;
}

/* ─── of_match_table ─────────────────────────────────────────── */
static const struct of_device_id oled_of_match[] = {
	{ .compatible = "ustc-embed,oled" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, oled_of_match);

/* ─── platform_driver ────────────────────────────────────────── */
static struct platform_driver oled_driver = {
	.driver = {
		.name           = "oled",
		.of_match_table = oled_of_match,
	},
	.probe  = key_oled_probe,
	.remove = key_oled_remove,
};

module_platform_driver(oled_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SSD1306 OLED driver");
