.\bin\lcc -Wa-l -Wl-m  -c -o PrintScreen360_example.o PrintScreen360_example.c
.\bin\lcc -Wa-l -Wl-m -c -o PrintCmd.o ..\PrintCmd.c
.\bin\lcc -Wa-l -Wl-m -o PrintScreen360_example.gb PrintScreen360_example.o PrintCmd.o
del *.asm
del *.o
del *.sym
del *.lst
del *.rel
del *.ihx
del *.map
del *.noi  
pause