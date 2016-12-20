/*
 *  This is a sample driver refernce for accessing
 *  Special Function Register in realtime
 *
 *
 */
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <linux/moduleparam.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>

#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/system.h>
#include <linux/spinlock_types.h>

#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
//#include <plat/regs-gpio.h>
#include <plat/irqs.h>
#include <linux/irq.h>
#include <mach/gpio-bank.h>
#include <mach/regs-gpio.h>

#include <linux/switch.h>
#include <linux/input.h>
#include "doortalk_drivers.h"

#define DOORTALK_VERSION "v0.1.7"

#define DEV_NAME    "DoorTalk_Drivers"

#define DEBUG_LEVEL     1

#define DPRINTK(level,fmt,args...) do{ if(level < DEBUG_LEVEL)\
    printk(KERN_INFO "%s [%s ,%d]: " fmt "\n",DEV_NAME,__FUNCTION__,__LINE__,##args);\
}while(0)

#if DEBUG_LEVEL
#include <linux/device.h>
#endif

#define HW_REG(reg) *((volatile unsigned int *)(reg))

extern int s3c_gpio_cfgpin(unsigned int pin, unsigned int config);
extern int s3c_gpio_setpull(unsigned int pin, s3c_gpio_pull_t pull);
extern int set_irq_type(unsigned int irq, unsigned int type);

struct delayed_work blink;
struct delayed_work irled;
struct delayed_work sensor;
struct delayed_work door_stat;

struct delayed_work m_reset;
struct delayed_work s1_reset;
struct delayed_work s2_reset;

struct delayed_work m_int_on;
struct delayed_work s1_int_on;
struct delayed_work s2_int_on;

struct delayed_work m_detect;
struct delayed_work s1_detect;
struct delayed_work s2_detect;

static struct workqueue_struct *motion_wq;

struct work_struct motion_work;
struct work_struct sensor1_work;
struct work_struct sensor2_work;

int m_delay=250;   		// 100 => 3/sec, 	250 =>1/sec,	500=>1/3sec
int s1_delay=250;
int s2_delay=250;
int door_delay=250;

#define GPIO_DESC1           "Sensor1 IRQ"
#define GPIO_DESC2           "Sensor2 IRQ"
#define GPIO_DESC3           "Motion IRQ"

#define SENSITIVITY_MULTIPLIER 1

#define MAX_KEYCODE_CNT 8

int delay = 300;
int return_sensor;
int high_low = 0;
int led = 0;
int pattern = 1;
int motion = 0;
int sensor1 = 0;
int sensor2 = 0;

int motion_isInterrupt = 0;
int motion_sensitivity = 50;

int sensor1_isInterrupt = 0;
int sensor1_sensitivity = 50;

int sensor2_isInterrupt = 0;
int sensor2_sensitivity = 50;
int door_status=0;

DTDRIVERS_DATA dt_data;

struct timer_list test_timer;
int ir=0;
int ir_on=50;
int ir_off=50;
static void ir_timer_handler(unsigned long data)
{

	switch(ir){
		/*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*
		*																																				*
		* case 0 is off	(initial state)																									*
		* case 1 and 2 controlled "pwm"																							*
		* case 3  always on should not be use unless resistors have been changed					*
		*																																				*
		*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*/
		case 0:
			gpio_direction_output(IRLED1_GPIO, 0);
			gpio_direction_output(IRLED2_GPIO, 0);
			gpio_direction_output(IRLED3_GPIO, 0);
		    mod_timer(&test_timer, jiffies + (HZ/60)); // (jiffies + HZ) == 1sec
			//DPRINTK(0, "IR_always off\n");
			break;
		case 1:
			ir=2;
			gpio_direction_output(IRLED1_GPIO, 1);
			gpio_direction_output(IRLED2_GPIO, 1);
			gpio_direction_output(IRLED3_GPIO, 1);
		    mod_timer(&test_timer, jiffies + (HZ*ir_on/6000)); // (jiffies + HZ) == 1sec
			//DPRINTK(0, "IR_ON\n");
			break;
		case 2:
			ir=1;
			gpio_direction_output(IRLED1_GPIO, 0);
			gpio_direction_output(IRLED2_GPIO, 0);
			gpio_direction_output(IRLED3_GPIO, 0);
		    mod_timer(&test_timer, jiffies + (HZ*ir_off/6000)); // (jiffies + HZ) == 1sec
			//DPRINTK(0, "IR_OFF\n");
			break;
		case 3:
			gpio_direction_output(IRLED1_GPIO, 1);
			gpio_direction_output(IRLED2_GPIO, 1);
			gpio_direction_output(IRLED3_GPIO, 1);
		    mod_timer(&test_timer, jiffies + (HZ/60)); // (jiffies + HZ) == 1sec
			//DPRINTK(0, "IR_always ON\n");
			break;
		default:
			gpio_direction_output(IRLED1_GPIO, 0);
			gpio_direction_output(IRLED2_GPIO, 0);
			gpio_direction_output(IRLED3_GPIO, 0);
		    mod_timer(&test_timer, jiffies + (HZ/60)); // (jiffies + HZ) == 1sec
	}
}

