@del comspi.exe
@REM g++ comspi.c libMPSSE_SPI_d.c -o comspi.exe -mwindows
g++ -static comspi.cpp libMPSSE_SPI_d.c -o comspi.exe
@REM @pause