#include "gpio.h"


//gpio setting tools
void exportGPIO(int pin) {
    FILE *exportFile = fopen("/sys/class/gpio/export", "w");
    if (exportFile == NULL) {
        perror("Failed to open export file");
        exit(1);
    }
    fprintf(exportFile, "%d", pin);
    fclose(exportFile);
}

void unexportGPIO(int pin) {
    FILE *unexportFile = fopen("/sys/class/gpio/unexport", "w");
    if (unexportFile == NULL) {
        perror("Failed to open unexport file");
        exit(1);
    }
    fprintf(unexportFile, "%d", pin);
    fclose(unexportFile);
}

void setGPIODirection(int pin, const char *direction) {
    char path[35];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);
    FILE *directionFile = fopen(path, "w");
    if (directionFile == NULL) {
        perror("Failed to open direction file");
        exit(1);
    }
    fputs(direction, directionFile);
    fclose(directionFile);
}

void setGPIOValue(int pin, int value) {
    char path[30];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
    FILE *valueFile = fopen(path, "w");
    if (valueFile == NULL) {
        perror("Failed to open value file");
        exit(1);
    }
    fprintf(valueFile, "%d", value);
    fclose(valueFile);
}

int getGPIOValue(int pin){
    char path[40];
    int value;


    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);

    FILE *valueFile = fopen(path, "r");
    if (valueFile == NULL){
        perror("Failed to open value file");
        exit(1);
    }

    // 파일에서 값 읽기
    if (fscanf(valueFile, "%d", &value) != 1) {
        perror("Failed to read value");
        fclose(valueFile); // 파일 닫기
        exit(1);
    }

    // 파일 닫기
    fclose(valueFile);

    return value; // 읽은 GPIO 값 반환

}