static int ir_timer_probe(void){
    init_timer(&test_timer);
    test_timer.expires = jiffies + ( 4*HZ);
    test_timer.data = 0;
    test_timer.function = ir_timer_handler;
	DPRINTK(0, "IR_INIT_TIMER\n");
    //add_timer(&test_timer);
    mod_timer(&test_timer, jiffies + (5*HZ/ir_on)); // (jiffies + HZ) == 1sec
	return 0;
}

static struct input_dev *sensor_dev;

//new code to be tested.
int doortalk_sensor_keycode[MAX_KEYCODE_CNT]={
		KEY_0,KEY_1,KEY_2,KEY_BACKSLASH,KEY_Z,KEY_X,KEY_C,KEY_V
};

void  init_doortalk_sensor_keypress(void){
	int error, key = 0, code = 0;
	sensor_dev = input_allocate_device();
	if (!sensor_dev) {
    		printk(KERN_ERR "SENSOR:error allocating input device not enough memory\n");
    	}
    	set_bit(EV_KEY, sensor_dev->evbit);
	for(key = 0; key < MAX_KEYCODE_CNT; key++){
		code = doortalk_sensor_keycode[key];
		if(code<=0)
               		continue;
		 set_bit(code & KEY_MAX, sensor_dev->keybit);
       }
	sensor_dev->name="doortalk_sensor_key";
	error=input_register_device(sensor_dev);
	if(error){
    		printk(KERN_ERR "SENSOR:Failed to register input device\n");
	}
	return;
}

static void led_blink(struct work_struct *pWork) {
	if (led == 0) {
		int ret;
		//	DPRINTK(0, "led_blink\n");
//		DPRINTK(0, "flag = %d\n",led);
		schedule_delayed_work(&blink, delay);
		ret = gpio_get_value(LEDINDICATOR_GPIO);
		if (ret == 0) {
			gpio_direction_output(LEDINDICATOR_GPIO, 1);
		} else if (ret == 1) {
			gpio_direction_output(LEDINDICATOR_GPIO, 0);
		}
	} else if (led == 1) {
//		DPRINTK(0, "Blink Twice");
		switch (pattern) {
		case 1:
			gpio_direction_output(LEDINDICATOR_GPIO, 1);
			schedule_delayed_work(&blink, 30);
			break;
		case 2:
			gpio_direction_output(LEDINDICATOR_GPIO, 0);
			schedule_delayed_work(&blink, 60);
			break;
		case 3:
			gpio_direction_output(LEDINDICATOR_GPIO, 1);
			schedule_delayed_work(&blink, 30);
			break;
		case 4:
			gpio_direction_output(LEDINDICATOR_GPIO, 0);
			//schedule_delayed_work(&pattern, 150);

			// pattern goes back to start because pattern flag is reset
			pattern = 0;
			//pattern ends because the "loop" is canceled
			cancel_delayed_work(&blink);
			break;
		default:
			pattern = 0;
		}
		pattern++;
	} else if (led == 2) {
//		DPRINTK(0, "Blink Once");
		switch (pattern) {
		case 1:
			gpio_direction_output(LEDINDICATOR_GPIO, 1);
			schedule_delayed_work(&blink, 30);
			break;
		case 2:
			gpio_direction_output(LEDINDICATOR_GPIO, 0);
			pattern = 0;
			//pattern ends because the "loop" is canceled
			cancel_delayed_work(&blink);
			break;
		default:
			pattern = 0;
		}
		pattern++;
	} else if (led == 3) {
//		DPRINTK(0, "RECOVERY MODE");
		switch (pattern) {
		case 1:
			gpio_direction_output(LEDINDICATOR_GPIO, 1);
			schedule_delayed_work(&blink, 600);
			break;
		case 2:
			gpio_direction_output(LEDINDICATOR_GPIO, 0);
			schedule_delayed_work(&blink, 20);
			pattern = 0;
			break;
		default:
			pattern = 0;
		}
		pattern++;
	} else {
//		DPRINTK(0, "LED ON");
		switch (pattern) {
		case 1:
			gpio_direction_output(LEDINDICATOR_GPIO, 1);
			schedule_delayed_work(&blink, 5000);
			break;
		case 2:
			gpio_direction_output(LEDINDICATOR_GPIO, 0);
			pattern = 0;
			cancel_delayed_work(&blink);
			break;
		default:
			pattern = 0;
		}
		pattern++;
	}
}

