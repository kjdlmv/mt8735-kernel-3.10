#ifndef CX20810_CONFIG_H_
#define CX20810_CONFIG_H_

enum
{
    CX20810_NORMAL_MODE = 0,
    CX20810_NORMAL_MODE_SIMPLE,
    CX20810_NIRMAL_MODE_CODEC3,
    CX20810_NIRMAL_MODE_CODEC3_SIMPLE,
    CX20810_96K_16BIT_MODE,
    CX20810_48K_16BIT_MODE,
};

const char codec_config_param_normal_mode[]=
{
    0x0F,0x03, //RST 
    0x0F,0x03, //repeat write is let chip has more time to RST 
    0x0F,0x03,  
    0x0F,0x03,
 
    0x0F,0x00,//release reset
 
    0x78,0x39, //*PLLEN = 1 ABIASEN IOBUFEN REFIMP = 11 3KR
    0x78,0x39, //*PLLEN = 1 ABIASEN IOBUFEN REFIMP = 11 3KR
    0x78,0x39, //*PLLEN = 1 ABIASEN IOBUFEN REFIMP = 11 3KR
    0x78,0x39, //*PLLEN = 1 ABIASEN IOBUFEN REFIMP = 11 3KR
    0x78,0x39, //*PLLEN = 1 ABIASEN IOBUFEN REFIMP = 11 3KR
 
    0x78,0x79, //*PLLEN = 1 ABIASEN IOBUFEN REFIMP = 11 3KR
    0x78,0x79, //*PLLEN = 1 ABIASEN IOBUFEN REFIMP = 11 3KR
    0x78,0x79, //*PLLEN = 1 ABIASEN IOBUFEN REFIMP = 11 3KR
    0x78,0x79, //*PLLEN = 1 ABIASEN IOBUFEN REFIMP = 11 3KR
    0x78,0x79, //*PLLEN = 1 ABIASEN IOBUFEN REFIMP = 11 3KR
 
    0x7A,0x01,
    0x01,0x01,
	
    // Analog ADC Control
    // MIcIn PGA A0,A7,AE,B5 [5:4] ctrl_rcm , [1] enable [3] mute [7] bypass
    // ģ�ⲿ�ֵ�Դ
    0xA0,0x07,//ADC bias EN
    0xA7,0x07,
    0xAE,0x07,
    0xB5,0x07,
	
    // mic pga ����
    // 4ͨ��¼������ 
    0xBC,0x3C,// 0x28 20dB 0x34 26dB
    0xBD,0x3C,
    0xBE,0x3C,
    0xBF,0x3C,

// PLL config
    0x08,0x00,// disable MCLK
    0x09,0x47,// I2S TX Bit Clock
	0x0a,0x0B,
    0x0a,0x8B,
	0x0C,0x0A,// Clocks gated 
	0x0d,0x08,
	0x0d,0x0C,
    0x60,0x00,// reset and Disable PLL1
    0x61,0x2f,// 23+1=24, 24*1.024MHz = 24.276MHz 
    0x62,0x00,// divin = 0+1
    0x63,0x01,// ENABLE INT MODE 
    //0x66,0x80,
    //{0x68,0x0},
    //{0x69,0x0},
    0x60,0x03,//delay for PLL locked
    0x60,0x03,
    0x60,0x03,
    0x60,0x03,
    0x60,0x03,
	0x67,0x02,
    // end PLL config*/

	//PAD����
	0x80,0x00,// MCLK disable
    0x83,0x0F,// LRCLK BCLK input, data1 data2 output
 

    //I2S CFG
    0x30,0x14,//14 24bit 0a 16bit
    0x31,0x07,//frame (n+1)*8 bit 32+32=64
    0x32,0x07,//
    0x33,0x00,//sys width 32 clk
    0x34,0x00,
    0x35,0xA2,// TX left justified i2s1+i2s2 //0xa2 bryan
    //0x36,0x00,// config for right justified ignored.
    //0x37,0x00,// RX left justified.
    //0x38,0x00,// config for right justified ignored.
    //0x39,0x00, // ADC12 0n DATA1.ADC34 On DATA2  -->chged by ysx
    //0x3A,0x00,//Slot1
    //0x3B,0x00,//slot2
    //0x3C,0x00,//slot3
    //0x3D,0x00,//slot4
 




    0x0F,0x01, //RST,clears DSP,audio data interface values 
    0x0F,0x01, //repeat write is let chip has more time to RST 
    0x0F,0x01,  
    0x0F,0x01,  

    0x0F,0x00,//clear RST 
    //0x08,0x30,// enable MCLK to chip  
    //0x08,0x38,
    //0x09,0x03,
 
    0x10,0x00, // Disable all ADC clocks
    0x11,0x10, // Disable all ADC and Mixer
    0x10,0x1F, // Enable all ADC clocks and ADC digital
    0x11,0x1F, // Enable all ADC and set 16k sample rate
    0x16,0x00,// Use DC Filter for ADCs
    0x10,0x5F, // Enable all ADC clocks, ADC digital and ADC Mic Clock Gate

};

