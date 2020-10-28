/*
 * letimer.c
 *
 *  Created on: May 13, 2019
 *      Author: kgraham
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "app.h"
#include "display.h"
#include "textdisplay.h"
#include "retargettextdisplay.h"
#include "gpio.h"

//***********************************************************************************
// defined files
//***********************************************************************************

//***********************************************************************************
// global variables
//***********************************************************************************

//***********************************************************************************
// function
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *   Sets up the peripherals used in the application.
 ******************************************************************************/

void print_speed_dir(int speed, int dir) {
	//Clear Screen
	printf("\f");
	//Position on Screen
	printf(TEXTDISPLAY_ESC_SEQ_CURSOR_DOWN_ONE_LINE);
	printf("    Vehicle Info\n");
	//Print speed
	printf(" Speed: %d\n", speed);

	switch (dir) {
		case HARD_LEFT:
			printf(" Direction: Hard Left");
			break;
		case LEFT:
			printf(" Direction: Left");
			break;
		case RIGHT:
			printf(" Direction: Right");
			break;
		case HARD_RIGHT:
			printf("Direction: Hard Right");
			break;
		default:
			printf(" Direction: Straight");
			break;
	}
}

