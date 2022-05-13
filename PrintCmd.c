// Original library by DragonEagles from http://shen.mansell.tripod.com/games/gameboy/gameboy.html
// Ported to GBDK 2020 by T0biasCZe			
// Print Screen function added by Christianr
// Print Screen (360 tile mode) added by T0biasCZe
#include <gb/gb.h>
uint8_t PrinterStatus[3];

const uint8_t PRINTER_INIT[]={
    10,0x88,0x33,0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x00
};
const uint8_t PRINTER_STATUS[]={
    10,0x88,0x33,0x0F,0x00,0x00,0x00,0x0F,0x00,0x00,0x00
};
const uint8_t PRINTER_EOF[]={
    10,0x88,0x33,0x04,0x00,0x00,0x00,0x04,0x00,0x00,0x00
};
const uint8_t PRINTER_START[]={
    14,0x88,0x33,0x02,0x00,0x04,0x00,0x01,0x03,0xE4,0x7F,0x6D,0x01,0x00,0x00
};
const uint8_t PRINT_TILE[]={
    6,0x88,0x33,0x04,0x00,0x80,0x02
};
const uint8_t PRINTER_LINE[]={
    14,0x88,0x33,0x02,0x00,0x04,0x00,0x01,0x00,0xE4,0x7F,0x6A,0x01,0x00,0x00
};

uint8_t tile_num, packet_num;

uint16_t CRC;

uint8_t SendPrinterByte(uint8_t byte){
    uint8_t result;
    disable_interrupts();
    SB_REG = byte; //data to send
    SC_REG = 0x81; //1000 0001 - start, internal clock
    while (SC_REG & 0x80){} //wait until b1 reset
    result = SB_REG; //return response stored in SB_REG
    enable_interrupts();
    return result;
}

void SendByte(uint8_t byte){
    uint8_t result;
    result = SendPrinterByte(byte);
    PrinterStatus[0]=PrinterStatus[1];
    PrinterStatus[1]=PrinterStatus[2];
    PrinterStatus[2]=result;
}

void SendPrinterCommand(uint8_t *Command){
    uint8_t length,index;
    index=0;
    length=*Command;
    while(index < length){
        index++;
        SendByte(*(Command+index));
    }
}

void PrinterInit (void)
{
    tile_num = 0;
    CRC = 0;
    packet_num = 0;

    SendPrinterCommand(PRINTER_INIT);
}

int CheckLinkCable(){
    if(PrinterStatus[0] != 0){
        return 2;
    }
    if((PrinterStatus[1] & 0xF0) != 0x80){
        return 2;
    }
    return 0;
}

int GetPrinterStatus(){
    SendPrinterCommand(PRINTER_STATUS);
    return CheckLinkCable();
}

int CheckForErrors(){
    if(PrinterStatus[2] & 128){
        return 1;
    }
    if(PrinterStatus[2] & 64){
        return 4;
    }
    if(PrinterStatus[2] & 32){
        return 3;
    }
    return 0;
}

uint8_t CheckBusy() {
    SendPrinterCommand(PRINTER_STATUS);
    return (PrinterStatus[2] & 0x2);
}

uint8_t GetHigh(uint16_t w) {
    return (w & 0xFF00u) >> 8;
}

uint8_t GetLow(uint16_t w) {
    return (w & 0xFFu);
}

void PrintTileData(uint8_t *TileData, uint8_t lf, uint8_t num_packets){
    uint8_t TileIndex;
    
    if (tile_num == 0)
    {
        SendPrinterCommand(PRINT_TILE);
        CRC = 0x04 + 0x80 + 0x02;
    }      
    
    tile_num ++;

    for(TileIndex = 0; TileIndex < 16; TileIndex++)
    {
        CRC += TileData[TileIndex];
        SendByte(TileData[TileIndex]);
    }

    if (tile_num == 40)
    {
        SendByte(GetLow(CRC));
        SendByte(GetHigh(CRC));
        SendByte(0x00);
        SendByte(0x00);
        tile_num = 0;  
        CRC = 0;
        packet_num ++;

        if (packet_num == num_packets) // all done the page
        {
            SendPrinterCommand(PRINTER_EOF); // data end packet
            if (lf)
                SendPrinterCommand(PRINTER_START);
            else
                SendPrinterCommand(PRINTER_LINE);
            packet_num = 0;
            SendPrinterCommand(PRINTER_STATUS);
        }
    }
}

