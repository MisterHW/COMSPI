/* Standard C++ libraries */
#include <string>

/* Standard C libraries */
// #include <stdio.h>
// #include <stdlib.h>
#include <cstdio>
#include <cstdlib>

/* OS specific libraries */
#include <windows.h>

using namespace std;
	
//extern "C" {
	/* Include D2XX header*/
	#include "ftd2xx.h"

	/* Include libMPSSE header */
	#include "libMPSSE_spi_d.h"
//}

#define ID_TIMER1 1

DWORD CONST WM_DATA_LISTENER = (WM_APP + 1);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

char AppTitle[] = "FT2232H  COM <> SPI Bridge";
std::string windowText;

int COM_PORT 		= 0;
int FT_CHANNEL_NUM 	= -1;
int FT_ID 			= 0;
int FT_LOC_ID 		= 0;
int SPI_MODE 		= 0;
int SPI_BYTES_TO_READ = 1;
int SPI_CLOCK_RATE 	= 5000;
int SPI_CSEL_ACTIVE = 0; // 0: active low; 1: active high

FT_HANDLE FT_CHANNEL_HANDLE;

typedef enum {
	OM_INVALID = 0,
	OM_CHANNEL = 1,
	OM_ID = 2
} TOpenMode;
TOpenMode openMode = OM_INVALID;

unsigned long totalBytesTransferred = 0;
unsigned long totalBytesTransferredLast = 0;
HANDLE HCOM;
HWND consoleHandle; // console window handle
HWND hwnd; // main window handle
HANDLE H_DATA_LISTENER_THREAD = INVALID_HANDLE_VALUE;

typedef struct {
	HANDLE MessageThreadHandle;
	DWORD MessageThreadID;
	HWND MainWindow;
	HANDLE OverlappedReadEvent;
} TCOMListenerThreadData;
TCOMListenerThreadData COMListenerThreadData;

typedef enum {
	CE_NEW_DATA,
	CE_COM_ERROR
} TCOMListenerThreadEvents;

typedef struct {
	unsigned int nBytes;
	char data[0];
} TDataBlock;

OVERLAPPED OVL_WRITE;



void reportChannelInfo(int numChannel)
{
	FT_STATUS status;
	FT_DEVICE_LIST_INFO_NODE devList;

	status = SPI_GetChannelInfo(numChannel,&devList);
	if(APP_TEST_STATUS(status))
	{	APP_REPORT_STATUS(status);
		return;
	}
	
	printf("  Information on channel number %d:\n",numChannel);
	/*print the dev info*/
	printf("    Flags=0x%x\n",devList.Flags); 
	printf("    Type=0x%x\n",devList.Type); 
	printf("    ID=0x%x\n",devList.ID); 
	printf("    LocId=0x%x\n",devList.LocId); 
	printf("    SerialNumber=%s\n",devList.SerialNumber); 
	printf("    Description=%s\n",devList.Description); 
	printf("    ftHandle=0x%x\n",devList.ftHandle);/*is 0 unless open*/
}

void enumChannels(void)
{
	FT_STATUS status;
	uint32 channels;

	status = SPI_GetNumChannels(&channels);
	if(APP_TEST_STATUS(status))
	{	APP_REPORT_STATUS(status);
		return;
	}
	
	printf("Number of available SPI channels = %d\n",channels);
	if(channels>0)
	{
		for(int i=0; i < channels; i++)
		{
			reportChannelInfo(i);
		}
	}
}


HWND GetConsoleHwnd(void)
{
   #define MY_BUFSIZE 1024 // Buffer size for console window titles.
   HWND hwndFound;         // This is what is returned to the caller.
   char pszNewWindowTitle[MY_BUFSIZE]; // Contains fabricated
									   // WindowTitle.
   char pszOldWindowTitle[MY_BUFSIZE]; // Contains original
									   // WindowTitle.

   // Fetch current window title.
   GetConsoleTitle(pszOldWindowTitle, MY_BUFSIZE);

   // Format a "unique" NewWindowTitle.
   wsprintf(pszNewWindowTitle,"%d/%d",
			   GetTickCount(),
			   GetCurrentProcessId());

   // Change current window title.
   SetConsoleTitle(pszNewWindowTitle);

   // Ensure window title has been updated.
   Sleep(40);

   // Look for NewWindowTitle.
   hwndFound=FindWindow(NULL, pszNewWindowTitle);

   // Restore original window title.
   SetConsoleTitle(pszOldWindowTitle);

   return(hwndFound);
}


