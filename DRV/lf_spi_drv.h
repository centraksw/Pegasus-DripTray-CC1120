#ifndef __LF_H
#define __LF_H

VOID LF_SPI_DRV_Initialize();
BYTE LF_SPI_DRV_ReadByte(BYTE addr);
VOID LF_SPI_DRV_WriteByte(BYTE addr, BYTE value);
VOID LF_SPI_DRV_DirectCommand(BYTE addr);

BYTE LF_SPI_DRV_GetAntennaDamping();
BYTE LF_SPI_DRV_GetAntenna_GainReduction();

#endif