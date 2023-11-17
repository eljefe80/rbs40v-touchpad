/**
 * @file   rbs40v-touchpad.c
 * @author Jeff Williams
 * @date   10 Nov 2023
 * @brief  A kernel module for controlling the I2C LED/buttons on an RBS40V. The attaches to
 * and i2c bus at an OF provided address, sets the leds and provides button events
 * skeleton provided by http://derekmolloy.ie/kernel-gpio-programming-buttons-and-leds/
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>                 // Required for the GPIO functions
#include <linux/interrupt.h>            // Required for the IRQ code

struct rbs40v_touchpad_dev {
	struct i2c_client i2c;
}
static int mic_mute_gpio;
static int speaker_mute_gpio;
static struct input_dev **button_dev;
static struct led_classdev **led_dev;
#define NAME		"RBS40V touchpad"
#define NUM_LEDS	3

static int rbs40v_tp_worker(void *dev_id) {
	struct rbs40v_tp_dev *tp = dev_id;

	while (!kthread_should_stop()) {
		msleep(20);
	}
}

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point. In this example this
 *  function sets up the GPIOs and the IRQ
 *  @return returns 0 if successful
 */
static int rbs40v_touchpad_probe(struct i2c_client *i2c,
				const stcut i2c_device_id *id){
	int ret = 0;
	struct device_node *np;
	struct rbs40v_tp_dev *tp;
	const struct of_device_id *match = of_match_device(ipq40xx_cpu_dai_id_table, &pdev->dev);
	        

	tp = devm_kzalloc(i2c->dev, sizeof(struct rbs40v_tp_dev), GFP_KERNEL);
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
	button_dev[0] = devm_input_allocate_device(i2c->dev);
	if (!button_dev[0]) {
		return -ENOMEM;
	}
	button_dev[0]->evbit[0] = BIT_MASK(EV_KEY);
	button_dev[0]->keybit[BIT_WORD(BTN_01)] = BIT_MASK(BTN_0);
	error = input_register_device(button_dev[0]);
	if (error) {
		goto err_free_dev;
	}

	/* set up the Mic Mute button */
	button_dev[1] = devm_input_allocate_device(i2c->dev);
	if (!button_dev[1]) {
		return -ENOMEM;
	}
	button_dev[1]->evbit[0] = BIT_MASK(EV_KEY);
	button_dev[1]->keybit[BIT_WORD(BTN_01)] = BIT_MASK(BTN_0);
	error = input_register_device(button_dev[1]);
	if (error) {
		goto err_free_dev;
	}
	/* set up the Notification button */
	button_dev[2] = devm_input_allocate_device(i2c->dev);
	if (!button_dev[2]) {
		return -ENOMEM;
	}
	button_dev[2]->evbit[0] = BIT_MASK(EV_KEY);
	button_dev[2]->keybit[BIT_WORD(BTN_01)] = BIT_MASK(BTN_0);
	error = input_register_device(button_dev[2]);
	if (error) {
		goto err_free_dev;
	}
	/* set up the volume mute button */
	button_dev[3] = devm_input_allocate_device(i2c->dev);
	if (!button_dev[3]) {
		return -ENOMEM;
	}
	button_dev[3]->evbit[0] = BIT_MASK(EV_KEY);
	button_dev[3]->keybit[BIT_WORD(BTN_01)] = BIT_MASK(BTN_0);
	error = input_register_device(button_dev[3]);
	if (error) {
		goto err_free_dev;
	}

	led_dev = devm_kzalloc(i2c->dev, sizeof(struct ledclass_dev) * NUM_BUTTONS, GFP_KERNEL);
	if (!led_dev) {
		return -ENOMEM;
	}
	/* Set up volume LED, we will abuse the led brightness mechanism to pass volumes 1-11 */
	led_dev[0]->name = "rbs40v_touchpad:white:volume";
	led_dev->max_brightness = 10;
	led_dev->brightness = LED_OFF;
	error = devm_led_classdev_register(i2c->dev, led_dev[0]);
	if (error) {
		goto err_free_dev;
	}

	/* Set up MIC mute */
	led_dev[1]->name = "rbs40v_touchbad:red:micmute";
	error = devm_led_classdev_register(i2c->dev, led_dev[1]);
	if (error) {
		goto err_free_dev;
	}
	/* Set up MIC mute */
	led_dev[2]->name = "rbs40v_touchbad:red:volmute";
	devm_led_classdev_register(i2c->dev, led_dev);
	error = devm_led_classdev_register(i2c->dev, led_dev[2]);
	if (error) {
		goto err_free_dev;
	}

	kthread = kthread_run(rbs40v_tp_worker, tp, "rbs40v_tp_worker");
	if (IS_ERR(kthread)) {
		error = PTR_ERR(kthread);
		dev_err(&client->dev, "Could not start worker thread: %d.\n", error);
		return error;
	}

	return i2c_smbus_write_word_data(i2c, RBS40V_REG_A, RBS40V_INIT_DEV);

err_free_dev:
	return -ENOMEM;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required. Used to release the
 *  GPIOs and display cleanup messages.
 */
static void rbs40v_touchpad_remove(struct i2c_client *client){
	return 0
}

static const struct of_device_id rbs40v_touchpad_id_table[] = {
        { .compatible = "rbs40v-touchpad" },
        {},
};

MODULE_DEVICE_TABLE(of, rbs40v_touchpad_id_table);

static struct platform_driver rbs40v_touchpad_driver = {
        .probe = rbs40v_touchpad_probe,
        .remove = rbs40v_touchpad_remove,
        .driver = {
                .name = "rbs40v_touchpad",
                .owner = THIS_MODULE,
                .of_match_table = rbs40v_touchpad_id_table,
        },
};

module_platform_driver(rbs40v_touchpad_driver);

MODULE_ALIAS("rbs40v_touchpad_driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeff Williams");
MODULE_DESCRIPTION("A touchpad button/led driver for RBS40V");
MODULE_VERSION("0.1");
