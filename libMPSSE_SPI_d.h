/*!
 * \file libMPSSE_SPI_d.h
 *
 * \author FTDI
 * \date 20110527
 *
 * Copyright © 2011 Future Technology Devices International Limited
 * Company Confidential
 *
 * Project: libMPSSE
 * Module: SPI
 *
 * Rivision History:
 * 0.1 - initial version
 * 0.2 - 20110708 - added FT_ReadGPIO, FT_WriteGPIO & SPI_ChangeCS
 * 0.3 - 20111025 - modified for supporting 64bit linux
 * 0.4 - 20140105 - BY H.W., NOT BY FTDI! - rewritten for dynamic library loading 
 */

#ifndef LIBMPSSE_SPI_D_H
#define LIBMPSSE_SPI_D_H

#ifdef __cplusplus
	extern "C" {
#endif

#include "ftd2xx.h"



#define _MSC_VER

/******************************************************************************/
/*						 		 Helper macros 								  */
/******************************************************************************/

#ifdef _WIN32
	#define CHECK_ERROR(exp) {if(exp==NULL){printf("%s:%d:%s():  NULL \
expression encountered \n",__FILE__, __LINE__, __FUNCTION__);exit(1);}else{;}};
	#define TEST_FOR_ERROR(exp) exp == NULL ? true : false;
	#define REPORT_ERROR(exp) {if(exp==NULL){printf("%s:%d:%s():  NULL \
expression encountered \n",__FILE__, __LINE__, __FUNCTION__);}else{;}};
#endif

#ifdef __linux
	#define CHECK_ERROR(exp) {if(dlerror() != NULL){printf("line %d: ERROR \
dlsym\n",__LINE__);}}
	#define TEST_FOR_ERROR(exp) dlerror() != NULL ? true : false;
	#define REPORT_ERROR(exp){if(dlerror() != NULL){printf("line %d: ERROR \
dlsym\n",__LINE__);}}
#endif

#define APP_CHECK_STATUS(exp) {if(exp!=FT_OK){printf("%s:%d:%s(): status(0x%x) \
!= FT_OK\n",__FILE__, __LINE__, __FUNCTION__,exp);exit(1);}else{;}};
#define APP_TEST_STATUS(exp) (exp!=FT_OK)
#define APP_REPORT_STATUS(exp) {if(exp!=FT_OK){printf("%s:%d:%s(): status(0x%x) \
!= FT_OK\n",__FILE__, __LINE__, __FUNCTION__,exp);}else{;}};

#define CHECK_NULL(exp){if(exp==NULL){printf("%s:%d:%s():  NULL expression \
encountered \n",__FILE__, __LINE__, __FUNCTION__);exit(1);}else{;}};
#define TEST_NULL(exp) (exp==NULL)
#define REPORT_NULL(exp){if(exp==NULL){printf("%s:%d:%s():  NULL expression \
encountered \n",__FILE__, __LINE__, __FUNCTION__);}else{;}}; 


/******************************************************************************/
/*								Macro defines								  */
/******************************************************************************/

#ifdef _MSC_VER
	#define FTDI_API extern "C"
#else
	#define FTDI_API
#endif

/* Bit definitions of the transferOptions parameter in SPI_Read, SPI_Write & SPI_Transfer  */

/* transferOptions-Bit0: If this bit is 0 then it means that the transfer size provided is in bytes */
#define	SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES		0x00000000
/* transferOptions-Bit0: If this bit is 1 then it means that the transfer size provided is in bytes */
#define	SPI_TRANSFER_OPTIONS_SIZE_IN_BITS		0x00000001
/* transferOptions-Bit1: if BIT1 is 1 then CHIP_SELECT line will be enables at start of transfer */
#define	SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE		0x00000002
/* transferOptions-Bit2: if BIT2 is 1 then CHIP_SELECT line will be disabled at end of transfer */
#define SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE		0x00000004


/* Bit defination of the Options member of configOptions structure*/
#define SPI_CONFIG_OPTION_MODE_MASK		0x00000003
#define SPI_CONFIG_OPTION_MODE0			0x00000000
#define SPI_CONFIG_OPTION_MODE1			0x00000001
#define SPI_CONFIG_OPTION_MODE2			0x00000002
#define SPI_CONFIG_OPTION_MODE3			0x00000003

#define SPI_CONFIG_OPTION_CS_MASK		0x0000001C		/*111 00*/
#define SPI_CONFIG_OPTION_CS_DBUS3		0x00000000		/*000 00*/
#define SPI_CONFIG_OPTION_CS_DBUS4		0x00000004		/*001 00*/
#define SPI_CONFIG_OPTION_CS_DBUS5		0x00000008		/*010 00*/
#define SPI_CONFIG_OPTION_CS_DBUS6		0x0000000C		/*011 00*/
#define SPI_CONFIG_OPTION_CS_DBUS7		0x00000010		/*100 00*/

#define SPI_CONFIG_OPTION_CS_ACTIVELOW	0x00000020


/******************************************************************************/
/*								Type defines								  */
/******************************************************************************/

typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned long long uint64;

typedef signed char   int8;
typedef signed short  int16;
typedef signed long long int64;

#ifndef _MSC_VER
  typedef unsigned char	bool;
#endif

#ifdef __x86_64__  
/*20111025: 64bit linux doesn't work is uint32 is unsigned long*/
	typedef unsigned int   uint32;
	typedef signed int   int32;
#else
	typedef unsigned long   uint32;
	typedef signed long   int32;
#endif


typedef struct ChannelConfig_t
{
	uint32	ClockRate; 

	uint8	LatencyTimer; 

	uint32	configOptions;	/*This member provides a way to enable/disable features
	specific to the protocol that are implemented in the chip
	BIT1-0=CPOL-CPHA:	00 - MODE0 - data captured on rising edge, propagated on falling
 						01 - MODE1 - data captured on falling edge, propagated on rising
 						10 - MODE2 - data captured on falling edge, propagated on rising
 						11 - MODE3 - data captured on rising edge, propagated on falling
	BIT4-BIT2: 000 - A/B/C/D_DBUS3=ChipSelect
			 : 001 - A/B/C/D_DBUS4=ChipSelect
 			 : 010 - A/B/C/D_DBUS5=ChipSelect
 			 : 011 - A/B/C/D_DBUS6=ChipSelect
 			 : 100 - A/B/C/D_DBUS7=ChipSelect
 	BIT5: ChipSelect is active high if this bit is 0
	BIT6 -BIT31		: Reserved
	*/

	uint32		Pin;/*BIT7   -BIT0:   Initial direction of the pins	*/
					/*BIT15 -BIT8:   Initial values of the pins		*/
					/*BIT23 -BIT16: Final direction of the pins		*/
					/*BIT31 -BIT24: Final values of the pins		*/
	uint16		reserved;
}ChannelConfig;


/******************************************************************************/
/*								Function declarations						  */
/******************************************************************************/

// wrapper functions
FTDI_API FT_STATUS SPI_GetNumChannels(uint32 *numChannels);
FTDI_API FT_STATUS SPI_GetChannelInfo(uint32 index, 
	FT_DEVICE_LIST_INFO_NODE *chanInfo);
FTDI_API FT_STATUS SPI_OpenChannel(uint32 index, FT_HANDLE *handle);
FTDI_API FT_STATUS SPI_InitChannel(FT_HANDLE handle, ChannelConfig *config);
FTDI_API FT_STATUS SPI_CloseChannel(FT_HANDLE handle);
FTDI_API FT_STATUS SPI_Read(FT_HANDLE handle, uint8 *buffer, 
	uint32 sizeToTransfer, uint32 *sizeTransfered, uint32 options);
FTDI_API FT_STATUS SPI_Write(FT_HANDLE handle, uint8 *buffer, 
	uint32 sizeToTransfer, uint32 *sizeTransfered, uint32 options);
FTDI_API FT_STATUS SPI_ReadWrite(FT_HANDLE handle, uint8 *inBuffer, 
	uint8 *outBuffer, uint32 sizeToTransfer, uint32 *sizeTransferred, 
	uint32 transferOptions);
FTDI_API FT_STATUS SPI_IsBusy(FT_HANDLE handle, bool *state);
FTDI_API void Init_libMPSSE(void); // you don't have to call Init_ or Cleanup_ with dynamic linking
FTDI_API void Cleanup_libMPSSE(void);
FTDI_API FT_STATUS SPI_ChangeCS(FT_HANDLE handle, uint32 configOptions);
FTDI_API FT_STATUS FT_WriteGPIO(FT_HANDLE handle, uint8 dir, uint8 value);
FTDI_API FT_STATUS FT_ReadGPIO(FT_HANDLE handle,uint8 *value);

// additional functions for DLL handling
FTDI_API FT_STATUS Load_libMPSSE(void); // returns FT_OK or FT_INVALID_HANDLE
FTDI_API void Unload_libMPSSE(void);


/******************************************************************************/

#ifdef __cplusplus
}  // extern "C"
#endif

#endif	/*LIBMPSSE_SPI_D_H*/