static void irLED_control(struct work_struct *pWork) {
//	DPRINTK(0, "IRLed_high\n");
	gpio_direction_output(IRLED1_GPIO, high_low);
}

static void sensor_status(struct work_struct *work) {
	return_sensor = sensor1 + sensor2 + motion;
//	DPRINTK(0, "<<<%d>>>\n", return_sensor);
	schedule_delayed_work(&sensor, 100);
}


static void motion_reset(struct work_struct *pWork) {
	motion_isInterrupt = 0;
	motion = 0;
}

static void motion_int_on(struct work_struct *pWork) {
	enable_irq(MOTION_IRQ);
}

static void motion_queue(struct work_struct *work) {
//	unsigned long flags, i;
//	disable_irq(MOTION_DETECT_IRQ);
	cancel_delayed_work(&m_detect);
	input_report_key(sensor_dev, KEY_BACKSLASH, 1); // press
	input_report_key(sensor_dev, KEY_BACKSLASH, 0); // press
	DPRINTK(0, "<{[[[[[[[( motion )]]]]]]}>\n");
//	DPRINTK(0,"sent\n	");
	motion = 1;
	schedule_delayed_work(&m_detect, 500);
//	schedule_delayed_work(&m_int_on, 500);
//	schedule_delayed_work(&m_reset, m_delay);
}


static void motion_detect(struct work_struct *work){
	int ret;
	ret=gpio_get_value(MOTION_GPIO);
	if (ret==0){
		motion_isInterrupt++;
		schedule_delayed_work(&m_detect,5);
		//DPRINTK(0,"SENSOR1:DISABLED IRQ");
		//disable_irq(SENSOR1_IRQ);
	}
	else{
		cancel_delayed_work(&m_detect);	
	}	

	if (motion_isInterrupt == 1) {
		schedule_delayed_work(&m_reset, 250);
	}
	DPRINTK(0,"motion_detect\n");

	if (motion_isInterrupt*2 > motion_sensitivity) {
		cancel_delayed_work(&m_detect);
		queue_work(motion_wq, &motion_work);
		motion_isInterrupt = 0;
		cancel_delayed_work(&m_reset);
	}


}

static irqreturn_t motion_interrupt(int irq, void *dev_id, struct pt_regs *regs) {
	unsigned long flags;

	DPRINTK(0,"motion_interrupt\n");

	motion_isInterrupt++;
	local_irq_save(flags);
//* when measuring pulse width
	schedule_delayed_work(&m_detect,5);
//*/

/* when counting number of pulse
	if (motion_isInterrupt == 1) {
		schedule_delayed_work(&m_reset, m_delay);
	}

	if (motion_isInterrupt > motion_sensitivity * SENSITIVITY_MULTIPLIER) {
		queue_work(motion_wq, &motion_work);
		motion_isInterrupt = 0;
		cancel_delayed_work(&m_reset);
	}
//*/
	local_irq_restore(flags);
	return IRQ_HANDLED;
}

