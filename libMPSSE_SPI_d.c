/*!
 * \file libMPSSE_SPI_d.c
 *
 * \author FTDI, H.W.
 * \date 20140105
 *
 * Copyright © 2011 Future Technology Devices International Limited
 * Company Confidential
 * 
 * This .c file has been created based on sample-dynamic.c and libMPSSE_SPI.h
 *
 * Project: libMPSSE
 * Module: SPI
 *
 * Rivision History:
 * 0.1 - initial version, for dynamic library loading (by H.W.)
 */

/******************************************************************************/
/* 							 Include files									  */
/******************************************************************************/
/* Standard C libraries */
#include<stdio.h>
#include<stdlib.h>

/* OS specific libraries */
#ifdef _WIN32
	#include<windows.h>
#endif

#ifdef __linux
	#include<dlfcn.h>
#endif

/* Include D2XX header*/
#include "ftd2xx.h"

/* Include libMPSSE header */
#include "libMPSSE_SPI_d.h"

/******************************************************************************/
/*						  Macro and type defines							  */
/******************************************************************************/
/* Helper macros */
#ifdef _WIN32
	#define GET_FUN_POINTER	GetProcAddress
#endif

#ifdef __linux
	#define GET_FUN_POINTER	dlsym
#endif

#define CHECK_NULL_RTN(exp, returnCode){if(exp==NULL){printf("%s:%d:%s():  NULL expression \
encountered \n",__FILE__, __LINE__, __FUNCTION__);return returnCode;}else{;}};

#define CHECK_LIB_LOADED {if((int)h_libMPSSE <= 0) return FT_INVALID_HANDLE;};


/******************************************************************************/
/*						  External Function Call Pointers					  */
/******************************************************************************/

typedef FT_STATUS (*pfunc_SPI_GetNumChannels)(uint32 *numChannels);
pfunc_SPI_GetNumChannels p_SPI_GetNumChannels;
typedef FT_STATUS (*pfunc_SPI_GetChannelInfo)(uint32 index, \
	FT_DEVICE_LIST_INFO_NODE *chanInfo);
pfunc_SPI_GetChannelInfo p_SPI_GetChannelInfo;
typedef FT_STATUS (*pfunc_SPI_OpenChannel)(uint32 index, FT_HANDLE *handle);
pfunc_SPI_OpenChannel p_SPI_OpenChannel;
typedef FT_STATUS (*pfunc_SPI_InitChannel)(FT_HANDLE handle, \
	ChannelConfig *config);
pfunc_SPI_InitChannel p_SPI_InitChannel;
typedef FT_STATUS (*pfunc_SPI_CloseChannel)(FT_HANDLE handle);
pfunc_SPI_CloseChannel p_SPI_CloseChannel;
typedef FT_STATUS (*pfunc_SPI_Read)(FT_HANDLE handle, uint8 *buffer, \
	uint32 sizeToTransfer, uint32 *sizeTransfered, uint32 options);
pfunc_SPI_Read p_SPI_Read;
typedef FT_STATUS (*pfunc_SPI_Write)(FT_HANDLE handle, uint8 *buffer, \
	uint32 sizeToTransfer, uint32 *sizeTransfered, uint32 options);
pfunc_SPI_Write p_SPI_Write;
typedef FT_STATUS (*pfunc_SPI_IsBusy)(FT_HANDLE handle, bool *state);
pfunc_SPI_IsBusy p_SPI_IsBusy;

/* additional types and pointers (previously missing */
typedef FT_STATUS (*pfunc_SPI_ReadWrite)(FT_HANDLE handle, uint8 *inBuffer, 
	uint8 *outBuffer, uint32 sizeToTransfer, uint32 *sizeTransferred, 
	uint32 transferOptions);
pfunc_SPI_ReadWrite p_SPI_ReadWrite;
typedef FT_STATUS (*pfunc_SPI_ChangeCS)(FT_HANDLE handle, \
	uint32 configOptions);
pfunc_SPI_ChangeCS p_SPI_ChangeCS;
typedef FT_STATUS (*pfunc_FT_WriteGPIO)(FT_HANDLE handle, \
	uint8 dir, uint8 value);
pfunc_FT_WriteGPIO p_FT_WriteGPIO;
typedef FT_STATUS (*pfunc_FT_ReadGPIO)(FT_HANDLE handle, \
	uint8 *value);
