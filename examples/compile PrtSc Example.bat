.\bin\lcc -Wa-l -Wl-m  -c -o PrintScreen_example.o PrintScreen_example.c
.\bin\lcc -Wa-l -Wl-m -c -o PrintCmd.o ..\PrintCmd.c
.\bin\lcc -Wa-l -Wl-m -o PrintScreen_example.gb PrintScreen_example.o PrintCmd.o
del *.asm
del *.o
del *.sym
del *.lst
del *.rel
del *.ihx
del *.map
del *.noi  
pause