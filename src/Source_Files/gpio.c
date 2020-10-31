//***********************************************************************************
// Include files
//***********************************************************************************
#include "gpio.h"
#include "capsense.h"
#include "game.h"

#include  <common/include/common.h>
#include  <kernel/include/os.h>
#include  <kernel/include/os_trace.h>
#include  <kernel/include/os_type.h>

#include  <common/include/lib_def.h>
#include  <common/include/rtos_utils.h>
#include  <common/include/toolchains.h>

//***********************************************************************************
// defined files
//***********************************************************************************
#define SPEED_DELTA 5 //Stores the delta to update the speed by

//***********************************************************************************
// global variables
//***********************************************************************************
BUTTON_MESSAGE_STRUCT b_msg;

//***********************************************************************************
// function prototypes
//***********************************************************************************

//***********************************************************************************
// functions
//***********************************************************************************
/***************************************************************************//**
 * @brief Unified GPIO Interrupt handler (pushbuttons)
 *     Used to turn on and off the LEDS when the corresponding button is pressed
 *****************************************************************************/
void GPIO_Unified_IRQ(void) {
	/* Get and clear all pending GPIO interrupts */
	uint32_t interruptMask = GPIO_IntGet();
	GPIO_IntClear(interruptMask);

	//Assume falling edge -> set to run
	b_msg.dino_state = RUN;
	int btns_state = NONE;
	RTOS_ERR err;

	//Check Button 0 rising edge -> set to DUCK
	if ((interruptMask & (1 << BUTTON0_pin)) && !GPIO_PinInGet(BUTTON0_port, BUTTON0_pin)) {
		b_msg.dino_state = JUMP;
		btns_state |= BTN0;

	}
	//Check Button 1 rising edge -> set to JUMP
	if ((interruptMask & (1 << BUTTON1_pin)) && !GPIO_PinInGet(BUTTON1_port, BUTTON1_pin)) {
		b_msg.dino_state = DUCK;
		btns_state |= BTN1;
	}

	//Check to see if both buttons were pressed -> set to RUN
	if(btns_state == BOTH){
		b_msg.dino_state = RUN;
	}

	//Send message to UpdateDinoTask
	OSQPost(&BUTTON_FIFO, &b_msg, (OS_MSG_SIZE) sizeof(BUTTON_MESSAGE_STRUCT),
	OS_OPT_POST_FIFO, &err);
}

/***************************************************************************//**
 * @brief GPIO Interrupt handler for even pins
 ******************************************************************************/
void GPIO_EVEN_IRQHandler(void) {
	GPIO_Unified_IRQ();
}

/***************************************************************************//**
 * @brief GPIO Interrupt handler for odd pins
 ******************************************************************************/
void GPIO_ODD_IRQHandler(void) {
	GPIO_Unified_IRQ();
}
/***************************************************************************//**
 * @brief  Sets up the GPIO pins for the push buttons and initializes the global
 * variables pb0_status and pb1_status to the default value of off.
 ******************************************************************************/
void gpio_button_setup() {
	/*BUTTON 0 SETUP*/
	GPIO_PinModeSet(BUTTON0_port, BUTTON0_pin, gpioModeInputPull,
	BUTTON0_default);

	/*BUTTON 1 SETUP*/
	GPIO_PinModeSet(BUTTON1_port, BUTTON1_pin, gpioModeInputPull,
	BUTTON1_default);

	//enable a rising and falling edge interrupt on each push button
	GPIO_IntConfig(BUTTON0_port, BUTTON0_pin, true, true, true);
	GPIO_IntConfig(BUTTON1_port, BUTTON1_pin, true, true, true);

	///Enable GPIO interrupts
	NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
	NVIC_EnableIRQ(GPIO_EVEN_IRQn);

	NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
	NVIC_EnableIRQ(GPIO_ODD_IRQn);
}
/***************************************************************************//**
 * @brief  Sets up the GPIO pins for the LEDs/
 ******************************************************************************/