pfunc_FT_ReadGPIO p_FT_ReadGPIO;


#ifdef _WIN32
#ifdef _MSC_VER
	HMODULE h_libMPSSE = NULL;
#else
	HANDLE h_libMPSSE = NULL;
#endif
#endif
#ifdef __linux
	void *h_libMPSSE = NULL;
#endif


/******************************************************************************/
/*						  Wrappers											  */
/******************************************************************************/

FTDI_API FT_STATUS SPI_GetNumChannels(uint32 *numChannels)
{
	CHECK_LIB_LOADED;
	return p_SPI_GetNumChannels(numChannels);
}


FTDI_API FT_STATUS SPI_GetChannelInfo(uint32 index, 
	FT_DEVICE_LIST_INFO_NODE *chanInfo)
{
	CHECK_LIB_LOADED;
	return p_SPI_GetChannelInfo(index, chanInfo);
}


FTDI_API FT_STATUS SPI_OpenChannel(uint32 index, FT_HANDLE *handle)
{
	CHECK_LIB_LOADED;
	return p_SPI_OpenChannel(index, handle);
}


FTDI_API FT_STATUS SPI_InitChannel(FT_HANDLE handle, ChannelConfig *config)
{
	CHECK_LIB_LOADED;
	return p_SPI_InitChannel(handle, config);
}


FTDI_API FT_STATUS SPI_CloseChannel(FT_HANDLE handle)
{
	CHECK_LIB_LOADED;
	return p_SPI_CloseChannel(handle);
}


FTDI_API FT_STATUS SPI_Read(FT_HANDLE handle, uint8 *buffer, 
	uint32 sizeToTransfer, uint32 *sizeTransfered, uint32 options)
{
	CHECK_LIB_LOADED;
	return p_SPI_Read(handle, buffer, sizeToTransfer, sizeTransfered, options);
}


FTDI_API FT_STATUS SPI_Write(FT_HANDLE handle, uint8 *buffer, 
	uint32 sizeToTransfer, uint32 *sizeTransfered, uint32 options)
{
	CHECK_LIB_LOADED;
	return p_SPI_Write(handle, buffer, sizeToTransfer, sizeTransfered, options);
}


FTDI_API FT_STATUS SPI_ReadWrite(FT_HANDLE handle, uint8 *inBuffer, 
	uint8 *outBuffer, uint32 sizeToTransfer, uint32 *sizeTransferred, 
	uint32 transferOptions)
{
	CHECK_LIB_LOADED;
	return p_SPI_ReadWrite(handle, inBuffer, outBuffer, sizeToTransfer, 
		sizeTransferred, transferOptions);
}


FTDI_API FT_STATUS SPI_IsBusy(FT_HANDLE handle, bool *state)
{
	CHECK_LIB_LOADED;
	return p_SPI_IsBusy(handle, state);
}


FTDI_API void Init_libMPSSE(void)
{
	// you don't have to call Init_ or Cleanup_ with dynamic linking
}


FTDI_API void Cleanup_libMPSSE(void)
{
	// you don't have to call Init_ or Cleanup_ with dynamic linking
}


FTDI_API FT_STATUS SPI_ChangeCS(FT_HANDLE handle, uint32 configOptions)
{
	CHECK_LIB_LOADED;
	return p_SPI_ChangeCS(handle, configOptions);
}


FTDI_API FT_STATUS FT_WriteGPIO(FT_HANDLE handle, uint8 dir, uint8 value)
{
	CHECK_LIB_LOADED;
	return p_FT_WriteGPIO(handle, dir, value);
}


FTDI_API FT_STATUS FT_ReadGPIO(FT_HANDLE handle,uint8 *value)
{
	CHECK_LIB_LOADED;
	return p_FT_ReadGPIO(handle, value);
}


/******************************************************************************/
/*						  Library Loading/Unloading							  */
/******************************************************************************/

