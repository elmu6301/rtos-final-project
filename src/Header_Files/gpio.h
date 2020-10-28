//***********************************************************************************
// Include files
//***********************************************************************************
#include "em_gpio.h"

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

// LED 0 pin is
#define	LED0_port		gpioPortF
#define LED0_pin		04u
#define LED0_default	false 	// Default false (0) = off, true (1) = on
// LED 1 pin is
#define LED1_port		gpioPortF
#define LED1_pin		05u
#define LED1_default	false	// Default false (0) = off, true (1) = on

// BUTTON 0 pin is
#define	BUTTON0_port	gpioPortF
#define BUTTON0_pin		06u
#define BUTTON0_default	true	// Default false (0) = off, true (1) = on
//#define BUTTON0_event   0x1;

// BUTTON 1 pin is
#define BUTTON1_port	gpioPortF
#define BUTTON1_pin		07u
#define BUTTON1_default	true	// Default false (0) = off, true (1) = on
//#define BUTTON1_event   0x2;


// CAPSENSOR Channel definitions
#define LEFT_CHANNEL    0
#define LEFT_C_CHANNEL  1
#define RIGHT_CHANNEL   2
#define RIGHT_C_CHANNEL 3



//Defaults for global variables
#define PB0_STATUS_default false
#define PB1_STATUS_default false
#define CAP_STATUS_default -1



//

//***********************************************************************************
// global variables
//***********************************************************************************

//EventGroup Defines
OS_FLAG_GRP VehicleEventGroup;
OS_FLAG_GRP ViolationEventGroup;

//MessageQueue
OS_Q BUTTON_FIFO;

//Semaphore for the button FIFO
OS_SEM BUTTON_FIFO_SEM;

// Enumerations for the ButtonStatusEventGroup
enum VEHICLE_EVENTS{
	UPDATE_SPEED = 0x1,
	UPDATE_DIRECTION = 0x2,

};

enum VIOLATION_EVENTS{
	NO_VIOLATION = 0x0,
	SPEED_VIOLATION = 0x1,
	DIRECTION_VIOLATION = 0x2,
	CLEAR_VIOLATION = 0x4
};

// Enumerations for the Button Status
enum BUTTONS_STATUS{
	NONE = 0,
	BTN0 = 0x01,
	BTN1 = 0x10,
	BOTH = 0x11
};

// Enumerations for the Slider Status
enum SLIDER_STATUS{
	Slider_Not_Or_Both_Pressed = 0,
	Slider_Left_Pressed  = 1,
	Slider_Right_Pressed = 2,
	Slider_Unknown = -1
};

enum SENDERS{
	BUTTON_INPUT_TASK,
	SLIDER_INPUT_TASK
};

enum DIRECTION{
	HARD_LEFT,
	LEFT,
	STRAIGHT,
	RIGHT,
	HARD_RIGHT
};

typedef struct {
	int dino_state;
} BUTTON_MESSAGE_STRUCT;

typedef struct{
	int speed;
	int num_deltas_seen;
}SETPOINT_DATA_STRUCT;

typedef struct{
	int dir;
	int curr_durr;
	int num_L_turns_seen;
	int num_R_turns_seen;
}VEHICLE_DIRECTION_STRUCT;


//***********************************************************************************
// function prototypes
//***********************************************************************************
void gpio_button_setup();
void gpio_LED_setup();
void gpio_CAPSENSE_setup();
void gpio_open(void);

int sample_pb0();
int sample_pb1();
int sample_cap();

void set_LED0(int pb0_status, int pb1_status, int cap_status);
void set_LED1(int pb0_status, int pb1_status, int cap_status);
void set_LEDs(int violations);


