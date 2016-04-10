/*
===============================================================================
 Name        : LCD_Test.c
 Author      : Prakash Kurup
 Description: main definition
===============================================================================
#include <cr_section_macros.h>
#include <NXP/crp.h>
#include "my_picture.h"

// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const unsigned int CRP_WORD = CRP_NO_CRP ;

#include "LPC17xx.h"                        /* LPC13xx definitions */
#include "ssp.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "font.h"
#include "extint.h"
#include <stdio.h>
/* Be careful with the port number and location number, because
some of the location may not exist in that port. */
#define PORT_NUM			1
#define LOCATION_NUM		0

#define pgm_read_byte(addr) (*(const unsigned char *)(addr))

uint8_t src_addr[SSP_BUFSIZE];
uint8_t dest_addr[SSP_BUFSIZE];
int colstart = 0;
int rowstart = 0;

#define ST7735_TFTWIDTH  127 //LCD width
#define ST7735_TFTHEIGHT 159 //LCD height
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define swap(x, y) { x = x + y; y = x - y; x = x - y; }

//color code
#define GREEN 0x00FF00
#define BLACK  0x000000
#define RED  0xFF0000
#define BLUE  0x0000FF
#define WHITE  0xFFFFFF
#define CYAN    0x00FFFF
#define PURPLE 0x8E388E
#define YELLOW  0xFFD700
#define ORANGE 0xFF8000

int _height = ST7735_TFTHEIGHT;
int _width = ST7735_TFTWIDTH;
int cursor_x = 0, cursor_y = 0;
//uint16_t textcolor = RED, textbgcolor= GREEN;
float textsize = 2;
int wrap = 1;

void spiwrite(uint8_t c)
{
	int portnum = 1;

	src_addr[0] = c;
	SSP_SSELToggle( portnum, 0 );
	SSPSend( portnum, (uint8_t *)src_addr, 1 );
	SSP_SSELToggle( portnum, 1 );
}
void writecommand(uint8_t c) {
	LPC_GPIO0->FIOCLR |= (0x1<<21);
	spiwrite(c);
}
void writedata(uint8_t c) {

	LPC_GPIO0->FIOSET |= (0x1<<21);
	spiwrite(c);
}
void writeword(uint16_t c) {

	uint8_t d;

	d = c >> 8;
	writedata(d);
	d = c & 0xFF;
	writedata(d);
}
void write888(uint32_t color, uint32_t repeat) {
	uint8_t red, green, blue;
	int i;
	red = (color >> 16);
	green = (color >> 8) & 0xFF;
	blue = color & 0xFF;
	for (i = 0; i< repeat; i++) {
		writedata(red);
		writedata(green);
		writedata(blue);
	}
}
void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1,
					uint16_t y1) {

	  writecommand(ST7735_CASET);
	  writeword(x0);
	  writeword(x1);
	  writecommand(ST7735_RASET);
	  writeword(y0);
	  writeword(y1);

}
void drawPixel(int16_t x, int16_t y, uint16_t color) {
    if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;

    setAddrWindow(x,y,x+1,y+1);
    writecommand(ST7735_RAMWR);

    write888(color, 1);
}

