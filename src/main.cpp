// Copyright (c) 2020 Antoine TRAN TAN
// Copyright (c) 2017 Sarah MARSH

#include "mbed.h"
#include "rtos.h"
#include "C12832.h"
#include "mp3_resources.h"

/**
 * \defgroup commands Commands
 * \{
 */
#define MP3_WRITE_CMD 0x02
#define MP3_READ_CMD 0x03
#define MP3_BASE_ADDR 0x00
#define MP3_MODE_ADDR 0x00
#define MP3_STATUS_ADDR 0x01
#define MP3_BASS_ADDR 0x02
#define MP3_CLOCKF_ADDR 0x03
#define MP3_DECODE_TIME_ADDR 0x04
#define MP3_AUDATA_ADDR 0x05
#define MP3_WRAM_ADDR 0x06
#define MP3_WRAMADDR_ADDR 0x07
#define MP3_HDAT0_ADDR 0x08
#define MP3_HDAT1_ADDR 0x09
#define MP3_AIADDR_ADDR 0x0A
#define MP3_VOL_ADDR 0x0B
#define MP3_AICTRL0_ADDR 0x0C
#define MP3_AICTRL1_ADDR 0x0D
#define MP3_AICTRL2_ADDR 0x0E
#define MP3_AICTRL3_ADDR 0x0F
/** \} */

/**
 * \defgroup error_code Error Code
 * \{
 */
#define MP3_OK 0
#define MP3_ERROR -1
/** \} */

// Using Arduino pin notation
C12832 lcd(D11, D13, D12, D7, D10);

SPI Myspi(PE_6, PE_5, PE_2);

DigitalOut SS(PE_4, 1), dcs(PD_12, 1);

DigitalIn dreq(PF_8);

char mp3_is_busy();
void mp3_cmd_write(char address, short input);
short mp3_cmd_read(char address);
char mp3_data_write(char input);
char mp3_data_write_32(char *input32);
void mp3_set_volume(char vol_left, char vol_right);
void application_init(void);
void application_task(void);

int main()
{
    // int j = 0;
    // lcd.cls();
    // lcd.locate(0, 3);
    // lcd.printf("mbed application shield!");

    // while (true)
    // { // this is the third thread
    //     lcd.locate(0, 15);
    //     lcd.printf("Counting : %d", j++);
    //     ThisThread::sleep_for(1000ms);
    // }

    application_init();

    while(true)
    {
        application_task( );
    }
}

char mp3_is_busy()
{
    return dreq;
}

void mp3_cmd_write(char address, short input)
{
    char tmp[4] = {0};

    tmp[0] = MP3_WRITE_CMD;
    tmp[1] = address;
    tmp[2] = (char)((input >> 8) & 0xFF);
    tmp[3] = (char)(input & 0xFF);

    SS = 0;
    Myspi.write(tmp, 4, nullptr, 0);
    SS = 1;

    // Poll DREQ pin and block until module is ready to receive another command
    while (!dreq)
    {
    }
}

short mp3_cmd_read(char address)
{
    char tmp1[2] = {0}, tmp2[2] = {0};
    short result = 0;

    tmp1[0] = MP3_READ_CMD;
    tmp1[1] = address;

    SS = 0;
    Myspi.write(tmp1, 2, tmp2, 2);
    SS = 1;

    result = tmp2[0];
    result <<= 8;
    result |= tmp2[1];

    // Poll DREQ pin and block until module is ready to receive another command
    while (!dreq)
    {
    }

    return result;
}

char mp3_data_write(char input)
{
    if (!dreq)
    {
        return MP3_ERROR;
    }
    dcs = 0;
    Myspi.write(input);
    dcs = 1;
    return MP3_OK;
}

char mp3_data_write_32(char *input32)
{
    if (!dreq)
    {
        return MP3_ERROR;
    }
    dcs = 0;
    Myspi.write((char *)input32, 32, nullptr, 0);
    dcs = 1;
    return MP3_OK;
}

void mp3_set_volume(char vol_left, char vol_right)
{
    mp3_cmd_write(MP3_VOL_ADDR, ((short)vol_left << 8) | vol_right);
}

void application_init(void)
{
    // Click initialization.
    SS = 0;
    SS = 1;
    Myspi.format(8, 0);
    Myspi.frequency(1000000);

    // mp3_reset( &mp3 );

    mp3_cmd_write(MP3_MODE_ADDR, 0x0800);
    mp3_cmd_write(MP3_BASS_ADDR, 0x7A00);
    mp3_cmd_write(MP3_CLOCKF_ADDR, 0x2000);

    // MP3 set volume, maximum volume is 0x00 and total silence is 0xFE.
    mp3_set_volume(0x2F, 0x2F);
    ThisThread::sleep_for(1000ms);

    // log_info( &logger, " Application Task " );
}

void application_task(void)
{
    int file_size = sizeof(gandalf_sax_mp3_compressed);
    int file_pos = 0;
    char data_buf[32] = {0};

    // log_printf( &logger, " Playing audio..." );
    for (file_pos = 0; (file_pos + 32) <= file_size; file_pos += 32)
    {
        memcpy(data_buf, &gandalf_sax_mp3_compressed[file_pos], 32);
        while (MP3_OK != mp3_data_write_32(data_buf))
            ;
    }

    for (; file_pos < file_size; file_pos++)
    {
        while (MP3_OK != mp3_data_write(gandalf_sax_mp3_compressed[file_pos]));
    }
    // log_printf( &logger, "Done\r\n\n" );
}