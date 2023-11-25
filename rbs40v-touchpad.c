/**
 * @file   rbs40v-touchpad.c
 * @author Jeff Williams
 * @date   10 Nov 2023
 * @brief  A kernel module for controlling the I2C LED/buttons on an RBS40V. The attaches to
 * and i2c bus at an OF provided address, sets the leds and provides button events
 * skeleton provided by http://derekmolloy.ie/kernel-gpio-programming-buttons-and-leds/
*/

#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>                 // Required for the GPIO functions
#include <linux/interrupt.h>            // Required for the IRQ code
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/leds.h>

struct rbs40v_tp_dev {
	struct i2c_client *i2c;
	struct task_struct *kthread;
	struct input_dev **button_dev;
	struct led_classdev **led_dev;
};

static int mic_mute_gpio;
static int speaker_mute_gpio;

enum 

/****
1-11: volume
32: speaker mute
64: center button
128: mic mute
****/
enum {
VOL_1 = 1,
VOL_2,
VOL_3,
VOL_4,
VOL_5,
VOL_6,
VOL_7,
VOL_8,
VOL_9,
VOL_10,
VOL_11
};
#define SPKR_MUTE	0x20
#define ACTION_BTN	0x40
#define MIC_MUTE	0x80

#define NAME		"RBS40V touchpad"
#define NUM_LEDS	3
#define NUM_BUTTONS     4

#define RBS40V_REG_A    0
#define RBS40V_REG_B    1
#define RBS40V_INIT_DEV 1

static const struct of_device_id rbs40v_touchpad_id_table[];