void fillrect(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color)
{
	//int16_t i;
	int16_t width, height;

	width = x1-x0+1;
	height = y1-y0+1;
	setAddrWindow(x0,y0,x1,y1);
	writecommand(ST7735_RAMWR);
	write888(color,width*height);
}
void lcddelay(int ms)
{
	int count = 24000;
	int i;

	for ( i = count*ms; i--; i > 0);
}
void lcd_init()
{
/*
 * portnum 	= 0 ;
 * cs 		= p0.16 / p0.6 ?
 * rs		= p0.21
 * rst		= p0.22
 */
	uint32_t portnum = 1;
	int i;
	printf(" inside lcd_init\n");
	/* Notice the hack, for portnum 0 p0.16 is used */
	if ( portnum == 0 )
	  {
		LPC_GPIO0->FIODIR |= (0x1<<16);		/* SSP1, P0.16 defined as Outputs */
	  }
	  else
	  {
		LPC_GPIO0->FIODIR |= (0x1<<6);		/* SSP0 P0.6 defined as Outputs */
	  }
	/* Set rs(dc) and rst as outputs */
	LPC_GPIO0->FIODIR |= (0x1<<21);		/* rs/dc P0.21 defined as Outputs */
	LPC_GPIO0->FIODIR |= (0x1<<22);		/* rst P0.22 defined as Outputs */


	/* Reset sequence */
	LPC_GPIO0->FIOSET |= (0x1<<22);

	lcddelay(500);						/*delay 500 ms */
	LPC_GPIO0->FIOCLR |= (0x1<<22);
	lcddelay(500);						/* delay 500 ms */
	LPC_GPIO0->FIOSET |= (0x1<<22);
	lcddelay(500);						/* delay 500 ms */
	 for ( i = 0; i < SSP_BUFSIZE; i++ )	/* Init RD and WR buffer */
	    {
	  	  src_addr[i] = 0;
	  	  dest_addr[i] = 0;
	    }

	 /* do we need Sw reset (cmd 0x01) ? */

	 /* Sleep out */
	 SSP_SSELToggle( portnum, 0 );
	 src_addr[0] = 0x11;	/* Sleep out */
	 SSPSend( portnum, (uint8_t *)src_addr, 1 );
	 SSP_SSELToggle( portnum, 1 );

	 lcddelay(200);
	/* delay 200 ms */
	/* Disp on */
	 SSP_SSELToggle( portnum, 0 );
	 src_addr[0] = 0x29;	/* Disp On */
	 SSPSend( portnum, (uint8_t *)src_addr, 1 );
	 SSP_SSELToggle( portnum, 1 );
	/* delay 200 ms */
	 lcddelay(200);
}