HWND CreateMainWindow(HINSTANCE hInst, int nCmdShow)
{
	HWND hwnd;
	WNDCLASS wc;

	wc.style=CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc=WindowProc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hInstance=hInst;
	wc.hIcon=LoadIcon(NULL,IDI_WINLOGO);
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground=(HBRUSH)COLOR_WINDOWFRAME;
	wc.lpszMenuName=NULL;
	wc.lpszClassName=AppTitle;

	if (!RegisterClass(&wc))
		return 0;

	hwnd = CreateWindow(AppTitle,AppTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,CW_USEDEFAULT,450,300,
		NULL,NULL,hInst,NULL);

	if (!hwnd)
		return 0;
		
	ShowWindow(hwnd,nCmdShow);
	
	return hwnd;
}
   
   
bool OpenSPIChannel(TOpenMode mode, int chan, int id, int locid, FT_HANDLE& handle)
{
	FT_STATUS status;
	ChannelConfig channelConf;
	uint32 channels;
	FT_DEVICE_LIST_INFO_NODE devList;
	
	channelConf.ClockRate 		= SPI_CLOCK_RATE;
	channelConf.LatencyTimer 	= 255;
	
	int __SPI_MODE;
	switch (SPI_MODE)
	{
		case 0: __SPI_MODE = SPI_CONFIG_OPTION_MODE0; break;
		case 1: __SPI_MODE = SPI_CONFIG_OPTION_MODE1; break;
		case 2: __SPI_MODE = SPI_CONFIG_OPTION_MODE2; break;
		default:
		case 3: __SPI_MODE = SPI_CONFIG_OPTION_MODE3; 
	}
	
	int __SPI_CSEL_MODE;
	if(SPI_CSEL_ACTIVE == 1)
 	{	
		__SPI_CSEL_MODE = 0;
	} else {
		__SPI_CSEL_MODE = SPI_CONFIG_OPTION_CS_ACTIVELOW;
	}
		
	channelConf.configOptions	= __SPI_MODE // MODE1 and MODE3 (sample on trailing edge) not supported in LibMPSSE_SPI?
								  | SPI_CONFIG_OPTION_CS_DBUS3
								  | __SPI_CSEL_MODE;
	channelConf.Pin 			= 0x00000000;/*FinalVal-FinalDir-InitVal-InitDir (for dir 0=in, 1=out)*/ 

	status = SPI_GetNumChannels(&channels);
	if(APP_TEST_STATUS(status)){ APP_REPORT_STATUS(status); return false;}
	
	
	for( int i = 0; i < channels; i++)
	{
		status = SPI_GetChannelInfo(i, &devList);
		if(APP_TEST_STATUS(status)){ APP_REPORT_STATUS(status); return false;}
		
		if( ((mode == OM_CHANNEL) && (i == chan)) || 
		    ((mode == OM_ID) && ((id == devList.ID)&&(locid == devList.LocId))) )
		{
			status = SPI_OpenChannel(i, &handle);
			if(APP_TEST_STATUS(status)){ APP_REPORT_STATUS(status); return false;}
				
			status = SPI_InitChannel(handle, &channelConf);
			if(APP_TEST_STATUS(status)){ APP_REPORT_STATUS(status); return false;}

			printf("SPI Channel %d opened.\n", i);
			
			return true;
		}
	}
	
	printf("Error: SPI Channel / ID not found.\n");
	return false;
}