void PrintScreen(uint8_t linefeed) {
    uint8_t x, y;
    uint8_t p_data[16];
    for (y=0; y<18; y++) {
        for (x=0; x<20; x++) {
            get_bkg_data(get_vram_byte(get_bkg_xy_addr(x, y)), 1, p_data);
            PrintTileData(p_data, linefeed, 9);
        }
    }
}/*
void PrintScreen360(uint8_t linefeed){
	PrinterInit();
	uint8_t p_data[16];
	uint8_t* curr_tile = (uint8_t*)8100;
	for(int tile = 0; tile < 359; tile++){
        vmemcpy(p_data,curr_tile,1);
        PrintTileData(p_data, linefeed, 9);
		curr_tile+=16;
	}
}
void PrintScreen361(uint8_t linefeed){
	PrinterInit();
	uint8_t p_data[16];
	const unsigned int tilearray[] = {0x8100,0x8110,0x8120,0x8130,0x8140,0x8150,0x8160,0x8170,0x8180,0x8190,0x81A0,0x81B0,0x81C0,0x81D0,0x81E0,0x81F0,0x8200,0x8210,0x8220,0x8230,0x8240,0x8250,0x8260,0x8270,0x8280,0x8290,0x82A0,0x82B0,0x82C0,0x82D0,0x82E0,0x82F0,0x8300,0x8310,0x8320,0x8330,0x8340,0x8350,0x8360,0x8370,0x8380,0x8390,0x83A0,0x83B0,0x83C0,0x83D0,0x83E0,0x83F0,0x8400,0x8410,0x8420,0x8430,0x8440,0x8450,0x8460,0x8470,0x8480,0x8490,0x84A0,0x84B0,0x84C0,0x84D0,0x84E0,0x84F0,0x8500,0x8510,0x8520,0x8530,0x8540,0x8550,0x8560,0x8570,0x8580,0x8590,0x85A0,0x85B0,0x85C0,0x85D0,0x85E0,0x85F0,0x8600,0x8610,0x8620,0x8630,0x8640,0x8650,0x8660,0x8670,0x8680,0x8690,0x86A0,0x86B0,0x86C0,0x86D0,0x86E0,0x86F0,0x8700,0x8710,0x8720,0x8730,0x8740,0x8750,0x8760,0x8770,0x8780,0x8790,0x87A0,0x87B0,0x87C0,0x87D0,0x87E0,0x87F0,0x8800,0x8810,0x8820,0x8830,0x8840,0x8850,0x8860,0x8870,0x8880,0x8890,0x88A0,0x88B0,0x88C0,0x88D0,0x88E0,0x88F0,0x8900,0x8910,0x8920,0x8930,0x8940,0x8950,0x8960,0x8970,0x8980,0x8990,0x89A0,0x89B0,0x89C0,0x89D0,0x89E0,0x89F0,0x8A00,0x8A10,0x8A20,0x8A30,0x8A40,0x8A50,0x8A60,0x8A70,0x8A80,0x8A90,0x8AA0,0x8AB0,0x8AC0,0x8AD0,0x8AE0,0x8AF0,0x8B00,0x8B10,0x8B20,0x8B30,0x8B40,0x8B50,0x8B60,0x8B70,0x8B80,0x8B90,0x8BA0,0x8BB0,0x8BC0,0x8BD0,0x8BE0,0x8BF0,0x8C00,0x8C10,0x8C20,0x8C30,0x8C40,0x8C50,0x8C60,0x8C70,0x8C80,0x8C90,0x8CA0,0x8CB0,0x8CC0,0x8CD0,0x8CE0,0x8CF0,0x8D00,0x8D10,0x8D20,0x8D30,0x8D40,0x8D50,0x8D60,0x8D70,0x8D80,0x8D90,0x8DA0,0x8DB0,0x8DC0,0x8DD0,0x8DE0,0x8DF0,0x8E00,0x8E10,0x8E20,0x8E30,0x8E40,0x8E50,0x8E60,0x8E70,0x8E80,0x8E90,0x8EA0,0x8EB0,0x8EC0,0x8ED0,0x8EE0,0x8EF0,0x8F00,0x8F10,0x8F20,0x8F30,0x8F40,0x8F50,0x8F60,0x8F70,0x8F80,0x8F90,0x8FA0,0x8FB0,0x8FC0,0x8FD0,0x8FE0,0x8FF0,0x9000,0x9010,0x9020,0x9030,0x9040,0x9050,0x9060,0x9070,0x9080,0x9090,0x90A0,0x90B0,0x90C0,0x90D0,0x90E0,0x90F0,0x9100,0x9110,0x9120,0x9130,0x9140,0x9150,0x9160,0x9170,0x9180,0x9190,0x91A0,0x91B0,0x91C0,0x91D0,0x91E0,0x91F0,0x9200,0x9210,0x9220,0x9230,0x9240,0x9250,0x9260,0x9270,0x9280,0x9290,0x92A0,0x92B0,0x92C0,0x92D0,0x92E0,0x92F0,0x9300,0x9310,0x9320,0x9330,0x9340,0x9350,0x9360,0x9370,0x9380,0x9390,0x93A0,0x93B0,0x93C0,0x93D0,0x93E0,0x93F0,0x9400,0x9410,0x9420,0x9430,0x9440,0x9450,0x9460,0x9470,0x9480,0x9490,0x94A0,0x94B0,0x94C0,0x94D0,0x94E0,0x94F0,0x9500,0x9510,0x9520,0x9530,0x9540,0x9550,0x9560,0x9570,0x9580,0x9590,0x95A0,0x95B0,0x95C0,0x95D0,0x95E0,0x95F0,0x9600,0x9610,0x9620,0x9630,0x9640,0x9650,0x9660,0x9670,0x9680,0x9690,0x96A0,0x96B0,0x96C0,0x96D0,0x96E0,0x96F0,0x9700,0x9710,0x9720,0x9730,0x9740,0x9750,0x9760,0x9770};
	for(int tilee = 0; tilee < 359; tilee++){
		vmemcpy(p_data,(uint8_t *)tilearray[tilee],16);
		PrintTileData(p_data, linefeed, 9);
	}
}*/
void PrintScreen360(uint8_t linefeed){
	PrinterInit();
	uint8_t p_data[16];
	vmemcpy(p_data,(uint8_t *)0x8100,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8110,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8120,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8130,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8140,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8150,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8160,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8170,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8180,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8190,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x81A0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x81B0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x81C0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x81D0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x81E0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x81F0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8200,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8210,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8220,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8230,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8240,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8250,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8260,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8270,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8280,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8290,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x82A0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x82B0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x82C0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x82D0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x82E0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x82F0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8300,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8310,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8320,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8330,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8340,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8350,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8360,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8370,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8380,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8390,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x83A0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x83B0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x83C0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x83D0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x83E0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x83F0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8400,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8410,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8420,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8430,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8440,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8450,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8460,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8470,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8480,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8490,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x84A0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x84B0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x84C0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x84D0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x84E0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x84F0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8500,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8510,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8520,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8530,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8540,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8550,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8560,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8570,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8580,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8590,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x85A0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x85B0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x85C0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x85D0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x85E0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x85F0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8600,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8610,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8620,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8630,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8640,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8650,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8660,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8670,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8680,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8690,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x86A0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x86B0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x86C0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x86D0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x86E0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x86F0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8700,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8710,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8720,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8730,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8740,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8750,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8760,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8770,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8780,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8790,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x87A0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x87B0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x87C0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x87D0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x87E0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x87F0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8800,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8810,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8820,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8830,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8840,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8850,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8860,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8870,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8880,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8890,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x88A0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x88B0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x88C0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x88D0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x88E0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x88F0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8900,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8910,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8920,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8930,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8940,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8950,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8960,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8970,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8980,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8990,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x89A0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x89B0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x89C0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x89D0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x89E0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x89F0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8A00,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8A10,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8A20,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8A30,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8A40,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8A50,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8A60,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8A70,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8A80,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8A90,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8AA0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8AB0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8AC0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8AD0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8AE0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8AF0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8B00,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8B10,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8B20,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8B30,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8B40,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8B50,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8B60,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8B70,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8B80,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8B90,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8BA0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8BB0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8BC0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8BD0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8BE0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8BF0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8C00,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8C10,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8C20,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8C30,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8C40,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8C50,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8C60,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8C70,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8C80,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8C90,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8CA0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8CB0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8CC0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8CD0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8CE0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8CF0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8D00,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8D10,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8D20,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8D30,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8D40,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8D50,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8D60,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8D70,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8D80,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8D90,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8DA0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8DB0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8DC0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8DD0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8DE0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8DF0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8E00,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8E10,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8E20,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8E30,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8E40,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8E50,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8E60,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8E70,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8E80,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8E90,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8EA0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8EB0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8EC0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8ED0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8EE0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8EF0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8F00,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8F10,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8F20,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8F30,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8F40,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8F50,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8F60,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8F70,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8F80,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8F90,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8FA0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8FB0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8FC0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8FD0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8FE0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x8FF0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9000,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9010,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9020,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9030,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9040,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9050,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9060,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9070,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9080,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9090,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x90A0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x90B0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x90C0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x90D0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x90E0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x90F0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9100,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9110,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9120,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9130,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9140,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9150,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9160,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9170,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9180,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9190,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x91A0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x91B0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x91C0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x91D0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x91E0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x91F0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9200,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9210,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9220,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9230,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9240,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9250,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9260,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9270,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9280,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9290,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x92A0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x92B0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x92C0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x92D0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x92E0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x92F0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9300,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9310,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9320,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9330,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9340,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9350,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9360,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9370,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9380,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9390,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x93A0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x93B0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x93C0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x93D0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x93E0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x93F0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9400,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9410,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9420,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9430,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9440,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9450,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9460,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9470,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9480,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9490,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x94A0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x94B0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x94C0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x94D0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x94E0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x94F0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9500,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9510,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9520,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9530,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9540,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9550,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9560,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9570,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9580,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9590,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x95A0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x95B0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x95C0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x95D0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x95E0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x95F0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9600,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9610,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9620,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9630,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9640,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9650,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9660,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9670,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9680,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9690,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x96A0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x96B0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x96C0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x96D0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x96E0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x96F0,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9700,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9710,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9720,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9730,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9740,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9750,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9760,16);
	PrintTileData(p_data, linefeed, 9);
	vmemcpy(p_data,(uint8_t *)0x9770,16);
	PrintTileData(p_data, linefeed, 9);
}