FTDI_API FT_STATUS Load_libMPSSE(void)
{
	// returns FT_OK or FT_INVALID_HANDLE

	#ifdef _WIN32
		#ifdef _MSC_VER
		//	h_libMPSSE = LoadLibrary(L"libMPSSE.dll");
		//#else
			h_libMPSSE = LoadLibrary("libMPSSE.dll");
		#endif
		if(NULL == h_libMPSSE)
		{
			printf("Failed loading libMPSSE.dll."\
				"\nPlease check if the file exists in the working directory\n");
			return FT_INVALID_HANDLE;
		}
	#endif

	#if __linux
		h_libMPSSE = dlopen("libMPSSE.so",RTLD_LAZY);
		if(!h_libMPSSE)
		{
			printf("Failed loading libMPSSE.so. Please check if the file exists in"\
				"the shared library folder(/usr/lib or /usr/lib64)\n");
			return FT_INVALID_HANDLE;
		}
	#endif
	
	/* init function pointers */
	p_SPI_GetNumChannels = (pfunc_SPI_GetNumChannels)GET_FUN_POINTER(h_libMPSSE\
	, "SPI_GetNumChannels");
	CHECK_NULL_RTN (p_SPI_GetNumChannels, FT_INVALID_HANDLE);
	p_SPI_GetChannelInfo = (pfunc_SPI_GetChannelInfo)GET_FUN_POINTER(h_libMPSSE\
	, "SPI_GetChannelInfo");
	CHECK_NULL_RTN(p_SPI_GetChannelInfo, FT_INVALID_HANDLE);
	p_SPI_OpenChannel = (pfunc_SPI_OpenChannel)GET_FUN_POINTER(h_libMPSSE\
	, "SPI_OpenChannel");
	CHECK_NULL_RTN(p_SPI_OpenChannel, FT_INVALID_HANDLE);	
	p_SPI_InitChannel = (pfunc_SPI_InitChannel)GET_FUN_POINTER(h_libMPSSE\
	, "SPI_InitChannel");
	CHECK_NULL_RTN(p_SPI_InitChannel, FT_INVALID_HANDLE);
	p_SPI_Read = (pfunc_SPI_Read)GET_FUN_POINTER(h_libMPSSE
	, "SPI_Read");
	CHECK_NULL_RTN(p_SPI_Read, FT_INVALID_HANDLE);
	p_SPI_Write = (pfunc_SPI_Write)GET_FUN_POINTER(h_libMPSSE
	, "SPI_Write");
	CHECK_NULL_RTN(p_SPI_Write, FT_INVALID_HANDLE);	
	p_SPI_CloseChannel = (pfunc_SPI_CloseChannel)GET_FUN_POINTER(h_libMPSSE\
	, "SPI_CloseChannel");
	CHECK_NULL_RTN(p_SPI_CloseChannel, FT_INVALID_HANDLE);
	p_SPI_IsBusy = (pfunc_SPI_IsBusy)GET_FUN_POINTER(h_libMPSSE, 
	"SPI_IsBusy");
	CHECK_NULL_RTN(p_SPI_IsBusy, FT_INVALID_HANDLE);
	
	/*init additional function pointers (previously missing)*/
	p_SPI_ReadWrite = (pfunc_SPI_ReadWrite)GET_FUN_POINTER(h_libMPSSE
	, "SPI_ReadWrite");
	CHECK_NULL_RTN(p_SPI_ReadWrite, FT_INVALID_HANDLE);
	p_SPI_ChangeCS = (pfunc_SPI_ChangeCS)GET_FUN_POINTER(h_libMPSSE
	, "SPI_ChangeCS");
	CHECK_NULL_RTN(p_SPI_ChangeCS, FT_INVALID_HANDLE);
	p_FT_WriteGPIO = (pfunc_FT_WriteGPIO)GET_FUN_POINTER(h_libMPSSE
	, "FT_WriteGPIO");
	CHECK_NULL_RTN(p_FT_WriteGPIO, FT_INVALID_HANDLE);
	p_FT_ReadGPIO = (pfunc_FT_ReadGPIO)GET_FUN_POINTER(h_libMPSSE
	, "FT_ReadGPIO");
	CHECK_NULL_RTN(p_FT_ReadGPIO, FT_INVALID_HANDLE);
	
	return FT_OK;		
}


FTDI_API void Unload_libMPSSE(void)
{
	#ifdef _WIN32
		FreeLibrary(h_libMPSSE);
		h_libMPSSE = NULL;
	#endif
	
	#if __linux
		// not tested
		dlclose(h_libMPSSE);
		h_libMPSSE = NULL;
	#endif	
}