const char codec_config_param_normal_mode_simple[]=
{
    // mic pga ����
    // 4ͨ��¼������
    0xBC,0x28,// 0x28 20dB 0x34 26dB
    0xBD,0x28,
    0xBE,0x28,
    0xBF,0x28,

    0x60,0x04,
    0x66,0x00,
    0x67,0x02,

    //PAD����
    0x80,0x03,// MCLK Ϊ����
    0x83,0x0F,// LRCLK BCLK Ϊ����ţ�TX1 TX2Ϊ�����

    //0x08,0x30,// MCLK ��Ϊ����
    //0x08,0x38,// MCLK divisor ��Ч
    0x08,0x20,// MCLK ��Ϊ���� 12.288MHz
    0x09,0x03,// ѡMCLK��ΪPLL����Դ
    0x0a,0x0b,
    0x0a,0x8b,
    0x0C,0x0A,// RT clock disable, TX clock enable, enable clock to ADC3/4

    // I2S
    //0x30,0x0A,// Tx sample size:16bit, Normal mode
    0x30,0x14,// Tx sample size:24bit, Normal mode
    0x35,0xA2,// left justified, enable I2S-1 and I2S-2

    0x10,0x00,
    0x11,0x00,
    0x10,0x1F,
    0x11,0x1F,// ADC 96k, enables all ADCs
    0x16,0x00,
    0x10,0x5F,

};

const char codec3_config_param_normal_mode[]=
{
    // POWER
    0x78,0x39, //*PLLEN = 1 ABIASEN IOBUFEN REFIMP = 11 3KR
    0x78,0x39, //*PLLEN = 1 ABIASEN IOBUFEN REFIMP = 11 3KR
    0x78,0x39, //*PLLEN = 1 ABIASEN IOBUFEN REFIMP = 11 3KR
    0x78,0x39, //*PLLEN = 1 ABIASEN IOBUFEN REFIMP = 11 3KR
    0x78,0x39, //*PLLEN = 1 ABIASEN IOBUFEN REFIMP = 11 3KR

    0x78,0x79, //*PLLEN = 1 ABIASEN IOBUFEN REFIMP = 11 3KR
    0x78,0x79, //*PLLEN = 1 ABIASEN IOBUFEN REFIMP = 11 3KR
    0x78,0x79, //*PLLEN = 1 ABIASEN IOBUFEN REFIMP = 11 3KR
    0x78,0x79, //*PLLEN = 1 ABIASEN IOBUFEN REFIMP = 11 3KR
    0x78,0x79, //*PLLEN = 1 ABIASEN IOBUFEN REFIMP = 11 3KR

    0x7A,0x01,
    0x01,0x01,

    // Analog ADC Control
    // MIcIn PGA A0,A7,AE,B5 [5:4] ctrl_rcm , [1] enable [3] mute [7] bypass
    // ģ�ⲿ�ֵ�Դ
    0xA0,0x07,//ADC bias EN
    0xA7,0x07,
    0xAE,0x07,
    0xB5,0x07,

    // mic pga ����
    // 4ͨ��¼������
    0xBC,0x06,// 0x28 20dB 0x34 26dB
    0xBD,0x06,
    0xBE,0x0C,
    0xBF,0x14,

    0x60,0x04,
    0x66,0x00,
    0x67,0x02,

    //PAD����
    0x80,0x03,// MCLK Ϊ����
    0x83,0x0F,// LRCLK BCLK Ϊ����ţ�TX1 TX2Ϊ�����

    //0x08,0x30,// MCLK ��Ϊ����
    //0x08,0x38,// MCLK divisor ��Ч
    0x08,0x20,// MCLK ��Ϊ���� 12.288MHz
    0x09,0x03,// ѡMCLK��ΪPLL����Դ
    0x0a,0x0b,
    0x0a,0x8b,
    0x0C,0x0A,// RT clock disable, TX clock enable, enable clock to ADC3/4

    // I2S
    //0x30,0x0A,// Tx sample size:16bit, Normal mode
    0x30,0x14,// Tx sample size:24bit, Normal mode
    0x35,0xA2,// left justified, enable I2S-1 and I2S-2

    0x10,0x00,
    0x11,0x00,
    0x10,0x1F,
    0x11,0x1F,// ADC 96k, enables all ADCs
    0x16,0x00,
    0x10,0x5F,

};

