#include <mraa.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ANGLE_ZERO 630
#define PROPAGATION_MARGIN 410
#define FREQUENCY_CONST 19710
#define PIN_PWM0 12

static mraa_gpio_context gpio;
static mraa_result_t ret;

void get_interval(struct timespec*, struct timespec*, int);
void reset_gpio(int);

int main(int argc, char *argv[]) {
    /* Argument check */
    if(argc != 2) {
        fprintf(stdout, "Usage: %s <angle>\n", argv[0]);
        return 1;
    }

    /* Kill old process if exists */
    pid_t pid=0;
    FILE*  pid_fp = fopen("/tmp/servo_pid", "r");
    if (pid_fp != NULL) {
        if(fscanf(pid_fp, "%d\n", &pid) == 1) {
            kill(pid, SIGINT);
            waitpid(pid, NULL, 0);
            fclose(pid_fp);
        }
    }

    /* Variables declaration */
    struct timespec high;
    struct timespec low;
    int pin = PIN_PWM0;
    int angle = strtol(argv[1], NULL, 0);
    if (angle < 0){
        exit(0);
    } else {
        angle = angle%180;
    }

    /* Fork a child process */
    if ((pid = fork()) == 0) {
        /* File out PID info for this process */
        pid_fp = fopen("/tmp/servo_pid", "w");
        if (pid_fp != NULL) {
            pid = getpid();
            fprintf(pid_fp, "%d\n", pid);
            fclose(pid_fp);
        }

        /* Time init */
        get_interval(&high, &low, angle);
    
        /* MRAA init */
        mraa_init();
        gpio = mraa_gpio_init_raw(pin);
        if(gpio == NULL) {
            perror("Failed to obtain GPIO context\n");
            return 1;
        }
        ret = mraa_gpio_dir(gpio, MRAA_GPIO_OUT);
        if(ret != MRAA_SUCCESS) {
            perror("MRAA unsuccessful\n");
            return 1;
        }
    
        /* Set SIGINT handler */
        signal(SIGINT, reset_gpio);

        /* Generate PWM for 2000*20ms = 40s */
        for(;;) {
            // HIGH
            mraa_gpio_write(gpio, 1);
            nanosleep(&high, NULL);
            // LOW
            mraa_gpio_write(gpio, 0);
            nanosleep(&low, NULL);
        }
    }
    return 0;
}

/* Set HIGH and LOW interval in nanoseconds for 20ms pulse
 * angle: angle in degree, assuming the range in 0-180      */
void get_interval(struct timespec* high, struct timespec* low, int angle) {
    unsigned long interval = (angle*10) + ANGLE_ZERO;
    high->tv_sec = 0;
    low->tv_sec = 0;
    high->tv_nsec = (interval - PROPAGATION_MARGIN)*1000;
    low->tv_nsec = (FREQUENCY_CONST - interval)*1000; // Subtract the pulse interval from 20ms
    //fprintf(stdout, "Rotating servo at angle: %d (Pulse: %lu/%lu usec)\n", angle, interval, 20000-interval);
}

/* Reset the GPIO before exit */
void reset_gpio(int signum) {
    /* Make the pin initial state  */
    mraa_gpio_dir(gpio, MRAA_GPIO_IN);
    mraa_deinit();
    exit(0);
}

