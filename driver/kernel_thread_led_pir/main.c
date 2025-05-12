#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>        // gpio_request, gpio_direction_*, gpio_set/get
#include <linux/kthread.h>     // kthread_run, kthread_stop
#include <linux/delay.h>       // msleep
#include <linux/err.h>         // IS_ERR

#define GPIO_LED_PIN      17   // BCM 17 -> LED
#define GPIO_PIR_SENSOR   23   // BCM 23 -> HC-SR501 output

static struct task_struct *motion_thread;

/* 
 * Thread function:
 *   - Poll PIR sensor every 200ms
 *   - If motion detected (gpio_get_value == 1), turn LED on
 *   - Else, turn LED off
 */
static int motion_monitor_fn(void *data)
{
    while (!kthread_should_stop()) {
        int motion = gpio_get_value(GPIO_PIR_SENSOR);
        if (motion) {
            gpio_set_value(GPIO_LED_PIN, 1);
            pr_debug("PIR: motion detected, LED ON\n");
        } else {
            gpio_set_value(GPIO_LED_PIN, 0);
            pr_debug("PIR: no motion, LED OFF\n");
        }
        msleep(200);
    }
    return 0;
}

static int __init pir_led_driver_init(void)
{
    int ret;

    /* 1) Request and configure LED GPIO */
    ret = gpio_request(GPIO_LED_PIN, "led_pin");
    if (ret) {
        pr_err("Failed to request GPIO %d for LED: %d\n", GPIO_LED_PIN, ret);
        return ret;
    }
    ret = gpio_direction_output(GPIO_LED_PIN, 0);
    if (ret) {
        pr_err("Failed to set GPIO %d as output: %d\n", GPIO_LED_PIN, ret);
        goto err_free_led;
    }

    /* 2) Request and configure PIR sensor GPIO */
    ret = gpio_request(GPIO_PIR_SENSOR, "pir_sensor_pin");
    if (ret) {
        pr_err("Failed to request GPIO %d for PIR sensor: %d\n", GPIO_PIR_SENSOR, ret);
        goto err_free_led;
    }
    ret = gpio_direction_input(GPIO_PIR_SENSOR);
    if (ret) {
        pr_err("Failed to set GPIO %d as input: %d\n", GPIO_PIR_SENSOR, ret);
        goto err_free_pir;
    }

    /* 3) Start monitoring thread */
    motion_thread = kthread_run(motion_monitor_fn, NULL, "pir_motion_thread");
    if (IS_ERR(motion_thread)) {
        ret = PTR_ERR(motion_thread);
        pr_err("Failed to create monitoring thread: %d\n", ret);
        goto err_free_pir;
    }

    pr_info("PIR-LED driver initialized: LED on GPIO %d, PIR on GPIO %d\n",
            GPIO_LED_PIN, GPIO_PIR_SENSOR);
    return 0;

err_free_pir:
    gpio_free(GPIO_PIR_SENSOR);
err_free_led:
    gpio_free(GPIO_LED_PIN);
    return ret;
}

static void __exit pir_led_driver_exit(void)
{
    /* Stop thread */
    if (motion_thread)
        kthread_stop(motion_thread);

    /* Turn off LED and free GPIOs */
    gpio_set_value(GPIO_LED_PIN, 0);
    gpio_free(GPIO_PIR_SENSOR);
    gpio_free(GPIO_LED_PIN);

    pr_info("PIR-LED driver exited\n");
}

module_init(pir_led_driver_init);
module_exit(pir_led_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Justin Kim <jiwook021@gmail.com>");
MODULE_DESCRIPTION("PIR sensor-driven LED control (polling)");
MODULE_VERSION("1.0");
