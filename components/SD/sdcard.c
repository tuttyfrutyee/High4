#include "sdcard.h"
#include "math.h"
#include "errno.h"

#define PIN_NUM_MISO 17
#define PIN_NUM_MOSI 16
#define PIN_NUM_CLK  19
#define PIN_NUM_CS   18

static const char *TAG = "sdcard";

int getNextMode(int* modes, int size, int currentMode);


void initSdCard(){

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();


    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    slot_config.gpio_miso = PIN_NUM_MISO;
    slot_config.gpio_mosi = PIN_NUM_MOSI;
    slot_config.gpio_sck  = PIN_NUM_CLK;
    slot_config.gpio_cs   = PIN_NUM_CS;

    //gpio_set_pull_mode(PIN_NUM_MOSI, GPIO_PULLUP_ONLY);

    host.max_freq_khz = 400;


    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024 * 100
    };


    sdmmc_card_t* card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                "If you want the card to be formatted, set format_if_mount_failed = true.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }

    sdmmc_card_print_info(stdout, card);

    //clearSensorData();

    struct stat st;

    if (stat("/sdcard/high4", &st) == -1) {
     mkdir("/sdcard/high4", 0777);
    }


}
int totalWroteDataBytes = 0;


void writeToFile(int8_t* bytes, int count, char* fileName){
    const char fileFullName[30];

    strcpy(fileFullName,"/sdcard/high4/");
    strcat(fileFullName, fileName);
    strcat(fileFullName, ".txt");
    errno = 0;
    FILE* f = fopen(fileFullName,"a+b");
    if(f == NULL) {
        printf("Error %d \n", errno);
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;        
    }
    fwrite(bytes, sizeof(int8_t), count, f);

    fclose(f);    
}
void writeToBinFile(int8_t* bytes, int count, char* fileName){
    const char fileFullName[30];

    strcpy(fileFullName,"/sdcard/high4/");
    strcat(fileFullName, fileName);
    strcat(fileFullName, ".bin");
    errno = 0;
    FILE* f = fopen(fileFullName,"a+b");
    if(f == NULL) {
        printf("Error %d \n", errno);
        ESP_LOGE(TAG, "Failed to open file for writing");
        printf("\n %s \n", fileFullName);
        return;        
    }
    fwrite(bytes, sizeof(int8_t), count, f);

    fclose(f);    
}

char* readFile(int count, const char* fileName, FILE** f_){

    if(!(*f_)){

        const char fileFullName[30];

        strcpy(fileFullName,"/sdcard/high4/");
        strcat(fileFullName, fileName);
        strcat(fileFullName, ".txt");  

        printf("\n %s \n", fileFullName);

        (*f_) = fopen(fileFullName,"r");
        if((*f_) == NULL) {
            ESP_LOGE(TAG, "Failed to open file for reading\n");
            return;        
        }


    }



    char* buffer = (char*)malloc(sizeof(char) * (count+2));

    fgets( buffer, count, *f_);

    if(buffer)
        printf("\n %s \n",buffer);

    return buffer;
}