void StrobeCSDemo()
{						
/* Strobe Chip Select */
	uint32 sizeToTransfer=0;
	uint32 sizeTransferred=0;
	uint8 buffer[16];
	FT_STATUS status;
	
	status = SPI_Write(FT_CHANNEL_HANDLE, buffer, sizeToTransfer, &sizeTransferred, 
		SPI_TRANSFER_OPTIONS_SIZE_IN_BITS|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);
		
	if(APP_TEST_STATUS(status))
		APP_REPORT_STATUS(status);
		
#ifndef __linux__
	Sleep(50);
#endif
	sizeToTransfer=0;
	sizeTransferred=0;
	status = SPI_Write(FT_CHANNEL_HANDLE, buffer, sizeToTransfer, &sizeTransferred, 
		SPI_TRANSFER_OPTIONS_SIZE_IN_BITS|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
		
	if(APP_TEST_STATUS(status))
		APP_REPORT_STATUS(status);
}

DWORD WINAPI COMListenerThread(LPVOID lpParam)
{
	char buf[2048]; 
	DWORD bytes_read;
	TCOMListenerThreadData* data = (TCOMListenerThreadData*) lpParam;
	if(!data) return 0;
	
	// set 100ms timeout for ReadFile(HCOM, ...)
	COMMTIMEOUTS cto;
	GetCommTimeouts(HCOM, &cto);
	cto.ReadIntervalTimeout = 100; // ms - no new data timeout
	cto.ReadTotalTimeoutConstant    = 0;
    cto.ReadTotalTimeoutMultiplier  = 0;
	cto.WriteTotalTimeoutMultiplier = 0;
	cto.WriteTotalTimeoutConstant   = 0;
	SetCommTimeouts(HCOM, &cto);
	
	OVERLAPPED OVL_READ;
	memset(&OVL_READ,0,sizeof(OVERLAPPED));
	OVL_READ.hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);	
	(*data).OverlappedReadEvent = OVL_READ.hEvent; 

	while(1)
	{
		// try to read data
		ReadFile(HCOM, buf, sizeof(buf), &bytes_read, &OVL_READ);
		if (WaitForSingleObject(OVL_READ.hEvent,75)==WAIT_OBJECT_0)
		{
			GetOverlappedResult(HCOM,&OVL_READ,&bytes_read,TRUE);
		} else {
			DWORD gle = GetLastError();
			if (gle!=ERROR_IO_PENDING)
				printf("Error: ReadData overlapped I/O error %d\n", gle);
		}
		if(bytes_read == 0) continue;
		
		// allocate buffer structure
		HLOCAL hmem = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, bytes_read + sizeof(unsigned int));
		TDataBlock* mem = (TDataBlock*) hmem; // just to document that the original type was HLOCAL
		(*mem).nBytes = bytes_read;
		memcpy((*mem).data, buf, bytes_read);
		
		// pass buffer structure to main thread
		// printf("Listener.post %d byte(s)\n", bytes_read);
		PostThreadMessage(	
			(*data).MessageThreadID, 
			WM_DATA_LISTENER,
			CE_NEW_DATA,
			(LPARAM) hmem);
			
		/* demo loopback
		WriteFile(HCOM, 
			buf,
			bytes_read,
			&bytes_read,
			NULL);
			
		FlushFileBuffers(HCOM);
		
		*/
	}
}


HANDLE WINAPI CreateListenerThread()
{
	// init payload data on demand
	COMListenerThreadData.MessageThreadHandle = GetCurrentThread();
	COMListenerThreadData.MessageThreadID = GetCurrentThreadId();
	COMListenerThreadData.MainWindow = hwnd;
	
	// create listener thread
	return CreateThread( 
				NULL, 				//  _In_opt_   LPSECURITY_ATTRIBUTES lpThreadAttributes (NULL : not inheritable)
				0, 					//  _In_       SIZE_T dwStackSize (0 : default number of pages)
				COMListenerThread, 	//  _In_       LPTHREAD_START_ROUTINE lpStartAddress 
				&COMListenerThreadData, //  _In_opt_   LPVOID lpParameter payload
				CREATE_SUSPENDED, 	//  _In_       DWORD dwCreationFlags
				NULL); 				//  _Out_opt_  LPDWORD lpThreadId
}


int WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{	
	// window handling
	//
	consoleHandle = GetConsoleHwnd(); // console window handle
	hwnd = CreateMainWindow(hInst, nCmdShow); // main window handle
	FT_STATUS status;
	MSG msg; // msg variable for window loop
	

	
	// parse command line arguments
	//
	
	int cmdLineLen = strlen(lpCmdLine);
	char CmdLine[cmdLineLen];
	strcpy(CmdLine, lpCmdLine);
	char* keyItem = NULL;
	
	char* item = strtok(CmdLine, " ");
	char* ep; 
	while (item != NULL)
	{
		if(item[0] == '-')
		{
			keyItem = item;
		} else {
			if(keyItem != NULL)
			{
				if(0==strcmp(keyItem, "-COM"))   COM_PORT = atoi(item);
				if(0==strcmp(keyItem, "-MODE"))  SPI_MODE = atoi(item);
				if(0==strcmp(keyItem, "-CSEL"))  SPI_CSEL_ACTIVE = atoi(item);
				if(0==strcmp(keyItem, "-READ"))  SPI_BYTES_TO_READ = atoi(item);
				if(0==strcmp(keyItem, "-CLK"))   SPI_CLOCK_RATE = atoi(item);
				if(0==strcmp(keyItem, "-CHAN"))  FT_CHANNEL_NUM = atoi(item);
				if(0==strcmp(keyItem, "-ID"))    FT_ID = strtol(item, &ep, 0);
				if(0==strcmp(keyItem, "-LOCID")) FT_LOC_ID = strtol(item, &ep, 0);
				
				keyItem = NULL; // only use first token after a key
			}
		}
		item = strtok (NULL, " ");
	}
	
	// perform validity checks
	//
 
	bool parametersValid = true;
	
	if(cmdLineLen == 0) {
	
		windowText += "command line syntax examples:\n"\
			"  comspi -COM <port no> -CHAN <chan no>\n"\
			"  comspi -COM <port no> -ID <id> -LOCID <locid>\n"\
			"  other params: -MODE <SPI mode=3> -CLK <SPI clock=5000> -READ <no. bytes=1>\n\n"
			"\nError: no parameters specified.\n";
		parametersValid = false;
	} else 
	{
		if(COM_PORT == 0) {
			windowText += "COM Port not specified.\n";
			parametersValid = false;
		}
		
		if(FT_CHANNEL_NUM != -1) openMode = OM_CHANNEL;
		if((FT_ID != 0) && (FT_LOC_ID != 0)) openMode = OM_ID;
		
		if(openMode == OM_INVALID)  {
			windowText += "Channel / ID+LOC_ID not specified.\n";
			parametersValid = false;
		}
	}
	
	// try to init FTDI USB device. If parameters invalid, try to enum FTDI ports.
	//
	
	bool DLL_LOAD_SUCCESS = (FT_OK == Load_libMPSSE());
	if(!DLL_LOAD_SUCCESS)
	{
		windowText += "Error: Failed to load library libMPSSE.\n";
	} 
	
	if (DLL_LOAD_SUCCESS && !parametersValid)
	{
		enumChannels();
	}
	
	COMListenerThreadData.OverlappedReadEvent = INVALID_HANDLE_VALUE;
	
	if(DLL_LOAD_SUCCESS && parametersValid)
	{
		// try to open COM port
		//
		char buf[32];
		sprintf(buf, "\\\\.\\COM%d", COM_PORT);
			
		HCOM = CreateFile(buf, GENERIC_READ | GENERIC_WRITE, 0, NULL, 
  		       		       OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		printf("COM file handle = 0x%x\n", HCOM);
		
		if((int)HCOM <= 0)
		{
			windowText += "Error: cannot open ";
			windowText += buf;
			windowText += "\n";
			parametersValid = false;
		} else {
			windowText += "Listening on ";
			windowText += buf;
			windowText += "\n";
			
			PurgeComm(HCOM, PURGE_RXABORT | PURGE_TXABORT | PURGE_RXCLEAR | PURGE_TXCLEAR);
			
			DCB config;
			if((GetCommState(HCOM, &config) == 0))
			{
				printf("Get configuration port has a problem.");
				return FALSE;
			}

			config.BaudRate = 9600;
			config.StopBits = ONESTOPBIT;
			config.Parity   = NOPARITY; 
			config.ByteSize = 8;
			config.fDtrControl = 0;
			config.fRtsControl = 0;

			if (!SetCommState(HCOM, &config))
			{
				printf( "Failed to Set Comm State Reason: %d\n",GetLastError());
			}
			
			PurgeComm(HCOM, PURGE_TXCLEAR | PURGE_RXCLEAR);
			
			memset(&OVL_WRITE,0,sizeof(OVERLAPPED));
			OVL_WRITE.hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
			
		}
		
		
		if(parametersValid)
		{		
			// try to open SPI port
			//
			
			if(!OpenSPIChannel(openMode, FT_CHANNEL_NUM, FT_ID, FT_LOC_ID, FT_CHANNEL_HANDLE))
			{
				windowText += "Error: failed to open SPI port.\n";
				parametersValid = false;
			} else {
				printf("SPI device handle = 0x%x\n", FT_CHANNEL_HANDLE);
				// StrobeCSDemo();
				
				if(openMode == OM_CHANNEL)
				{ 
					printf("\n\nDevice on channel %d started successfully.\n", FT_CHANNEL_NUM);
				} else {
					printf("\n\nDevice ID=0x%x LOC_ID=0x%x started successfully.\n", FT_ID, FT_LOC_ID);
				}
								
				// start listener thread
				H_DATA_LISTENER_THREAD = CreateListenerThread();
			}
		}	
			
		if(parametersValid) // all went well
		{	
			// start timer
			SetTimer(hwnd, ID_TIMER1, 100, NULL);
			
			// hide console
			if(consoleHandle != INVALID_HANDLE_VALUE) ShowWindow(consoleHandle, SW_HIDE);
		}
	}	
	// enter main loop, handle other functionality via WM_APP messages
	//
	
	windowText += "\nClose to terminate program.\nDouble-click to toggle console.\n";
	
	if(H_DATA_LISTENER_THREAD != INVALID_HANDLE_VALUE)
	{
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
		SetThreadPriority(H_DATA_LISTENER_THREAD, THREAD_PRIORITY_NORMAL);
		ResumeThread(H_DATA_LISTENER_THREAD);
	}
	
	// main window message loop (wait for WM_QUIT)
	while (GetMessage(&msg,NULL,0,0) > 0)
	{
		TranslateMessage(&msg); // virtual key message handling
		if (msg.message >= WM_USER) // other message handling
			WindowProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
		DispatchMessage(&msg); 
	}
	
	// exit code
	// 
	
	if(H_DATA_LISTENER_THREAD != INVALID_HANDLE_VALUE) 
		TerminateThread(H_DATA_LISTENER_THREAD, 0);
	if(COMListenerThreadData.OverlappedReadEvent != INVALID_HANDLE_VALUE) 
		CloseHandle(COMListenerThreadData.OverlappedReadEvent);
		
	if(OVL_WRITE.hEvent != INVALID_HANDLE_VALUE)
		CloseHandle(OVL_WRITE.hEvent);
		
	if((int)HCOM > 0) CloseHandle(HCOM);
	KillTimer(hwnd, ID_TIMER1);
	
	status = SPI_CloseChannel(FT_CHANNEL_HANDLE);
	FT_CHANNEL_HANDLE = NULL;
	Unload_libMPSSE();
	return 0;
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
	case WM_DATA_LISTENER:
	{	
		switch(wparam)
		{
			case CE_NEW_DATA:
			{
				// printf("CE_NEW_DATA\n");
				
				TDataBlock* data = (TDataBlock*) (HLOCAL) lparam;
				
				// // for SPI_ReadWrite:
				// char* returnBuffer = (char*) malloc((*data).nBytes);
				// memset(returnBuffer,0,(*data).nBytes);
				
				// for SPI_Read only:
				char* returnBuffer = (char*) malloc(SPI_BYTES_TO_READ + 1);
				memset(returnBuffer,0,SPI_BYTES_TO_READ);
	
				FT_STATUS status;
				uint32 sizeTransferred;
				sizeTransferred = 0;
	
				
				/*
				status = SPI_ReadWrite(
					FT_CHANNEL_HANDLE, // FT_HANDLE handle
					(uint8*) returnBuffer, // uint8 *inBuffer 
					(uint8*) (*data).data, // uint8 *outBuffer
					(*data).nBytes, // uint32 sizeToTransfer
					&sizeTransferred, // uint32 *sizeTransferred 
					SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES //uint32 transferOptions
					| SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE
					| SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
				//*/
				
				
				
				// SPI_Write works, SPI_ReadWrite doesn't. > perform an SPI write, followed by 
				// SPI read operation.
				uint32 transferOptions = SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE;
				if(SPI_BYTES_TO_READ == 0)
				{
					transferOptions |= SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE;
				}
				
				status = SPI_Write(
					FT_CHANNEL_HANDLE,
					(uint8*)(*data).data, // wrong: (uint8*) returnBuffer,// uint8 *outBuffer
					(*data).nBytes, // uint32 sizeToTransfer
					&sizeTransferred, // uint32 *sizeTransferred 
					transferOptions //uint32 transferOptions	
				);
				//*/
				if(APP_TEST_STATUS(status))
				{	APP_REPORT_STATUS(status);
					break;
				}
				
				printf("%d SPI transfer: %d of %d byte(s)\n", GetTickCount(), sizeTransferred, (*data).nBytes);
				
				totalBytesTransferredLast = totalBytesTransferred;
				totalBytesTransferred += sizeTransferred;

				if(SPI_BYTES_TO_READ > 0)
				{
						
					sizeTransferred = 0;
					
					status = SPI_Read(
						FT_CHANNEL_HANDLE,
						(uint8*) returnBuffer,// uint8 *inBuffer
						SPI_BYTES_TO_READ, // uint32 sizeToTransfer
						&sizeTransferred, // uint32 *sizeTransferred 
						SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES //uint32 transferOptions
						| SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE
					);					

					
					if(APP_TEST_STATUS(status))
					{	APP_REPORT_STATUS(status);
						break;
					}
					//*/

					
					// DWORD errors;
					// COMSTAT comstat;
					// ClearCommError(HCOM, &errors, &comstat);
					
					DWORD COMBytesWritten;
					COMBytesWritten = 0;
					
					WriteFile(HCOM, 
						//(*data).data,
						returnBuffer,
						//(*data).nBytes,
						sizeTransferred,
						&COMBytesWritten,
						&OVL_WRITE
					);
					// FlushFileBuffers(HCOM);
					if (WaitForSingleObject(OVL_WRITE.hEvent,75)!=WAIT_OBJECT_0)
					{	printf("Error: WriteData overlapped I/O error %d\n", GetLastError());
					}

					// printf("written %d bytes\n", COMBytesWritten);
		
				}
		
				free(returnBuffer);
				LocalFree(data);
				break;	
			}
			default:;
		}
		break;
	} 
  
	case WM_LBUTTONDBLCLK:
	{
		if(IsWindowVisible(consoleHandle))
		{
			ShowWindow(consoleHandle, SW_HIDE);
		} else {
			ShowWindow(consoleHandle, SW_SHOW);
			SetFocus(hwnd);
		}
		break;
	}
	
	case WM_TIMER:
	{
		switch(wparam)
		{
			case ID_TIMER1:{
				// timer 1 event here...
				if (totalBytesTransferred != totalBytesTransferredLast)
				{
					char buf[128];
					sprintf(buf, "[%d|%d] %s", COM_PORT,totalBytesTransferred, AppTitle);
					SetWindowText(hwnd, buf);
				}
				break;
			};
			default:;
		}
		break;
	}

    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC dc;
      RECT r;
      GetClientRect(hwnd,&r);
      dc=BeginPaint(hwnd,&ps);
      DrawText(dc,windowText.c_str(),-1,&r,DT_WORDBREAK);
      EndPaint(hwnd,&ps);
      break;
    }

    case WM_DESTROY:
      PostQuitMessage(0);
      break;

    default:
      return DefWindowProc(hwnd, msg, wparam, lparam);
  }
  return 0;
}