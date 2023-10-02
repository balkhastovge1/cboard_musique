// Copyright (c) 2020 Antoine TRAN TAN
// Copyright (c) 2017 Sarah MARSH

#include "mbed.h"
#include "rtos.h"
#include "C12832.h"
#include "mp3_resources.h"

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

#define MP3_OK 0
#define MP3_ERROR -1

void application_init();
void application_task();
void mp3_cfg_setup(int bit, char mode, int frequence);
void mp3_cmd_write(char address, short input);
void mp3_set_volume(short vol_left, short vol_right);

short mp3_cmd_read(char address);
int mp3_data_write(char input);
int mp3_data_write_32(char *input32);

// Using Arduino pin notation
C12832 lcd(D11, D13, D12, D7, D10);

SPI Myspi(PE_6, PE_5, PE_2);
DigitalOut SS(PE_4, 1);
DigitalOut dcs(PD_12, 1);
InterruptIn dreq(PF_8);
// DigitalOut ledR(PE_8,1);

EventFlags eflags;

int main()
{
    dreq.rise(
        []()
        {
            eflags.set(0x01);
        });

    application_init();

    while (1)
    {
        application_task();
    }
}

void application_init(void)
{
    mp3_cfg_setup(8, 0, 12000000);

    mp3_cmd_write(MP3_MODE_ADDR, 0x0800);
    mp3_cmd_write(MP3_BASS_ADDR, 0x7A00);
    mp3_cmd_write(MP3_CLOCKF_ADDR, 0x2000);

    mp3_set_volume(0x2F, 0x2F);
    ThisThread::sleep_for(1000ms);
}

void application_task(void)
{
    int file_size = sizeof(gandalf_sax_mp3_compressed);
    int file_pos = 0;
    char data_buf[32] = {0};

    for (file_pos = 0; (file_pos + 32) <= file_size; file_pos += 32)
    {
        memcpy(data_buf, &gandalf_sax_mp3_compressed[file_pos], 32);
        while (MP3_OK != mp3_data_write_32(data_buf))
        {
        }
    }

    for (; file_pos < file_size; file_pos++)
    {
        while (MP3_OK != mp3_data_write(gandalf_sax_mp3_compressed[file_pos]))
        {
        }
    }
}

void mp3_cfg_setup(int bit, char mode, int frequence)
{
    SS = 0;
    SS= 1;
    Myspi.format(bit, mode);
    Myspi.frequency(frequence);
}

/**
 * @brief  Function writes one byte (command) to MP3
 * @param ctx       Click object.
 * @param address   adddress of register whitch be written
 * @param input     command which be written
 */

void mp3_cmd_write(char address, short input)
{
    char tmp[4] = {0}, recu[4] = {0};

    tmp[0] = MP3_WRITE_CMD;
    tmp[1] = address;
    tmp[2] = (char)((input >> 8) & 0xFF);
    tmp[3] = (char)(input & 0xFF);

    SS = 0;
    Myspi.write(tmp, 4, recu, 0);
    SS = 1;

    // Poll DREQ pin and block until module is ready to receive another command
    eflags.wait_any(0x01);

    // ledR = !ledR;
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
    eflags.wait_any(0x01);
    // ledR = !ledR;

    return result;
}

int mp3_data_write(char input)
{
    if (!dreq)
    {
        return MP3_ERROR;
    }

    dcs = 0;
    Myspi.write(input);
    dcs = 1;

    // Warning ajout pour essai
    //  Poll DREQ pin and block until module is ready to receive another command

    if (!dreq)
    {
        eflags.wait_any(0x01);
        // ledR = !ledR;
    }
    eflags.clear(0x01);

    return MP3_OK;
}

int mp3_data_write_32(char *input32)
{
    if (!dreq)
    {
        return MP3_ERROR;
    }
    dcs = 0;
    Myspi.write((char *)input32, 32, nullptr, 0);
    dcs = 1;

    // Warning ajout pour essai
    //  Poll DREQ pin and block until module is ready to receive another command

    if (!dreq)
    {
        eflags.wait_any(0x01);
        // ledR = !ledR;
    }
    eflags.clear(0x01);

    return MP3_OK;
}

void mp3_set_volume(short vol_left, short vol_right)
{
    mp3_cmd_write(MP3_VOL_ADDR, ((short)vol_left << 8) | vol_right);
}
