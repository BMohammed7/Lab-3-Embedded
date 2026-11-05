#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

#define ADDRESS 0x70

int fd;

void command(uint8_t cmd) {
    if (i2c_smbus_write_byte(fd, cmd) < 0) {
        perror("I2C command failed");
        exit(1);
    }
}

void display(uint16_t a, uint16_t b,
             uint16_t c, uint16_t d) {

    uint8_t data[17] = {0};

    uint16_t values[4] = {a, b, c, d};
    int positions[4] = {0, 2, 6, 8};

    for (int i = 0; i < 4; i++) {
        data[1 + positions[i]] = values[i] & 0xFF;
        data[2 + positions[i]] = values[i] >> 8;
    }

    if (write(fd, data, sizeof(data)) != sizeof(data)) {
        perror("Display write failed");
        exit(1);
    }
}

uint16_t digit(int n) {
    uint16_t numbers[] = {
        0x3F, 0x06, 0x5B, 0x4F,
        0x66, 0x6D, 0x7D, 0x07,
        0x7F, 0x6F
    };

    return numbers[n];
}

void showNumber(int n) {
    display(
        digit((n / 1000) % 10),
        digit((n / 100) % 10),
        digit((n / 10) % 10),
        digit(n % 10)
    );
}

void task1() {
    printf("Task 1: Blinking dashes\n");

    for (int i = 0; i < 10; i++) {
        display(0x40, 0x40, 0x40, 0x40);
        sleep(1);

        display(0, 0, 0, 0);
        sleep(1);
    }
}

void task2() {
    printf("Task 2: Display ABCD\n");

    display(0x77, 0x7C, 0x39, 0x5E);
    sleep(5);
}

void task3() {
    printf("Task 3: Countdown 10 to 0\n");

    for (int i = 10; i >= 0; i--) {
        showNumber(i);
        sleep(1);
    }
}

void task4() {
    printf("Task 4: Digital clock\n");

    for (int i = 0; i <= 60; i++) {
        int minutes = i / 60;
        int seconds = i % 60;

        display(
            digit(minutes / 10),
            digit(minutes % 10),
            digit(seconds / 10),
            digit(seconds % 10)
        );

        sleep(1);
    }
}

int main() {

    fd = open("/dev/i2c-1", O_RDWR);

    if (fd < 0) {
        perror("Cannot open I2C");
        return 1;
    }

    if (ioctl(fd, I2C_SLAVE, ADDRESS) < 0) {
        perror("Cannot connect to display");
        close(fd);
        return 1;
    }

    command(0x21);
    command(0x81);
    command(0xEF);

    int choice;

    printf("1. Blinking dashes\n");
    printf("2. Display ABCD\n");
    printf("3. Countdown 10 to 0\n");
    printf("4. Digital clock\n");

    printf("Choose task: ");

    if (scanf("%d", &choice) != 1) {
        close(fd);
        return 1;
    }

    switch (choice) {
        case 1:
            task1();
            break;

        case 2:
            task2();
            break;

        case 3:
            task3();
            break;

        case 4:
            task4();
            break;

        default:
            printf("Invalid choice\n");
    }

    display(0, 0, 0, 0);
    close(fd);

    return 0;
}