void draw_myline(int16_t x0, int16_t y0, int16_t x1, int16_t y1,uint16_t color) {
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		swap(x0, y0);
		swap(x1, y1);
	}

	if (x0 > x1) {
		swap(x0, x1);
		swap(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}

	for (; x0<=x1; x0++) {
		if (steep) {
			drawPixel(y0, x0, color);
		} else {
			drawPixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color){
	draw_myline(x, y, x, y+h-1, color);
}
void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color){
	draw_myline(x, y, x+w-1, y, color);
}

void rotating_mysquare(int x1, int x2, int x3, int x4, int y1, int y2, int y3, int y4){
//	printf("inside mysquare\n");

	int q1x=0, q2x=0, q3x=0, q4x=0, q1y=0,q2y=0, q3y=0, q4y=0; //new coordinates
	int i;

	for(i=1;i<=11;i++){
		lcddelay(200);
		q1x = (x2+(0.8*(x1-x2))); //P = P0 + lamda*(P1 - P0)
//		printf("q1x : %d\n", q1x);

		q1y = (y2+(0.8*(y1-y2)));
//		printf("q1y : %d\n", q1y);

		q2x = (x3+(0.8*(x2-x3)));
//		printf("q2x : %d\n", q2x);

		q2y = (y3+(0.8*(y2-y3)));
//		printf("q2y : %d\n", q2y);

		q3x = (x4+(0.8*(x3-x4)));
//		printf("q3x : %d\n", q3x);

		q3y = (y4+(0.8*(y3-y4)));
//		printf("q3y : %d\n", q3y);

		q4x = (x1+(0.8*(x4-x1)));
//		printf("q4x : %d\n", q4x);

		q4y = (y1+(0.8*(y4-y1)));
//		printf("q4y : %d\n", q4y);


		//rotating squares
		draw_myline(q1x,q1y,q2x,q2y,BLUE);
		draw_myline(q2x,q2y,q3x,q3y,YELLOW);
		draw_myline(q4x,q4y,q3x,q3y,CYAN);
		draw_myline(q1x,q1y,q4x,q4y,GREEN);

		x1 = q1x;
		x2 = q2x;
		x3 = q3x;
		x4 = q4x;

		y1 = q1y;
		y2 = q2y;
		y3 = q3y;
		y4 = q4y;
	}
//	printf("exiting mysquare\n");
}

void rotating_mytriangle(int x1, int x2, int x3, int y1, int y2, int y3){
//	printf("inside mytriangle\n");

	int q1x=0, q2x=0, q3x=0, q1y=0, q2y=0, q3y=0;
	int i;

	for(i=1;i<=11;i++){
		lcddelay(200);
		q1x = (x2+(0.8*(x1-x2)));
//		printf("q1x : %d\n", q1x);

		q1y = (y2+(0.8*(y1-y2)));
//		printf("q1y : %d\n", q1y);

		q2x = (x3+(0.8*(x2-x3)));
//		printf("q2x : %d\n", q2x);

		q2y = (y3+(0.8*(y2-y3)));
//		printf("q2y : %d\n", q2y);

		q3x = (x1+(0.8*(x3-x1)));
//		printf("q3x : %d\n", q3x);

		q3y = (y1+(0.8*(y3-y1)));
//		printf("q3y : %d\n", q3y);

		draw_myline(q1x,q1y,q2x,q2y,WHITE);
		draw_myline(q2x,q2y,q3x,q3y,YELLOW);
		draw_myline(q1x,q1y,q3x,q3y,BLUE);

		x1 = q1x;
		x2 = q2x;
		x3 = q3x;

		y1 = q1y;
		y2 = q2y;
		y3 = q3y;
	}
//	printf("exiting mytriangle\n");
}

void grow_mytree(int x0, int y0, float angle, int length, int level, int color){
//	printf("inside mytree\n");

	int x1, y1, length1;
	float angle1;

	if(level>0){
		//x-y coordinates of branch
		x1 = x0+length*cos(angle);
//		printf("%f\n",x1);
	    y1 = y0+length*sin(angle);
//	    printf("%f\n",y1);

	    draw_myline(x0,y0,x1,y1,color); //tree branch

	    angle1 = angle + 0.52; //deviate right->0.52 rad/30 deg
	    length1 = 0.8 * length; //reduction of length by 20% of previous length
	    grow_mytree(x1,y1,angle1,length1,level-1,color);

	    angle1 = angle - 0.52; //deviate left->0.52 rad/30 deg
	    length1 = 0.8 * length;
	    grow_mytree(x1,y1,angle1,length1,level-1,color);

	    angle1 = angle; //center->0 deg
	    length1 = 0.8 * length;
	    grow_mytree(x1,y1,angle1,length1,level-1,color);
	}
	//	printf("exiting mytree\n");
}
//============================================ROTATING 3D CUBE==============================================================
const float sin_d[] = {
  0,0.17,0.34,0.5,0.64,0.77,0.87,0.94,0.98,1,0.98,0.94,
  0.87,0.77,0.64,0.5,0.34,0.17,0,-0.17,-0.34,-0.5,-0.64,
  -0.77,-0.87,-0.94,-0.98,-1,-0.98,-0.94,-0.87,-0.77,
  -0.64,-0.5,-0.34,-0.17 };
const float cos_d[] = {
  1,0.98,0.94,0.87,0.77,0.64,0.5,0.34,0.17,0,-0.17,-0.34,
  -0.5,-0.64,-0.77,-0.87,-0.94,-0.98,-1,-0.98,-0.94,-0.87,
  -0.77,-0.64,-0.5,-0.34,-0.17,0,0.17,0.34,0.5,0.64,0.77,
  0.87,0.94,0.98};

int px[] = {-15,  15,  15, -15, -15,  15,  15, -15 };
int py[] = {-15, -15,  15,  15, -15, -15,  15,  15 };
int pz[] = {-15, -15, -15, -15,  15,  15,  15,  15 };

int p2x[] = {0,0,0,0,0,0,0,0};
int p2y[] = {0,0,0,0,0,0,0,0};

int r[] = {0,0,0};

/******************************************************************************
**   Main Function  main()
******************************************************************************/
int main (void)
{
  SSP1Init(); //SPI initialize

  lcd_init(); //LCD initialize

  //fillrect(0, 0, ST7735_TFTWIDTH, ST7735_TFTHEIGHT, RED);
  //writestring("Lab3 LCD Interface \n CMPE 240 Prakash 010015655",0,0,GREEN,RED,2);
  //lcddelay(500);

//============================================================3D CUBE========================================================================
  fillrect(0, 0, ST7735_TFTWIDTH, ST7735_TFTHEIGHT, WHITE);
  int i;

  int x1 = 14+50*cos(0.785); //49.36
  int y1 = 80+50*sin(0.785);

  int x2 = 64+50*cos(0.785); //99.36
  int y2 = 80+50*sin(0.785);

  //creating 3D cube
  for(i=0; i<50; i++){
	  draw_mysquare(14,80-i,x1,y1-i,x2,y2-i,64,80-i);
  }

  y1 = y1-50;
  y2 = y2-50;
  for(i=0;i<50;i++){
	  draw_myline(x1,y1+i,x2,y2+i,RED);
	  draw_myline(14,30+i,x1,y1+i,PURPLE);
	  draw_myline(14+i,30,x1+i,y1,GREEN);
  }

  y1 = y1+50;
  for(i=0; i<40; i++){
	  draw_myline(14-i,80,x1-i,y1,BLACK);
  }


  //Tree

  draw_myline(77.36,y1-10,77.36,y1,BROWN); //tree trunk_1

  grow_mytree(77.36,y1-10,5.23,10,4,GREEN); //right branch (angle = 5.23 rad/300 deg)

  grow_mytree(77.36,y1-10,4.18,10,4,GREEN); //left branch (angle = 4.18 rad/240 deg)

  grow_mytree(77.36,y1-10,4.71,10,4,GREEN); //center branch (angle = 4.71 rad/0 deg)


  //Rotating square
  //14 = 14+5
  //80 = 80

  int a = 19+35*cos(0.785);
  int b = 80+35*sin(0.785);
  int c = 45+35*sin(0.785);

  draw_myline(19,80,a,b,BLACK);
  draw_myline(19,45,19,80,BLACK);
  draw_myline(19,45,a,c,BLACK);
  draw_myline(a,c,a,b,BLACK);

  rotating_mysquare(19,a,a,19,80,b,c,45); //rotating squares

  y1 = y1-55;
  x1 = x1+3;

  //Rotating Triangle
  draw_myline(x1,y1,x1+35,y1,BLACK);
  draw_myline(x1,y1,44,35,BLACK);
  draw_myline(44,35,x1+35,y1,BLACK);


  //Rotating Triangle
  rotating_mytriangle(x1,x1+35,44,y1,y1,35); //rotating triangles

  lcddelay(1000);


//=======================================================ROTATING 3D CUBE===================================================================
  fillrect(0, 0, ST7735_TFTWIDTH, ST7735_TFTHEIGHT, BLACK);
  while(1){

//  for(i=0;i<30;i++){
//  fillrect(0, 0, ST7735_TFTWIDTH, ST7735_TFTHEIGHT, BLACK);
  r[0]=r[0]+1;
  r[1]=r[1]+1;
  if (r[0] == 36) r[0] = 0;
  if (r[1] == 36) r[1] = 0;
  if (r[2] == 36) r[2] = 0;

  int i;
  for (i=0;i<8;i++)
  {
    float px2 = px[i];
    float py2 = cos_d[r[0]]*py[i] - sin_d[r[0]]*pz[i];
    float pz2 = sin_d[r[0]]*py[i] + cos_d[r[0]]*pz[i];

    float px3 = cos_d[r[1]]*px2 + sin_d[r[1]]*pz2;
    float py3 = py2;
    float pz3 = -sin_d[r[1]]*px2 + cos_d[r[1]]*pz2;

    float ax = cos_d[r[2]]*px3 - sin_d[r[2]]*py3;
    float ay = sin_d[r[2]]*px3 + cos_d[r[2]]*py3;
    float az = pz3-190;

    p2x[i] = ((_width)/2)+ax*400/az;
    p2y[i] = ((_height)/2)+ay*400/az;
  }

  //creating 3D cube
  for (i=0;i<3;i++) {
	draw_myline(p2x[i],p2y[i],p2x[i+1],p2y[i+1],GREEN);
	draw_myline(p2x[i+4],p2y[i+4],p2x[i+5],p2y[i+5],GREEN);
	draw_myline(p2x[i],p2y[i],p2x[i+4],p2y[i+4],GREEN);
  }
  draw_myline(p2x[3],p2y[3],p2x[0],p2y[0],GREEN);
  draw_myline(p2x[7],p2y[7],p2x[4],p2y[4],GREEN);
  draw_myline(p2x[3],p2y[3],p2x[7],p2y[7],GREEN);
//  }

//lcddelay(200);

  //clearing the cube
  for(i=0;i<3;i++) {
	draw_myline(p2x[i],p2y[i],p2x[i+1],p2y[i+1],BLACK);
	draw_myline(p2x[i+4],p2y[i+4],p2x[i+5],p2y[i+5],BLACK);
	draw_myline(p2x[i],p2y[i],p2x[i+4],p2y[i+4],BLACK);
  }
  draw_myline(p2x[3],p2y[3],p2x[0],p2y[0],BLACK);
  draw_myline(p2x[7],p2y[7],p2x[4],p2y[4],BLACK);
  draw_myline(p2x[3],p2y[3],p2x[7],p2y[7],BLACK);

//lcddelay(300);
}

  //========================ROTATING SQUARE========================
  fillrect(0, 0, ST7735_TFTWIDTH, ST7735_TFTHEIGHT, BLACK);

  //drawing base square
  draw_myline(0,0,126,0,GREEN);
  draw_myline(126,0,126,126,GREEN);
  draw_myline(0,126,126,126,GREEN);
  draw_myline(0,126,0,0,GREEN);

  lcddelay(200);

  rotating_mysquare(0,126,126,0,0,0,126,126); //rotating squares

  lcddelay(200);

  //========================ROTATING TRIANGLE==========================
  //drawing base triangle
  fillrect(0, 0, ST7735_TFTWIDTH, ST7735_TFTHEIGHT, BLACK);
  draw_myline(63,0,126,158,GREEN);
  draw_myline(0,158,126,158,GREEN);
  draw_myline(63,0,0,158,GREEN);

  lcddelay(200);

  rotating_mytriangle(63,126,0,0,158,158); //rotating triangles

  lcddelay(500);

  //==========================TREES================================
  fillrect(0, 0, ST7735_TFTWIDTH, ST7735_TFTHEIGHT, BLACK);

  draw_myline(64,100,64,160,GREEN); //tree trunk_1

//  draw_myline(32,100,32,160,GREEN); //tree trunk_2

  grow_mytree(64,100,5.23,20,7,GREEN); //right branch (angle = 5.23 rad/300 deg)
//  grow_mytree(32,150,5.23,30,4,GREEN);
  grow_mytree(64,100,4.18,20,7,GREEN); //left branch (angle = 4.18 rad/240 deg)
//  grow_mytree(32,150,4.18,30,4,GREEN);
  grow_mytree(64,100,4.71,20,7,GREEN); //center branch (angle = 4.71 rad/0 deg)
//  grow_mytree(32,150,4.71,30,4,GREEN);

  return 0;
}


/******************************************************************************
**                            End Of File
******************************************************************************/
