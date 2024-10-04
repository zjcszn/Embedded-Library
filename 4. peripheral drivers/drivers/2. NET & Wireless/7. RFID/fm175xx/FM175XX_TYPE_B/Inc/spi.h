#ifndef	SPI_H
#define SPI_H
extern unsigned char SPIRead(unsigned char addr);				//SPI������
extern void SPIWrite(unsigned char add,unsigned char wrdata);	//SPIд����
extern void SPIRead_Sequence(unsigned char sequence_length,unsigned char addr,unsigned char *reg_value);
extern void SPIWrite_Sequence(unsigned char sequence_length,unsigned char addr,unsigned char *reg_value);

extern unsigned char SPI_Init();

#endif