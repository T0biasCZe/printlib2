
#include <gb/gb.h>
#include <stdio.h>
#include <gbdk/metasprites.h>
#include "..\PrintCmd.h"
#include "example_data.c"
void main(){
	//load some test backgroud
	set_bkg_tiles(0,0,20,18,bkg_map_data);
	set_bkg_data(0,101,bkg_tile_data);
	
	//load some test sprites
	SPRITES_8x16;
	OBP0_REG = 0xE0;
	set_sprite_data(102, 8, sonicspritetiles);
	move_metasprite(sonicsprite,102,0,60,57);
	
	//show the test backgroud/sprites
	SHOW_BKG; 
	SHOW_SPRITES;
	
	//print the screen
	if(fullPrinterInit(5)){
		PrintScreen(1);
		gprint("print succesful");
	}
	else gprint("printer not connected");
}
