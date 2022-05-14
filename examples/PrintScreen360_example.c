
#include <gb/gb.h>
#include <stdio.h>
#include <gb/drawing.h>
#include "..\PrintCmd.h"
#include "example360_data.c"
void main(){
	//draw some sine wave
	int8_t old = 0-sinus(-810)/140+72;
	int8_t cur;
	for(uint8_t x = 0; x < 159; x++){
		signed int j = x-80;
		cur = 0-sinus(j*10)/140+72;
		line(x,old,x+1,cur);
		old = cur;
	}
	//print the screen
	if(fullPrinterInit(5)){
		PrintScreen360(1);
		gprint("print succesful");
	}
	else gprint("printer not connected");
}