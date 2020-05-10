idf_component_register(SRCS "mpu6050.c" "acceleration.c"
                  INCLUDE_DIRS "include"
                  REQUIRES I2C)