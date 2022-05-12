.\bin\lcc -Wa-l -Wl-m  -c -o main.o main.c
.\bin\lcc -Wa-l -Wl-m -c -o PrintCmd.o PrintCmd.c
.\bin\lcc -Wa-l -Wl-m -o WOOMY.gb main.o PrintCmd.o
pause