const char codec3_config_param_normal_mode_simple[]=
{
    // mic pga ����
    // 4ͨ��¼������
    0xBC,0x28,// 0x28 20dB 0x34 26dB
    0xBD,0x28,
    0xBE,0x28,
    0xBF,0x28,

    0x60,0x04,
    0x66,0x00,
    0x67,0x02,

    //PAD����
    0x80,0x03,// MCLK Ϊ����
    0x83,0x0F,// LRCLK BCLK Ϊ����ţ�TX1 TX2Ϊ�����

    //0x08,0x30,// MCLK ��Ϊ����
    //0x08,0x38,// MCLK divisor ��Ч
    0x08,0x20,// MCLK ��Ϊ���� 12.288MHz
    0x09,0x03,// ѡMCLK��ΪPLL����Դ
    0x0a,0x0b,
    0x0a,0x8b,
    0x0C,0x0A,// RT clock disable, TX clock enable, enable clock to ADC3/4

    // I2S
    //0x30,0x0A,// Tx sample size:16bit, Normal mode
    0x30,0x14,// Tx sample size:24bit, Normal mode
    0x35,0xA2,// left justified, enable I2S-1 and I2S-2

    0x10,0x00,
    0x11,0x00,
    0x10,0x1F,
    0x11,0x1F,// ADC 96k, enables all ADCs
    0x16,0x00,
    0x10,0x5F,
};

const char codec_config_param_48k_16bit_mode[]=
{
    // mic pga ����
    // 4ͨ��¼������
    0xBC,29 << 1,// 0x28 20dB 0x34 26dB
    0xBD,29 << 1,
    0xBE,29 << 1,
    0xBF,29 << 1,

    0x60,0x04,
    0x66,0x00,
    0x67,0x02,

    //PAD����
    0x80,0x03,// MCLK Ϊ����
    0x83,0x0F,// LRCLK BCLK Ϊ����ţ�TX1 TX2Ϊ�����

    //0x08,0x30,// MCLK ��Ϊ����
    //0x08,0x38,// MCLK divisor ��Ч
    0x08,0x20,// MCLK ��Ϊ���� 12.288MHz
    0x09,0x03,//ѡMCLK��ΪPLL����Դ
    0x0a,0x03,
    0x0a,0x83,
    0x0C,0x0A,// RT clock disable, TX clock enable, enable clock to ADC3/4

    // I2S
    0x30,0x0A,// Tx sample size:16bit, Normal mode
    //0x30,0x14,// Tx sample size:24bit, Normal mode
    0x35,0xA2,// left justified, enable I2S-1 and I2S-2

    0x10,0x00,
    0x11,0x00,
    0x10,0x1F,
    0x11,0x4F,// ADC 96k, enables all ADCs
    0x16,0x00,
    0x10,0x5F,
};

const char codec_config_param_96k_16bit_mode[]=
{
    // mic pga ����
    // 4ͨ��¼������
    0xBC,29 << 1,// 0x28 20dB 0x34 26dB
    0xBD,29 << 1,
    0xBE,29 << 1,
    0xBF,29 << 1,

    0x60,0x04,
    0x66,0x00,
    0x67,0x02,

    //PAD����
    0x80,0x03,// MCLK Ϊ����
    0x83,0x0F,// LRCLK BCLK Ϊ����ţ�TX1 TX2Ϊ�����

    //0x08,0x30,// MCLK ��Ϊ����
    //0x08,0x38,// MCLK divisor ��Ч
    0x08,0x20,// MCLK ��Ϊ���� 12.288MHz
    0x09,0x03,// ѡMCLK��ΪPLL����Դ
    0x0a,0x01,
    0x0a,0x81,
    0x0C,0x0A,// RT clock disable, TX clock enable, enable clock to ADC3/4

    // I2S
    0x30,0x0A,// Tx sample size:16bit, Normal mode
    //0x30,0x14,// Tx sample size:24bit, Normal mode
    0x35,0xA2,// left justified, enable I2S-1 and I2S-2

    0x10,0x00,
    0x11,0x00,
    0x10,0x1F,
    0x11,0x5F,// ADC 96k, enables all ADCs
    0x16,0x00,
    0x10,0x5F,
};

#endif /* CX20810_CONFIG_H_ */