static void poll_doorstatus(struct work_struct *pWork) {
	int ret=0;
	ret = gpio_get_value(DOORSTAT_GPIO);
	if(door_status==ret){
		// checks for no change in status // do nothing...
	}
	else{
		//if change occurs
		if(ret==1){
			input_report_key(sensor_dev, KEY_C, 1); // press
			input_report_key(sensor_dev, KEY_C, 0); // press
		}
		if(ret==0){
			input_report_key(sensor_dev, KEY_V, 1); // press
			input_report_key(sensor_dev, KEY_V, 0); // press
		}
		door_status=ret;
	}
	schedule_delayed_work(&door_stat, door_delay);
}

int init_led(void) {
	int ret = 1;

	INIT_DELAYED_WORK(&blink, led_blink);
	if (gpio_request(LEDINDICATOR_GPIO, "LEDINDICATOR_GPIO")) {
		DPRINTK(0, "-->err %d", __LINE__);
		ret = -1;
	} else
		gpio_direction_output(LEDINDICATOR_GPIO, 1);
//		schedule_delayed_work(&blink, 100);	//1000 jiffies

	return ret;
}

int init_ir(void) {
	int ret = 1;

	INIT_DELAYED_WORK(&irled, irLED_control);

	if (gpio_request(IRLED1_GPIO, "IRLED1_GPIO")) {
		DPRINTK(0, "-->err %d", __LINE__);
		ret = 0;
	}

	if (gpio_request(IRLED2_GPIO, "IRLED2_GPIO")) {
			DPRINTK(0, "-->err %d", __LINE__);
			ret = 0;
	}

	if (gpio_request(IRLED3_GPIO, "IRLED3_GPIO")) {
				DPRINTK(0, "-->err %d", __LINE__);
				ret = 0;
	}
	ir_timer_probe();
	schedule_delayed_work(&irled, 200);	//1000 jiffies

	return ret;
}

int init_sensor(void) {
	int ret = 1;

	DPRINTK(0, "init_sensor\n");
	motion_wq = create_workqueue("motion_queue");
	INIT_WORK(&motion_work, motion_queue);

//	INIT_DELAYED_WORK(&sensor, sensor_status);
	INIT_DELAYED_WORK(&m_reset, motion_reset);
	INIT_DELAYED_WORK(&m_int_on, motion_int_on);
	INIT_DELAYED_WORK(&m_detect, motion_detect);

	if (gpio_is_valid(MOTION_GPIO)) {
		if (gpio_request(MOTION_GPIO, "MOTION_GPIO")) {
			DPRINTK(0, "-->err %d", __LINE__);
			ret = 0;
		} else
			gpio_direction_input(MOTION_GPIO);
			DPRINTK(0, "MOTION_GPIO\n");
	}
	
	s3c_gpio_cfgpin(MOTION_GPIO, S5P_GPIO_CONMASK(5));
	s3c_gpio_setpull(MOTION_GPIO, S3C_GPIO_PULL_UP);
	set_irq_type(MOTION_IRQ, IRQ_TYPE_EDGE_FALLING);

	if (request_irq(MOTION_IRQ, (irq_handler_t) motion_interrupt,IRQF_DISABLED,GPIO_DESC3, &motion_interrupt)) {
		DPRINTK(0, "IRQ ERROR");
		DPRINTK(0, "--> %d\n", __LINE__);
	}else
		DPRINTK(0, "REQUEST_IRQ\n");

	init_doortalk_sensor_keypress();
//	schedule_delayed_work(&sensor, 50);
	return ret;

}

int init_relay(void) {
	int ret = 1;

	if (gpio_request(RELAY_GPIO, "RELAY_GPIO")) {
		DPRINTK(0, "-->err %d", __LINE__);
		ret = -1;
	} else

		gpio_direction_output(RELAY_GPIO,0);

	return ret;
}

int init_doorstat(void) {
	int ret = 1;
	INIT_DELAYED_WORK(&door_stat, poll_doorstatus);
	if (gpio_request(DOORSTAT_GPIO, "DOORSTAT_GPIO")) {
		DPRINTK(0, "-->err %d", __LINE__);
		ret = -1;
	} else
		gpio_direction_input(DOORSTAT_GPIO);

	schedule_delayed_work(&door_stat, 5000);
	return ret;
}