void gpio_LED_setup() {
	/*LED 0 SETUP*/
	// Set LED ports to be standard output drive with default off (cleared)
	GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);
	GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, LED0_default);

	/*LED 1 SETUP*/
	GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthStrongAlternateStrong);
	GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPull, LED1_default);

}
/***************************************************************************//**
 * @brief  Sets up the touch sense slider and initializes the global variable
 * 	cap_status to the default value of off.
 ******************************************************************************/
void gpio_CAPSENSE_setup() {
	/*CAPSENSOR Setup*/
	CAPSENSE_Init();
}

/***************************************************************************//**
 * @brief  Sets up all GPIO pins. This case the LEDS and BUTTONS
 ******************************************************************************/
void gpio_open(void) {
	gpio_LED_setup();
	gpio_button_setup();
	gpio_CAPSENSE_setup();
}

/***************************************************************************//**
 * @brief
 * Returns the current state of the button,
 * where 1 means the button is pressed and 0 means that the button is not pressed.
 ******************************************************************************/
int sample_pb0() {
	return !GPIO_PinInGet(BUTTON0_port, BUTTON0_pin);
}

/***************************************************************************//**
 * @brief
 * Returns the current state of the button,
 * where 1 means the button is pressed and 0 means that the button is not pressed.
 ******************************************************************************/
int sample_pb1() {
	return !GPIO_PinInGet(BUTTON1_port, BUTTON1_pin);
}

/***************************************************************************//**
 * @brief
 * Returns the state of the capacitive sensor, where 0 means the left side is pressed, 1 means that
 * the right side is pressed, and -1 means that either no side is pressed or both are.
 ******************************************************************************/
int sample_cap() {
	CAPSENSE_Sense();
	int cap_status = CAPSENSE_getSliderPosition();
	if(cap_status < 0){
		return cap_status;
	}
	cap_status = (MAX_GAME_SPEED-MIN_GAME_SPEED)/48.0 * cap_status + MIN_GAME_SPEED;

	return cap_status;
}
/***************************************************************************//**
 * @brief
 * Turns on LED0 if the left side of the touch sensor is touched or push button 0 is pressed.
 * If both buttons is pressed or the opposite button and side of the touch sensor are pressed,
 * the LED will not turn on.
 ******************************************************************************/
void set_LED0(int pb0_status, int pb1_status, int cap_status) {
//	if ((cap_status == Slider_Left_Pressed)
//			|| (pb0_status == Button_Pressed && pb1_status == Button_Not_Pressed)) { //Turn on LED0
//		GPIO_PinOutSet(LED0_port, LED0_pin);
//	} else { //Turn off the LEDS
//		GPIO_PinOutClear(LED0_port, LED0_pin);
//	}

}

/***************************************************************************//**
 * @brief
 * Turns on LED1 if the right side of the touch sensor is touched or push button 1 is pressed.
 * If both buttons is pressed or the opposite button and side of the touch sensor are pressed,
 * the LED will not turn on.
 ******************************************************************************/
void set_LED1(int pb0_status, int pb1_status, int cap_status) {
//	if ((cap_status == Slider_Right_Pressed)
//			|| (pb0_status == Button_Not_Pressed && pb1_status == Button_Pressed)) { //Turn on LED1
//		GPIO_PinOutSet(LED1_port, LED1_pin);
//	} else { //Turn off the LED
//		GPIO_PinOutClear(LED1_port, LED1_pin);
//	}

}

/***************************************************************************//**
 * @brief
 * TODO
 ******************************************************************************/
void set_LEDs(int violations) {
	//Set LED0 for speed violations
	if (violations & SPEED_VIOLATION) {
		GPIO_PinOutSet(LED0_port, LED0_pin);
	} else {
		GPIO_PinOutClear(LED0_port, LED0_pin);
	}
	//Set LED1 for direction violations
	if (violations & DIRECTION_VIOLATION) {
		GPIO_PinOutSet(LED1_port, LED1_pin);
	} else {
		GPIO_PinOutClear(LED1_port, LED1_pin);
	}

}

