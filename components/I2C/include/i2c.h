#ifndef I2C

#define I2C


#include "driver/i2c.h"

//#define BOARD


#ifdef BOARD

#define I2C_MASTER_SCL_IO GPIO_NUM_22               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO GPIO_NUM_21               /*!< gpio number for I2C master data  */

#else

#define I2C_MASTER_SCL_IO GPIO_NUM_23               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO GPIO_NUM_22               /*!< gpio number for I2C master data  */

#endif



#define I2C_MASTER_NUM 1                            /*!< I2C port number for master dev */
#define I2C_MASTER_TX_BUF_DISABLE 0                 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                 /*!< I2C master doesn't need buffer */


#define WRITE_BIT 0                                 /*!< I2C master write */
#define READ_BIT 1                                  /*!< I2C master read */
#define ACK_CHECK_EN 0x1                            /*!< I2C master will check ack from slave*/
#define ACK_VAL 0x0                                 /*!< I2C ack value */
#define NACK_VAL 0x1                                /*!< I2C nack value */
#define I2C_ACK_VAL ACK_VAL
#define I2C_NACK_VAL NACK_VAL


esp_err_t i2c_master_init(uint32_t freqHz);

int8_t user_i2c_read(uint8_t deviceAddr, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);

int8_t user_i2c_write(uint8_t deviceAddr, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);

int checkI2cError(int ret, char* operationName);

#endif