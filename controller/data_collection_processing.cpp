
#include "mbed.h"
#include <math.h>
// Sensors drivers present in the BSP library
#include "stm32l475e_iot01_accelero.h"
#include "stm32l475e_iot01_gyro.h"
#include "lsm6dsl.h"

int tracking = 0; // use this to start and stop tracking

/**
 * @brief Initialize the accelerometer and register interrupts.
 *
 * This function raises an assertion error if the sensors
 * cannot be initialized.
 */
void sensors_init()
{
    uint32_t status;    // For saving the result of an initialization

    // Initialize the accelerometer and check for success
    status = BSP_ACCELERO_Init(); 
    assert(status == ACCELERO_OK);

    // Initialize the gyroscope and check for success
    uint8_t g_status; 
    g_status = BSP_GYRO_Init(); 
    assert(g_status == GYRO_OK); 
    
}

/**
 * @brief Read the accelerometer (signed) counts for x, y and z, respecitvely.
 *
 * @param raw_counts The array to store the raw values for x, y, and z.
 */
void read_accelerometer_raw(int16_t * raw_counts) {
    
    BSP_ACCELERO_AccGetXYZ(raw_counts);
}

/**
 * @brief Read the angular accelerometer (signed) counts for x, y and z, respecitvely.
 *
 * @param pfData pointer on floating array    
 */
void read_gyro_raw(float* pfData){
    BSP_GYRO_GetXYZ(pfData);
}



/**
 * @brief Read and print (to stdout) the accelerometer data. Also print the number of blocks to left moved or right moved
 */
void read_accelerometer() {
    float magnitude_g = 0.0;
    float x_mg = 0.0, y_mg = 0.0, z_mg = 0.0;

    int16_t xyz_counts[3];
    read_accelerometer_raw(xyz_counts);
    // Convert the raw xyz values to mg
    x_mg = xyz_counts[0] * 0.061;
    y_mg = xyz_counts[1] * 0.061;
    z_mg = xyz_counts[2] * 0.061;
    // Calculate the mangitude of acceleration and convert it to g (1g = 1000mg)
    // The magnitude is the square root of the sum of x^2, y^2, and z^2
    magnitude_g = sqrt(pow(x_mg, 2) + pow(y_mg, 2) + pow(z_mg, 2)) / 1000;
    printf("ACCELEROMETER  %f g; (x, y, z) = (%.1f mg, %.1f mg, %.1f mg)\n",
        magnitude_g, x_mg, y_mg, z_mg
    );
}

/**
 * @brief Print (to stdout) information about the accelerometer
 * only when output_toggle is true.
 *
 * Also turn LED1 on when output_toggle is true and off otherwise.
 *
 * This function does not return.
 */
void start_imu_tracking()
{   
    DigitalOut led(LED1); 
    while (true) {
        // React to (and possibly mutate) application state
        if (output_on == 1){
            read_accelerometer();
        }
        if (output_on != prev_state){
            led = !led; 
            prev_state = output_on;
        }
        thread_sleep_for(100);
    }
}