static int doortalk_drivers_open(struct inode *inode, struct file *file) {
//	DPRINTK(0, "--> %s\n", __func__);
	return 0;
}

static int doortalk_drivers_close(struct inode *inode, struct file *file) {
//	DPRINTK(0, "--> %s\n", __func__);
	return 0;
}

/* TODO */
static int doortalk_drivers_ioctl(struct inode *inode, struct file *fle,
		unsigned int cmd, unsigned long arg) {

	int len;

	switch (cmd) {

	case ON:
		DPRINTK(0,"ON\n");
		flush_scheduled_work();
		cancel_delayed_work_sync(&blink);
		gpio_direction_output(LEDINDICATOR_GPIO, 1);
		break;

	case ON_DELAY:
		DPRINTK(0,"ON_DELAY\n");
		led = 4;
		schedule_delayed_work(&blink, 10);
		break;

	case SLOW:
		DPRINTK(0,"SLOW\n");
		led = 0;
		delay = 500;
		schedule_delayed_work(&blink, 10);
		break;

	case FAST:
		DPRINTK(0,"FAST\n");
		led = 0;
		delay = 50;
		schedule_delayed_work(&blink, 10);
		break;

	case ONCE:
		DPRINTK(0,"ONCE\n");
		led = 2;
		schedule_delayed_work(&blink, 10);
		break;

	case TWICE:
		DPRINTK(0,"TWICE\n");
		led = 1;
		schedule_delayed_work(&blink, 10);
		break;

	case RECOVERY:
		DPRINTK(0,"RECOVERY\n");
		led = 3;
		schedule_delayed_work(&blink, 10);
		break;

	case OFF:
		DPRINTK(0,"OFF\n");
		flush_scheduled_work();
		cancel_delayed_work_sync(&blink);
		gpio_direction_output(LEDINDICATOR_GPIO, 0);
		break;

	case IRLED_ON:
		high_low = 1;
		schedule_delayed_work(&irled, 100);
		break;

	case IRLED_OFF:
		high_low = 0;
		schedule_delayed_work(&irled, 100);
		break;


	case SENSOR_DISABLED:
		DPRINTK(0,"Disabling sensor device\n");
		disable_irq(MOTION_IRQ);
//		disable_irq(SENSOR1_IRQ);
//		disable_irq(SENSOR2_IRQ);
		break;

	case SENSOR_ENABLED:
		DPRINTK(0,"Enabling sensor device\n");
		enable_irq(MOTION_IRQ);
//		enable_irq(SENSOR1_IRQ);
//		enable_irq(SENSOR2_IRQ);
		break;

	case MOTION_SENSITIVITY:
		len = copy_from_user(&dt_data, (struct __user DTDRIVERS_DATA *) arg, sizeof(dt_data));
		if (len > 0) {
			return -EFAULT;
		}
		motion_sensitivity = dt_data.value;
		DPRINTK(0,"sensitivity = %d\n", motion_sensitivity);
		break;

	case SENSOR_STATUS:
		dt_data.value = return_sensor;
		len = copy_to_user((struct __user DTDRIVERS_DATA *) arg, &dt_data, sizeof(dt_data));
		if (len > 0) {
			return -EFAULT;
		}
		break;

	case ETHERNET_ON:
		input_report_key(sensor_dev, KEY_Z, 1);
		input_report_key(sensor_dev, KEY_Z, 0);
		DPRINTK(0,"\nETHERNET_ON\n\n");
		break;

	case ETHERNET_OFF:
		input_report_key(sensor_dev, KEY_X, 1);
		input_report_key(sensor_dev, KEY_X, 0);
		DPRINTK(0,"\nETHERNET_OFF\n\n");
		break;

	case DOORLOCK_OFF:
		gpio_direction_output(RELAY_GPIO,0);
		break;

	case DOORLOCK_ON:
		gpio_direction_output(RELAY_GPIO,1);
		break;

	case RELAY_STATUS:
		dt_data.value = gpio_get_value(RELAY_GPIO);
		len = copy_to_user((struct __user DTDRIVERS_DATA *) arg, &dt_data, sizeof(dt_data));
		if (len > 0) {
			return -EFAULT;
		}
		break;
		
	case IR_TIMER_SET:
		len = copy_from_user(&dt_data, (struct __user DTDRIVERS_DATA *) arg, sizeof(dt_data));
		if (len > 0) {
			DPRINTK(0,"IR_TIMER_SET FAILED\n");
			return -EFAULT;
		}
		ir_on = dt_data.value;
		if (ir_on==0){
			DPRINTK(0,"IR_ON=0,IR_OFF=100\n");			
			ir=0;
		}else if (ir_on<100){
			ir_off= 100-ir_on;
			ir=1;		
			DPRINTK(0,"IR_ON=%d,IR_OFF=%d\n",ir_on,ir_off);
		}else if(ir_on==100){
			ir =3;
			DPRINTK(0,"IR_ON=100,IR_OFF=0\n");
		}else{
			ir=0;
			DPRINTK(0,"IR_ON=0,IR_OFF=100\n");			
			return 0;
		}
//		return 1;
		break;
	}

	return 0;
}

