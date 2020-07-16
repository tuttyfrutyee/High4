idf_component_register(SRCS "mode0.c" "mode1.c" 
                  INCLUDE_DIRS "include"
                  REQUIRES NETWORKS SD IMUGATHER MPU6050 HELPER)