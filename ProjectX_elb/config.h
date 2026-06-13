#ifndef CONFIG_H
#define CONFIG_H

// I2C addresses for MGBot sensors
#define MGS_THP80_ADDR    0x76
#define MGS_WT1_ADDR      0x77
#define MGS_CLM60_ADDR    0x78
#define MGS_FR403_ADDR    0x79
#define MGS_L75_ADDR      0x7A
#define MGS_SND504_ADDR   0x7B
#define MGS_A6_ADDR       0x7C
#define MGS_CO30_ADDR     0x7D
#define MGS_D20_ADDR      0x7E
#define MGL_RGB3_ADDR     0x3F

// Bluetooth configuration
#define BT_RX_PIN         2
#define BT_TX_PIN         3
#define BT_BAUD_RATE      9600

// Timing
#define DATA_INTERVAL     2000  // Send data every 2 seconds

// Error value
#define ERROR_VALUE       -999.0

#endif // CONFIG_H
