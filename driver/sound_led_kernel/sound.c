#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/err.h>

#define GPIO_LED_PIN         26  // BCM 17 -> LED
#define GPIO_AUDIO_SENSOR    27  // BCM 27 -> KY-038 D0 output

static struct task_struct *audio_monitor_thread;

/*
 * Thread function to monitor sound sensor (D0)
 * - Reads GPIO 27 (D0 output of KY-038)
 * - If sound detected, turns on LED
 * - Otherwise, turns LED off
 */
static int audio_monitor_fn(void *data)
{
    while (!kthread_should_stop()) {
        int sound_detected = gpio_get_value(GPIO_AUDIO_SENSOR);

        if (sound_detected) {
            gpio_set_value(GPIO_LED_PIN, 1);
            pr_info("Sound detected: LED ON\n");
        } else {
            gpio_set_value(GPIO_LED_PIN, 0);
            pr_info("No sound: LED OFF\n");
        }

        msleep(100);  // Polling delay
    }
    return 0;
}

static int __init sound_led_driver_init(void)
{
    int ret;

    // LED GPIO setup
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

    // Sound sensor GPIO setup
    ret = gpio_request(GPIO_AUDIO_SENSOR, "audio_sensor_pin");
    if (ret) {
        pr_err("Failed to request GPIO %d for sound sensor: %d\n", GPIO_AUDIO_SENSOR, ret);
        goto err_free_led;
    }
    ret = gpio_direction_input(GPIO_AUDIO_SENSOR);
    if (ret) {
        pr_err("Failed to set GPIO %d as input: %d\n", GPIO_AUDIO_SENSOR, ret);
        goto err_free_sensor;
    }

    // Start kernel thread
    audio_monitor_thread = kthread_run(audio_monitor_fn, NULL, "audio_monitor_thread");
    if (IS_ERR(audio_monitor_thread)) {
        ret = PTR_ERR(audio_monitor_thread);
        pr_err("Failed to create monitoring thread: %d\n", ret);
        goto err_free_sensor;
    }

    pr_info("Sound sensor LED driver initialized\n");
    return 0;

err_free_sensor:
    gpio_free(GPIO_AUDIO_SENSOR);
err_free_led:
    gpio_free(GPIO_LED_PIN);
    return ret;
}

static void __exit sound_led_driver_exit(void)
{
    if (audio_monitor_thread)
        kthread_stop(audio_monitor_thread);

    gpio_set_value(GPIO_LED_PIN, 0);
    gpio_free(GPIO_AUDIO_SENSOR);
    gpio_free(GPIO_LED_PIN);

    pr_info("Sound sensor LED driver exited\n");
}

module_init(sound_led_driver_init);
module_exit(sound_led_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Justin Kim <jiwook021@gmail.com>");
MODULE_DESCRIPTION("Sound sensor (KY-038) driven LED control (polling)");
MODULE_VERSION("1.0");