/* file operation supported. */
static struct file_operations doortalk_drivers_fops = { .owner = THIS_MODULE,
		.open = doortalk_drivers_open, .release = doortalk_drivers_close,
		.ioctl = doortalk_drivers_ioctl };

static struct miscdevice doortalk_drivers_ctrl = { MISC_DYNAMIC_MINOR,
DEV_NAME, &doortalk_drivers_fops };

static int __init doortalk_drivers_init(void) {
	int ret;
	printk("%s called\n", __func__);

	DPRINTK(0, "DoorTalk_Drivers %s \n", DOORTALK_VERSION);

	ret = misc_register(&doortalk_drivers_ctrl);
	if (ret) {
		DPRINTK(0, "could not register %s\n", DEV_NAME);
		return -1;
	}

	ret = init_led();
	if (ret < 1) {
		DPRINTK(0, "could not init LED device");
		return -1;
	}

	ret = init_ir();
	if (ret < 1) {
		DPRINTK(0, "could not init IR device");
		return -1;
	}

	ret = init_sensor();
	if (ret < 1) {
		DPRINTK(0, "could not init SENSOR device");
		return -1;
	}

	ret = init_relay();
	if (ret < 1) {
		DPRINTK(0, "could not init SENSOR device");
		return -1;
	}

	ret = init_doorstat();
	if (ret < 1) {
		DPRINTK(0, "could not init SENSOR device");
		return -1;
	}

	DPRINTK(0, "Driver created at %s-%s\n", __DATE__, __TIME__);
	return 0;
}

static void __exit doortalk_drivers_exit(void) {

	input_unregister_device(sensor_dev);
	misc_deregister(&doortalk_drivers_ctrl);

	cancel_delayed_work_sync(&blink);
	gpio_free(LEDINDICATOR_GPIO);
//	cancel_delayed_work_sync(&sensor);
	gpio_free(IRLED1_GPIO);
	free_irq(MOTION_IRQ, &motion_interrupt);
	gpio_free(MOTION_GPIO);
	cancel_work_sync(&motion_work);
	flush_work(&motion_work);
	destroy_workqueue(motion_wq);
	cancel_delayed_work(&m_reset);
	cancel_delayed_work(&m_int_on);
	cancel_delayed_work(&door_stat);
	gpio_free(RELAY_GPIO);
	gpio_free(DOORSTAT_GPIO);
//	free_irq(SENSOR1_IRQ, &sensor1_interrupt);
//	gpio_free(SENSOR1_GPIO);
//	cancel_work_sync(&sensor1_work);
//	cancel_delayed_work(&s1_reset);
//	cancel_delayed_work(&s1_int_on);
//	flush_work(&sensor1_work);
//	destroy_workqueue(sensor1_wq);
//	free_irq(SENSOR2_IRQ, &sensor2_interrupt);
//	gpio_free(SENSOR2_GPIO);
//	cancel_work_sync(&sensor2_work);
//	cancel_delayed_work(&s2_reset);
//	cancel_delayed_work(&s2_int_on);
//	flush_work(&sensor2_work);
//	destroy_workqueue(sensor2_wq);

	DPRINTK(0, "DoorTalk_Drivers removed\n");

}

module_init(doortalk_drivers_init);
module_exit(doortalk_drivers_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ranaguinaldo");
