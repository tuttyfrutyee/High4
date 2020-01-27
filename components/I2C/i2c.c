#include "./include/i2c.h"
#include "esp_log.h"

static const char *TAG = "i2c driver";


esp_err_t i2c_master_init(uint32_t freqHz)
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = freqHz;
    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode,
                              I2C_MASTER_RX_BUF_DISABLE,
                              I2C_MASTER_TX_BUF_DISABLE, 0);
}

int8_t user_i2c_read(uint8_t deviceAddr, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    if (len == 0) return true;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (true)
    {
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, ( deviceAddr << 1 ) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, reg_addr, true);
        if (!reg_data)
            i2c_master_stop(cmd);
    }
    if (true)
    {
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, ( deviceAddr << 1 ) | I2C_MASTER_READ, true);
        if (len > 1) i2c_master_read(cmd, reg_data, len-1, I2C_ACK_VAL);
        i2c_master_read_byte(cmd, reg_data + len-1, I2C_NACK_VAL);
        i2c_master_stop(cmd);
    }
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

return err;
}

int8_t user_i2c_write(uint8_t deviceAddr, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, deviceAddr << 1 | I2C_MASTER_WRITE, true);

    i2c_master_write_byte(cmd, reg_addr, true);
    
    i2c_master_write(cmd, reg_data, len, true);
        
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    
return err;
}

int checkI2cError(int ret, char* operationName){
    if (ret == ESP_ERR_TIMEOUT) {
        ESP_LOGE(TAG, "I2C Timeout");
        return 0;
    } else if (ret == ESP_OK) {
        return 1;
    } else {
        ESP_LOGW(TAG, "%s: No ack, sensor not connected...skip...", esp_err_to_name(ret));
        ESP_LOGW(TAG, "operation name = %s", operationName);
        return 0;
    }
}