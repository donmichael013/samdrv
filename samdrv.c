#include <linux/init.h>
#include <linux/module.h>

#define SAMCON_MAJOR			153

static struct class *samcon_class;
static struct spi_device *samcon_device;

struct spi_board_info samcon_device_info = {
		.modalias = "jethrosam",
		.max_speed_hz = 10000000,
		.bus_num = 3,
		.chip_select = 1,
		.mode = 0,
};
static const struct file_operations samcon_fops = {
	.owner =	THIS_MODULE,
	/* REVISIT switch to aio primitives, so that userspace
	 * gets more complete API coverage.  It'll simplify things
	 * too, except for the locking.
	 */
	.write =	spidev_write,
	.read =		spidev_read,
	.unlocked_ioctl = spidev_ioctl,
	.compat_ioctl = spidev_compat_ioctl,
	.open =		spidev_open,
	.release =	spidev_release,
	.llseek =	no_llseek,
};
static struct spi_driver samcon_driver = {
	.driver = {
		.name =		"jethrosam",
	},

static int __init samdev_init(void){

	int status;
	struct spidev_data	*samcondev;
	unsigned long		minor;
	struct spi_master *master;

	status = register_chrdev(SAMCON_MAJOR, "spi", &samcon_fops);

	if (status < 0)
		return status;
	
	samcon_class = class_create(THIS_MODULE, "jethrosam");

	if (IS_ERR(samcon_class)) {
		unregister_chrdev(SAMCON_MAJOR, samcon_driver.driver.name);
		return PTR_ERR(samcon_class);
	}

	status = spi_register_driver(&samcon_driver);

	if (status < 0) {
		class_destroy(samcon_class);
		unregister_chrdev(SAMCON_MAJOR, samcon_driver.driver.name);
		return status;
	}

	master = spi_busnum_to_master( samcon_device_info.bus_num );
	if( !master )
		return -ENODEV;

	samcon_device = spi_new_device( master, &samcon_device_info );
	if( !samcon_device )
		return -ENODEV;

	samcon_dev = kzalloc(sizeof(*samcon_dev), GFP_KERNEL);
	if (!samcon_dev)
		return -ENOMEM;

	samcon_dev->spi = samcon_device;
	spin_lock_init(&samcon_dev->spi_lock);
	mutex_init(&samcon_dev->buf_lock);





	minor = find_first_zero_bit(minors, N_SPI_MINORS);
	if (minor < N_SPI_MINORS) {
		struct device *dev;

		spidev->devt = MKDEV(SPIDEV_MAJOR, minor);
		dev = device_create(spidev_class, &spi->dev, spidev->devt,
				    spidev, "spidev%d.%d",
				    spi->master->bus_num, spi->chip_select);
		status = PTR_ERR_OR_ZERO(dev);
	} else {
		dev_dbg(&spi->dev, "no minor number available!\n");
		status = -ENODEV;
	}
	if (status == 0) {
		set_bit(minor, minors);
		list_add(&spidev->device_entry, &device_list);
		printk(KERN_INFO "device_create completed successfully\n");
	}
	mutex_unlock(&device_list_lock);

	spidev->speed_hz = spi->max_speed_hz;

	if (status == 0) {
		spi_set_drvdata(spi, spidev);
		printk(KERN_INFO "spi_set_drvdata completed\n");
	}
	else{
		kfree(spidev);
	}

	status = spi_setup( spi );

	if( status ){
		printk(KERN_INFO "spi_setup failed\n");
		spi_unregister_device( spi );
	}
	else
		printk( KERN_INFO "SamCon registered to SPI bus %u, chipselect %u\n",
			spi_device_info.bus_num, spi_device_info.chip_select );

	/*End probe*/

	return status;
}
module_init(samdev_init);

static void __exit samdev_exit(void){

}
module_exit(samdev_exit);

MODULE_AUTHOR("Don Michael <donmichael013@gmail.com>");
MODULE_DESCRIPTION("Interface for jethro sam controller");
MODULE_LICENSE("GPL");