static int rbs40v_tp_worker(void *dev_id) {
	struct rbs40v_tp_dev *tp = dev_id;

	while (!kthread_should_stop()) {
		msleep(20);
	}
	ret = i2c_smbus_read_word_data(tp->i2c, );
	switch (ret) :
	{
		case VOL_1:
		case VOL_2:
		case VOL_3:
		case VOL_4:
		case VOL_5:
		case VOL_6:
		case VOL_7:
		case VOL_8:
		case VOL_9:
		case VOL_10:
		case VOL_11:
			input_report_key(tp->button[0], BTN_0, inb(BUTTON_PORT) & 1);
			input_sync(tp->button[0];
			break;
		case SPKR_MUTE:
			input_report_key(tp->button[1], BTN_1, inb(BUTTON_PORT) & 1);
			input_sync(tp->button[1];
			break;
		case ACTION_BTN:
			input_report_key(tp->button[2], BTN_2, inb(BUTTON_PORT) & 1);
			input_sync(tp->button[2];
			break;
		case MIC_MUTE:
			input_report_key(tp->button[3], BTN_0, inb(BUTTON_PORT) & 1);
			input_sync(tp->button[3];
			break;
	}
	
	return 0;
}

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point. In this example this
 *  function sets up the GPIOs and the IRQ
 *  @return returns 0 if successful
 */
static int rbs40v_touchpad_probe(struct i2c_client *i2c,
				const struct i2c_device_id *id){
	int ret = 0;
	struct device_node *np;
	struct rbs40v_tp_dev *tp;
	const struct of_device_id *match = i2c_of_match_device(rbs40v_touchpad_id_table, i2c);
	struct task_struct *kthread;

	tp = devm_kzalloc(&i2c->dev, sizeof(struct rbs40v_tp_dev), GFP_KERNEL);
	if (!tp) {
		ret = -ENODEV;
		goto error;
	}
	i2c_set_clientdata(i2c, tp);

	np = of_node_get(i2c->dev.of_node);
	mic_mute_gpio = of_get_named_gpio(np, "mic-pin", 0);
	speaker_mute_gpio = of_get_named_gpio(np, "speaker-pin", 0);
	of_node_put(i2c->dev.of_node);

	/* set up the volume button */
	tp->button_dev[0] = devm_input_allocate_device(&i2c->dev);
	if (!tp->button_dev[0]) {
		return -ENOMEM;
	}
	tp->button_dev[0]->evbit[0] = BIT_MASK(EV_KEY);
	tp->button_dev[0]->keybit[BIT_WORD(BTN_0)] = BIT_MASK(BTN_0);
	ret = input_register_device(tp->button_dev[0]);
	if (ret) {
		goto err_free_dev;
	}

	/* set up the Mic Mute button */
	tp->button_dev[1] = devm_input_allocate_device(&i2c->dev);
	if (!tp->button_dev[1]) {
		return -ENOMEM;
	}
	tp->button_dev[1]->evbit[0] = BIT_MASK(EV_KEY);
	tp->button_dev[1]->keybit[BIT_WORD(BTN_1)] = BIT_MASK(BTN_1);
	ret = input_register_device(tp->button_dev[1]);
	if (ret) {
		goto err_free_dev;
	}
	/* set up the Notification button */
	tp->button_dev[2] = devm_input_allocate_device(&i2c->dev);
	if (!tp->button_dev[2]) {
		return -ENOMEM;
	}
	tp->button_dev[2]->evbit[0] = BIT_MASK(EV_KEY);
	tp->button_dev[2]->keybit[BIT_WORD(BTN_2)] = BIT_MASK(BTN_2);
	ret = input_register_device(tp->button_dev[2]);
	if (ret) {
		goto err_free_dev;
	}
	/* set up the volume mute button */
	tp->button_dev[3] = devm_input_allocate_device(&i2c->dev);
	if (!tp->button_dev[3]) {
		return -ENOMEM;
	}
	tp->button_dev[3]->evbit[0] = BIT_MASK(EV_KEY);
	tp->button_dev[3]->keybit[BIT_WORD(BTN_3)] = BIT_MASK(BTN_3);
	ret = input_register_device(tp->button_dev[3]);
	if (ret) {
		goto err_free_dev;
	}

	tp->led_dev = devm_kzalloc(&i2c->dev, sizeof(struct led_classdev) * NUM_BUTTONS, GFP_KERNEL);
	if (!tp->led_dev) {
		return -ENOMEM;
	}
	/* Set up volume LED, we will abuse the led brightness mechanism to pass volumes 1-11 */
	tp->led_dev[0]->name = "rbs40v_touchpad:white:volume";
	tp->led_dev[0]->max_brightness = 10;
	tp->led_dev[0]->brightness = LED_OFF;
	ret = devm_led_classdev_register(&i2c->dev, tp->led_dev[0]);
	if (ret) {
		goto err_free_dev;
	}

	/* Set up MIC mute */
	tp->led_dev[1]->name = "rbs40v_touchbad:red:micmute";
	ret = devm_led_classdev_register(&i2c->dev, tp->led_dev[1]);
	if (ret) {
		goto err_free_dev;
	}
	/* Set up vol mute */
	tp->led_dev[2]->name = "rbs40v_touchbad:red:volmute";
	ret = devm_led_classdev_register(&i2c->dev, tp->led_dev[2]);
	if (ret) {
		goto err_free_dev;
	}

	tp->kthread = kthread_run(rbs40v_tp_worker, tp, "rbs40v_tp_worker");
	if (IS_ERR(kthread)) {
		ret = PTR_ERR(kthread);
		dev_err(&i2c->dev, "Could not start worker thread: %d.\n", ret);
		return ret;
	}

	return i2c_smbus_write_word_data(i2c, RBS40V_REG_A, RBS40V_INIT_DEV);

error:
err_free_dev:
	return ret;

}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required. Used to release the
 *  GPIOs and display cleanup messages.
 */
static int rbs40v_touchpad_remove(struct i2c_client *client){

	tp = i2c_get_clientdata(i2c);
	kthread_stop(tp->kthread);

	return 0;
}

static const struct of_device_id rbs40v_touchpad_id_table[] = {
        { .compatible = "rbs40v-touchpad" },
        {},
};

MODULE_DEVICE_TABLE(of, rbs40v_touchpad_id_table);

static struct i2c_driver rbs40v_touchpad_driver = {
        .probe = rbs40v_touchpad_probe,
        .remove = rbs40v_touchpad_remove,
        .driver = {
                .name = "rbs40v_touchpad",
                .owner = THIS_MODULE,
                .of_match_table = rbs40v_touchpad_id_table,
        },
};

module_i2c_driver(rbs40v_touchpad_driver);

MODULE_ALIAS("rbs40v_touchpad_driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeff Williams");
MODULE_DESCRIPTION("A touchpad button/led driver for RBS40V");
MODULE_VERSION("0.1");