void getAndUpdateLookup(int* recordCounter_, int* currentModeCounter_){
    FILE *ft;
    char const *name = "/sdcard/high4/lookup.txt";
    int ch;
    ft = fopen(name, "r+");
    if (ft == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    int bigBufferSize = 30;

    char * bigBuffer = (char*) calloc(bigBufferSize,sizeof(char));

    int counter = 0;

    int posRecordCounterLeft = -1;
    int posRecordCounterRight = -1;
    int posCurrentModeCounterLeft = -1;
    int posCurrentModeCounterRight = -1;
    int posArrayLeft = -1;
    int posArrayRight = -1;

    while((ch = fgetc(ft)) != EOF){
        counter++;

        if(bigBufferSize <= counter){
            bigBufferSize += 20;
            bigBuffer = realloc(bigBuffer, bigBufferSize * sizeof(char));
        }
        
        if(ch == '$'){
            if(posRecordCounterLeft == -1)
                posRecordCounterLeft = counter-1;
            else if(posCurrentModeCounterLeft == -1)
                posCurrentModeCounterLeft = counter-1;
        }

        if(ch == '\n'){
            if(posRecordCounterRight == -1)
                posRecordCounterRight = counter-1;
            else if(posCurrentModeCounterRight == -1)
                posCurrentModeCounterRight = counter-1;      
        }

        if(ch == '[') posArrayLeft = counter -1;
        if(ch == ']') posArrayRight = counter -1;

        
        bigBuffer[counter-1] = ch;
    } 


    //record counter
    int recordCounter = 0;
    for(int i = 0; i < (posRecordCounterRight - posRecordCounterLeft -1); i++){
        recordCounter += (((int)bigBuffer[posRecordCounterLeft + i + 1]) - 48) * pow(10, posRecordCounterRight - posRecordCounterLeft - i - 2);
    }

    //currentModeCounter
    int currentModeCounter = 0;
    for(int i = 0; i < (posCurrentModeCounterRight - posCurrentModeCounterLeft -1); i++){
        currentModeCounter += (((int)bigBuffer[posCurrentModeCounterLeft + i + 1]) - 48) * pow(10, posCurrentModeCounterRight - posCurrentModeCounterLeft - i - 2);
    }    

    //modeNumbers
    int arrayCounter = 0;
    int* modeNumbers = calloc((posArrayRight - posArrayLeft-1), sizeof(int));
    int tempL = posArrayLeft + 1;

    for(int i = 0; i < (posArrayRight - posArrayLeft); i++){

        if(bigBuffer[posArrayLeft + i + 1] == ',' || bigBuffer[posArrayLeft + i + 1] == ']'){
            int temp = 0;
            for(int j = tempL; j < (posArrayLeft + i + 1); j++)
                temp += ((int)(bigBuffer[j]) - 48) * pow(10, posArrayLeft + i + 1 - tempL - 1);
            modeNumbers[arrayCounter] = temp;
            arrayCounter ++;
            tempL = posArrayLeft + i + 2;
        }
    }

    //fill the return variables
    (*recordCounter_) = recordCounter;
    (*currentModeCounter_) = currentModeCounter;

    int nextCurrentModeCounter = -1;
    for(int i = 0; i < arrayCounter; i++)
        if(modeNumbers[i] == currentModeCounter)
            nextCurrentModeCounter = modeNumbers[(i+1) % arrayCounter];

    //now update the counters for the next record

    freopen(name, "w", ft);

     if (ft == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    } 

    //now the contents of the file is deleted actually

    char * recordCounterUpdatedString = calloc((int)log10(recordCounter + 1) + 2, sizeof(char));
    char * currentModeCounterUpdatedString;
    
    if(nextCurrentModeCounter == 0)
        currentModeCounterUpdatedString = calloc(2, sizeof(char));
    else
        currentModeCounterUpdatedString = calloc((int) log10(nextCurrentModeCounter) + 2, sizeof(char));

    itoa ( recordCounter + 1, recordCounterUpdatedString, 10);
    itoa ( nextCurrentModeCounter, currentModeCounterUpdatedString, 10);



    for(int i = 0; i < counter;i++){


        if(i == posRecordCounterLeft){
            
            fputc(bigBuffer[i], ft);            

            for(int j = 0; j < (int)log10(recordCounter + 1) + 1; j++){
                fputc(recordCounterUpdatedString[j], ft);
            }

            i = posRecordCounterRight;

        }else if(i == posCurrentModeCounterLeft){

            fputc(bigBuffer[i], ft);            

            for(int j = 0; j < (int)log10(nextCurrentModeCounter) + 1; j++){
                fputc(currentModeCounterUpdatedString[j], ft);
            }

            i = posCurrentModeCounterRight;

        }

        fputc(bigBuffer[i], ft);

    }

    fclose(ft);
    return 0;    
}

void deleteLastSensorFile(char* fileName){
    const char fileFullName[30];

    strcpy(fileFullName,"/sdcard/high4/");
    strcat(fileFullName, fileName);
    strcat(fileFullName, ".bin");    

    remove(fileFullName);
}

void clearSensorData(){
    remove("/sdcard/high4/sData.bin");
};

void clearErrorLog(){
    remove("/sdcard/high4/errorLog.txt");
};

void unMountSdCard(){
    printf("\n Unmounting card\n");
    esp_vfs_fat_sdmmc_unmount();
}


//helper helper functions

int getNextMode(int* modes, int size, int currentMode){
    for(int i = 0; i < size; i++){
        if(modes[i] == currentMode){
            if((i+1) != size)
                return modes[i+1];
            return modes[0];
        }
    }
    return -1;
}

