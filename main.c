#include "bme280_lib/bme280.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>

/* -------------------------------------------------------------------------------- */
#define RETURN_ON_FAIL(status) ({ if (status != BME280_OK) return status;})
#define I2C "/dev/i2c-1"
#define MEASUREMENTS_COUNT 60

static double temperature[MEASUREMENTS_COUNT];
static double pressure[MEASUREMENTS_COUNT];
static double humidity[MEASUREMENTS_COUNT];

int file_desc;   // I2C device file descriptor
/* -------------------------------------------------------------------------------- */

/* ---------------------------- functions declarations ---------------------------- */
void user_delay_ms(uint32_t period);

int8_t user_i2c_read(uint8_t id, uint8_t reg_addr, uint8_t* data, uint16_t len);

int8_t user_i2c_write(uint8_t id, uint8_t reg_addr, uint8_t* data, uint16_t len);

int8_t setup_sensor_settings(struct bme280_dev* dev);

int8_t run_measurements(struct bme280_dev* dev);

void save_measurements(struct bme280_data* comp_data, int8_t index);

double get_average_value(double array[MEASUREMENTS_COUNT]);

void save_to_file(const char* file_name, double value);
/* --------------------------------------------------------------------------------- */


int main(int argc, char* argv[])
{
    printf("BME280 - START MEASUREMENTS\n");

    int8_t status = BME280_OK;
    struct bme280_dev dev;

    /* ------------------------ open I2C configuration file ------------------------ */
    file_desc = open(I2C, O_RDWR);
    if (file_desc < 0)
    {
        printf("Cannot open I2C bus!\n");
        return 1;
    }
    /* ----------------------------------------------------------------------------- */

    /*  ---------------------- setup device in slave mode -------------------------- */
    if (ioctl(file_desc, I2C_SLAVE, 0x77) < 0)
    {
        printf("Cannot acquire bus access!\n");
        return 1;
    }
    /* ----------------------------------------------------------------------------- */

    /* ---------------------------- init bme280 sensor ----------------------------- */
    dev.dev_id = BME280_I2C_ADDR_SEC;
    dev.intf = BME280_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_ms = user_delay_ms;

    status = bme280_init(&dev);
    if (status != BME280_OK)
    {
        printf("BME280 init failed with status: %d\n", status);
        return 1;
    }
    /* ----------------------------------------------------------------------------- */

    /* ---------------------------- setup and run sensor --------------------------- */
    status = setup_sensor_settings(&dev);
    if (status != BME280_OK)
    {
        printf("BME280 setup failed with status: %d\n", status);
        return 1;
    }

    status = run_measurements(&dev);
    if (status != BME280_OK)
    {
        printf("BME280 get sensor data failed with status: %d\n", status);
        return 1;
    }
    /* ----------------------------------------------------------------------------- */

    /* ---------------------------- get average values ----------------------------- */
    double avg_temperature = get_average_value(temperature);
    save_to_file("temperature", avg_temperature);
    printf("temperature: %0.2f\n", avg_temperature);

    double avg_pressure = get_average_value(pressure);
    save_to_file("pressure", avg_pressure);
    printf("pressure: %0.2f\n", avg_pressure);

    double avg_humidity = get_average_value(humidity);
    save_to_file("humidity", avg_humidity);
    printf("humidity: %0.2f\n", avg_humidity);
    /* ----------------------------------------------------------------------------- */

    printf("BME280 - STOP MEASUREMENTS\n");

    return 0;
}


void user_delay_ms(uint32_t period)
{
    // from us to ms
    usleep(period * 1000);
}


int8_t user_i2c_read(uint8_t id, uint8_t reg_addr, uint8_t* data, uint16_t len)
{
    // send read request
    write(file_desc, &reg_addr, 1);

    // read data
    read(file_desc, data, len);

    return 0;
}


int8_t user_i2c_write(uint8_t id, uint8_t reg_addr, uint8_t* data, uint16_t len)
{
    // build message
    int8_t* buf = malloc(len + 1);
    buf[0] = reg_addr;
    memcpy(buf + 1, data, len);

    // send message
    write(file_desc, buf, len + 1);

    free(buf);
    return 0;
}


int8_t setup_sensor_settings(struct bme280_dev* dev)
{
    int8_t status = BME280_OK;

    // setup sensor settings
    dev->settings.osr_h = BME280_OVERSAMPLING_1X;
    dev->settings.osr_p = BME280_OVERSAMPLING_16X;
    dev->settings.osr_t = BME280_OVERSAMPLING_2X;
    dev->settings.filter = BME280_FILTER_COEFF_16;
    dev->settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

    // set sensor settings
    uint8_t desired_settings = BME280_OSR_PRESS_SEL;
    desired_settings |= BME280_OSR_TEMP_SEL;
    desired_settings |= BME280_OSR_HUM_SEL;
    desired_settings |= BME280_STANDBY_SEL;
    desired_settings |= BME280_FILTER_SEL;

    status = bme280_set_sensor_settings(desired_settings, dev);
    RETURN_ON_FAIL(status);

    // set sensor mode
    status = bme280_set_sensor_mode(BME280_NORMAL_MODE, dev);
    RETURN_ON_FAIL(status);

    return status;
}


int8_t run_measurements(struct bme280_dev* dev)
{
    int8_t status = BME280_OK;
    struct bme280_data comp_data;

    for (int i = 0; i < MEASUREMENTS_COUNT; i++)
    {
        // wait for a measurement
        dev->delay_ms(1000);

        // read data
        status = bme280_get_sensor_data(BME280_ALL, &comp_data, dev);
        RETURN_ON_FAIL(status);

        // save data
        save_measurements(&comp_data, i);
    }

    return status;
}


void save_measurements(struct bme280_data* comp_data, int8_t index)
{
    if (index < MEASUREMENTS_COUNT)
    {
        temperature[index] = comp_data->temperature;
        pressure[index] = comp_data->pressure / 100;
        humidity[index] = comp_data->humidity;
    }
}


double get_average_value(double array[MEASUREMENTS_COUNT])
{
    double total = 0;
    for (int8_t i = 0; i < MEASUREMENTS_COUNT; i++)
        total += array[i];

    return total / (double)MEASUREMENTS_COUNT;
}


void save_to_file(const char* file_name, double value)
{
    FILE* file = fopen(file_name, "wb+");
    fprintf(file, "%0.2f", value);
    fclose(file);
}

