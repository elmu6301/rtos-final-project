/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 * The software is governed by the sections of the MSLA applicable to Micrium
 * Software.
 *
 ******************************************************************************/

/*
 *********************************************************************************************************
 *
 *                                             EXAMPLE MAIN
 *
 * File : ex_main.c
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                            INCLUDE FILES
 *********************************************************************************************************
 *********************************************************************************************************
 */
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include  <bsp_os.h>
#include  "bsp.h"

#include  <cpu/include/cpu.h>
#include  <common/include/common.h>
#include  <kernel/include/os.h>
#include  <kernel/include/os_trace.h>

#include  <common/include/lib_def.h>
#include  <common/include/rtos_utils.h>
#include  <common/include/toolchains.h>
#include "gpio.h"

#include "em_device.h"
#include "em_chip.h"
#include "em_emu.h"

#include "em_pcnt.h"
#include "em_prs.h"
#include "display.h"
#include "dmd.h"
#include "glib.h"
#include "bspconfig.h"
#include "textdisplay.h"
#include "retargettextdisplay.h"




#include "app.h"
#include "game.h"


/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                             LOCAL DEFINES
 *********************************************************************************************************
 *********************************************************************************************************
 */
// Starting task definitions
#define  EX_MAIN_START_TASK_PRIO             21u
#define  EX_MAIN_START_TASK_STK_SIZE         512u

// Update Dino task definition
#define  UPDATE_DINO_TASK_PRIO           22u
#define  UPDATE_DINO_TASK_STK_SIZE       512u

// Update game speed task definition
#define  UPDATE_GAME_SPEED_TASK_PRIO        22u
#define  UPDATE_GAME_SPEED_TASK_STK_SIZE    512u

// Game monitor task definition
#define  GAME_MONITOR_TASK_PRIO          22u
#define  GAME_MONITOR_TASK_STK_SIZE      512u

// LCD display task definition
#define  LCD_DISPLAY_TASK_PRIO              22u
#define  LCD_DISPLAY_TASK_STK_SIZE          512u

// LEDOUTPUT task definition
#define  LED_OUTPUT_TASK_PRIO                22u
#define  LED_OUTPUT_TASK_STK_SIZE            512u

#define  MY_IDLE_TASK_PRIO                   23u
#define  MY_IDLE_TASK_STK_SIZE            	 512u

//Violation Conditions
#define MAX_SPEED                           75
#define MAX_TURN_SPEED						45
#define MIN_SPEED							-5
#define MAX_DURR							5

//Timer Periods
#define CAP_SENSE_TMR_PERIOD				10


/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                        LOCAL GLOBAL VARIABLES
 *********************************************************************************************************
 *********************************************************************************************************
 */
//Starting task Stack and TCB
static CPU_STK Ex_MainStartTaskStk[EX_MAIN_START_TASK_STK_SIZE]; //Start Task Stack
static OS_TCB Ex_MainStartTaskTCB; //Start Task TCB

//Update Dino task Stack and TCB
static CPU_STK UpdateDinoTaskStk[UPDATE_DINO_TASK_STK_SIZE];
static OS_TCB  UpdateDinoTaskTCB;

//Update Game Speed
static CPU_STK UpdateGameSpeedTaskStk[UPDATE_GAME_SPEED_TASK_STK_SIZE];
static OS_TCB UpdateGameSpeedTaskTCB;

//Game Monitor task Stack and TCB
static CPU_STK GameMonitorTaskStk[GAME_MONITOR_TASK_STK_SIZE];
static OS_TCB GameMonitorTaskTCB;
//
////LCDDisplay task Stack and TCB
//static CPU_STK LCDDisplayTaskStk[LCD_DISPLAY_TASK_STK_SIZE];
//static OS_TCB LCDDisplayTaskTCB;
//
////LEDOutput task Stack and TCB
//static CPU_STK LEDOutputTaskStk[LED_OUTPUT_TASK_STK_SIZE]; //LEDOutput Task Stack
//static OS_TCB LEDOutputTaskTCB; //LEDOutput Task TCB

//MyIdle task Stack and TCB
static CPU_STK MyIdleTaskStk[MY_IDLE_TASK_STK_SIZE];
static OS_TCB MyIdleTaskTCB;

//Timers
OS_TMR CAP_SENSE_TMR;
OS_TMR MONITOR_TMR;
OS_TMR LCD_TMR;

//Semaphores
OS_SEM CAP_SENSE_SEM;
OS_SEM MONITOR_SEM;
OS_SEM LCD_SEM;

//Locks
OS_MUTEX DINO_LOCK;
OS_MUTEX DIRECTION_LOCK;

//Game Object Global Variables
DINO_STRUCT dino;

//Global glib context
GLIB_Context_t gc;
#define RTCC_PULSE_FREQUENCY    (64)

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                       LOCAL FUNCTION PROTOTYPES
 *********************************************************************************************************
 *********************************************************************************************************
 */

//Timer Callback Function
void cap_sense_tmr_callback(OS_TMR * p_tmr, void * p_arg);
void monitor_tmr_callback(OS_TMR * p_tmr, void * p_arg);
void lcd_tmr_callback(OS_TMR * p_tmr, void * p_arg);

//Task Functions
static void Ex_MainStartTask(void *p_arg);
static void UpdateDinoTask(void *p_arg);
static void UpdateGameSpeedTask(void *p_arg);
static void GameMonitorTask(void *p_arg);
//static void LCDDisplayTask(void *p_arg);
//static void LEDOutputTask(void *p_arg);
static void MyIdleTask(void *p_arg);

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                          GLOBAL FUNCTIONS
 *********************************************************************************************************
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 *                                                main()
 *
 * Description : This is the standard entry point for C applications. It is assumed that your code will
 *               call main() once you have performed all necessary initialization.
 *
 * Argument(s) : None.
 *
 * Return(s)   : None.
 *
 * Note(s)     : None.
 *********************************************************************************************************
 */

int main(void) {
	RTOS_ERR err;

	BSP_SystemInit(); /* Initialize System.                                   */
	CPU_Init(); /* Initialize CPU.                                      */

	OS_TRACE_INIT();
	OSInit(&err); /* Initialize the Kernel.                               */
	/*   Check error code.                                  */
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

	//Initialize Global Structs
	dino.state = RUN;

	//Create Message Queue for button updates
	OSQCreate(&BUTTON_FIFO, "Button FIFO", sizeof(BUTTON_MESSAGE_STRUCT) * 3, &err);
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

	//Create Mutex for Dino Struct
	OSMutexCreate(&DINO_LOCK, "Dino Data Lock", &err);
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

	//Create Timer for the Touch Slider
	OSTmrCreate(&CAP_SENSE_TMR, "Touch Sense Timer", 0, CAP_SENSE_TMR_PERIOD,
			OS_OPT_TMR_PERIODIC,
			&cap_sense_tmr_callback, //callback foo
			(void*) 0, //p_arg
			&err);
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

	//Create Semaphore for the Cap_Sense Timer
	OSSemCreate(&CAP_SENSE_SEM, "Touch Sense Semaphore", 0, &err);
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

	//Create Timer for the Game Monitor Task
	OSTmrCreate(&MONITOR_TMR, "Game Monitor Timer", 0, DEFAULT_GAME_SPEED,
			OS_OPT_TMR_PERIODIC,
			&monitor_tmr_callback, //callback foo
			(void*) 0, //p_arg
			&err);
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

	//Create Semaphore for the Monitor Timer
	OSSemCreate(&MONITOR_SEM, "Game Monitor Semaphore", 0, &err);
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

//
//	//Create Timer with a period of ~100 ms
//	OSTmrCreate(&LCD_TMR, "LCD Timer", 0, 20, // 1 seems too long
//			OS_OPT_TMR_PERIODIC,
//			&lcd_tmr_callback, //callback foo
//			(void*) 0, //p_arg
//			&err);
//	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
//
//	//Create Semaphore for the Timer
//	OSSemCreate(&LCD_SEM, "LCD Semaphore", 0, &err);
//	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
	EMSTATUS status;
	  status = DISPLAY_Init();
	  if (DISPLAY_EMSTATUS_OK != status) {
	    while (true)
	      ;
	  }

	  /* Initialize the DMD module for the DISPLAY device driver. */
	  status = DMD_init(0);
	  if (DMD_OK != status) {
	    while (true)
	      ;
	  }

	  status = GLIB_contextInit(&gc);
	  if (GLIB_OK != status) {
	    while (true)
	      ;
	  }

	//Create Main Task
	OSTaskCreate(&Ex_MainStartTaskTCB, /* Create the Start Task.                               */
	"Ex Main Start Task", Ex_MainStartTask,
	DEF_NULL,
	EX_MAIN_START_TASK_PRIO, &Ex_MainStartTaskStk[0],
			(EX_MAIN_START_TASK_STK_SIZE / 10u),
			EX_MAIN_START_TASK_STK_SIZE, 0u, 0u,
			DEF_NULL, (OS_OPT_TASK_STK_CLR ), &err);
	/*   Check error code.                                  */
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

	OSStart(&err); /* Start the kernel.                                    */
	/*   Check error code.                                  */
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

	return (1);
}

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                           LOCAL FUNCTIONS
 *********************************************************************************************************
 *********************************************************************************************************
 */

void cap_sense_tmr_callback(OS_TMR * p_tmr, void * p_arg) {
	RTOS_ERR err;

	//post the semaphore
	OSSemPost(&CAP_SENSE_SEM, OS_OPT_POST_1, &err);
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
}

void monitor_tmr_callback(OS_TMR * p_tmr, void * p_arg) {
	RTOS_ERR err;

	//post the semaphore
	OSSemPost(&MONITOR_SEM, OS_OPT_POST_1, &err);
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
}


void lcd_tmr_callback(OS_TMR * p_tmr, void * p_arg) {
	RTOS_ERR err;

	//post the semaphore
	OSSemPost(&LCD_SEM, OS_OPT_POST_1, &err);
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
}

/*
 *********************************************************************************************************
 *                                          Ex_MainStartTask()
 *
 * Description : This is the task that will be called by the Startup when all services are initializes
 *               successfully.
 *
 * Argument(s) : p_arg   Argument passed from task creation. Unused, in this case.
 *
 * Return(s)   : None.
 *
 * Notes       : None.
 *********************************************************************************************************
 */

static void Ex_MainStartTask(void *p_arg) {
	RTOS_ERR err;

	PP_UNUSED_PARAM(p_arg); /* Prevent compiler warning.                            */

	BSP_TickInit(); /* Initialize Kernel tick source.                       */

#if (OS_CFG_STAT_TASK_EN == DEF_ENABLED)
	OSStatTaskCPUUsageInit(&err); /* Initialize CPU Usage.                                */
	/*   Check error code.                                  */
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
	CPU_IntDisMeasMaxCurReset(); /* Initialize interrupts disabled measurement.          */
#endif

	Common_Init(&err); /* Call common module initialization example.           */
	APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

	BSP_OS_Init(); /* Initialize the BSP. It is expected that the BSP ...  */
	/* ... will register all the hardware controller to ... */
	/* ... the platform manager at this moment.             */
	/* Create Idle Tasks*/
	if (OS_CFG_TASK_IDLE_EN == DEF_DISABLED) {
		OSTaskCreate(&MyIdleTaskTCB, "MyIdleTask Task", MyIdleTask,
		DEF_NULL,
		MY_IDLE_TASK_PRIO, &MyIdleTaskStk[0], (MY_IDLE_TASK_STK_SIZE / 10u),
		MY_IDLE_TASK_STK_SIZE, 0u, 0u,
		DEF_NULL, (OS_OPT_TASK_STK_CLR ), &err);
	}

	/* Create Tasks*/

	//Create Update Dino Task
//	OSTaskCreate(&UpdateDinoTaskTCB, "Update Dino Task",
//			UpdateDinoTask,
//			DEF_NULL,
//			UPDATE_DINO_TASK_PRIO, &UpdateDinoTaskStk[0],
//			(UPDATE_DINO_TASK_STK_SIZE / 10u), UPDATE_DINO_TASK_STK_SIZE,
//			0u, 0u,
//			DEF_NULL, (OS_OPT_TASK_STK_CLR ), &err);
//
//	//Create Update Game Speed task
//	OSTaskCreate(&UpdateGameSpeedTaskTCB, "Update Game Speed Task",
//			UpdateGameSpeedTask,
//			DEF_NULL, UPDATE_GAME_SPEED_TASK_PRIO, &UpdateGameSpeedTaskStk[0],
//			(UPDATE_GAME_SPEED_TASK_STK_SIZE / 10u),
//			UPDATE_GAME_SPEED_TASK_STK_SIZE, 0u, 0u,
//			DEF_NULL, (OS_OPT_TASK_STK_CLR ), &err);

	//Create Game Monitor task
	OSTaskCreate(&GameMonitorTaskTCB, "Game Monitor Task",
			GameMonitorTask,
			DEF_NULL, GAME_MONITOR_TASK_PRIO, &GameMonitorTaskStk[0],
			(GAME_MONITOR_TASK_STK_SIZE / 10u),
			GAME_MONITOR_TASK_STK_SIZE, 0u, 0u,
			DEF_NULL, (OS_OPT_TASK_STK_CLR ), &err);
//
//	//Create V. Monitor task
//	OSTaskCreate(&LCDDisplayTaskTCB, "LCD Display Task", LCDDisplayTask,
//	DEF_NULL, LCD_DISPLAY_TASK_PRIO, &LCDDisplayTaskStk[0],
//			(LCD_DISPLAY_TASK_STK_SIZE / 10u),
//			LCD_DISPLAY_TASK_STK_SIZE, 0u, 0u,
//			DEF_NULL, (OS_OPT_TASK_STK_CLR ), &err);
//
//	//Create LEDOutput Task
//	OSTaskCreate(&LEDOutputTaskTCB, "LEDOutput Task", LEDOutputTask,
//	DEF_NULL,
//	LED_OUTPUT_TASK_PRIO, &LEDOutputTaskStk[0],
//			(LED_OUTPUT_TASK_STK_SIZE / 10u),
//			LED_OUTPUT_TASK_STK_SIZE, 0u, 0u,
//			DEF_NULL, (OS_OPT_TASK_STK_CLR ), &err);

	/*   Check error code.                                  */
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

	while (DEF_ON) {
//		BSP_LedToggle(0);
		/* Delay Start Task execution for                       */
		OSTimeDly(1000, /*   1000 OS Ticks                                      */
		OS_OPT_TIME_DLY, /*   from now.                                          */
		&err);
		/*   Check error code.                                  */
		APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
	}
}
/***************************************************************************//**
 * @brief TODO
 * @param p_arg pointer to optional data parameter for the task.
 ******************************************************************************/
static void UpdateDinoTask(void *p_arg) {
	RTOS_ERR err;
	PP_UNUSED_PARAM(p_arg); /* Prevent compiler warning.                            */

	//Hardware setup required
	gpio_button_setup();

	//Variable Declaration and initialization
	BUTTON_MESSAGE_STRUCT * msg;
	OS_MSG_SIZE msg_size;

	while (DEF_ON) {
		//Pend on the button message
		msg = (BUTTON_MESSAGE_STRUCT *) OSQPend(&BUTTON_FIFO, 0, //Wait till a msg is received
						OS_OPT_POST_FIFO, &msg_size, (CPU_TS *) 0, &err);
		APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);

		//Acquire SETPOINT_LOCK
		OSMutexPend(&DINO_LOCK, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *) 0,&err);
		APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);

		//Update dino state
		dino.state = msg->dino_state;

		//Release SETPOINT_LOCK
		OSMutexPost(&DINO_LOCK, OS_OPT_POST_NONE, &err);
		APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);



	}

}

/***************************************************************************//**
 * @brief TODO
 * @param p_arg pointer to optional data parameter for the task.
 ******************************************************************************/
static void GameMonitorTask(void *p_arg) {
	RTOS_ERR err;
	PP_UNUSED_PARAM(p_arg); /* Prevent compiler warning.                            */

	//Variable Declaration and initialization
//	OSTmrStart(&MONITOR_TMR, &err);
//	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
	OS_STATE temp;

	//Phase 1 Stuff
//	while (DEF_ON) {
//		OSSemPend(&MONITOR_SEM, 0, //1000,
//						OS_OPT_PEND_BLOCKING,
//						(CPU_TS*) 0, &err);
//		APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
//		BSP_LedToggle(0);
//		OSTimeDly(5, OS_OPT_TIME_DLY, &err);
////		temp = OSTmrStateGet (&MONITOR_TMR,&err);
//	}
	//Phase 2: testing LCD display drawing
//	EMSTATUS status;


//	/* Init DCDC regulator with kit specific parameters */
//	  EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_STK_DEFAULT;
//	  EMU_DCDCInit(&dcdcInit);
	 /* Initialize display module */
//	  status = DISPLAY_Init();
//	  if (DISPLAY_EMSTATUS_OK != status) {
//	    while (true)
//	      ;
//	  }
//
//	  /* Initialize the DMD module for the DISPLAY device driver. */
//	  status = DMD_init(0);
//	  if (DMD_OK != status) {
//	    while (true)
//	      ;
//	  }
//
//	  status = GLIB_contextInit(&gc);
//	  if (GLIB_OK != status) {
//	    while (true)
//	      ;
//	  }

	while (DEF_ON) {
		GLIB_drawLineH(&gc,10,10,20);
		DMD_updateDisplay();
	}




}

/***************************************************************************//**
 * @brief Tasks that reads the slider input and sets the global status variables
 * 	for the slider to the appropriate value.
 * @param p_arg pointer to optional data parameter for the task.
 ******************************************************************************/
static void UpdateGameSpeedTask(void *p_arg) {
	RTOS_ERR err;
	PP_UNUSED_PARAM(p_arg); /* Prevent compiler warning.                            */

	//Hardware setup required
	gpio_CAPSENSE_setup();

	//Variable Declaration and initialization
	int new_speed;
	int curr_speed = DEFAULT_GAME_SPEED;

	//Start Timer
	OSTmrStart(&CAP_SENSE_TMR, &err);
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

	//Run task
	while (DEF_ON) {

		//Pend on the CAP_SENSE_SEM semaphore
		OSSemPend(&CAP_SENSE_SEM, 0, //1000,
				OS_OPT_PEND_BLOCKING,
				(CPU_TS*) 0, &err);
		APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);

		//Sample the touch sensor
		new_speed = sample_cap();

		if(new_speed != -1 && new_speed != curr_speed){
			OSTmrSet (&MONITOR_TMR, 0, new_speed, &monitor_tmr_callback, //callback foo
					(void*) 0, //p_arg
					&err );
			new_speed = curr_speed;
		}


	}
}
//
///***************************************************************************//**
// * @brief Tasks that reads the slider input and sets the global status variables
// * 	for the slider to the appropriate value.
// * @param p_arg pointer to optional data parameter for the task.
// ******************************************************************************/
//static void LCDDisplayTask(void *p_arg) {
//	RTOS_ERR err;
//	PP_UNUSED_PARAM(p_arg); /* Prevent compiler warning.                            */
//
//	//	Hardware setup required
//	DISPLAY_Init();
//	//Retarget stdio to a text display.
//	if (RETARGET_TextDisplayInit() != TEXTDISPLAY_EMSTATUS_OK) {
//		while (1);
//	}
//
//	//Variable Declaration and initialization
//	int curr_dir;
//	int curr_speed;
//
//	//Start Timer
//	OSTmrStart(&LCD_TMR, &err);
//	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
//
//	//Run task
//	while (DEF_ON) {
//
//		//Pend on the LCD_SEM semaphore
//		OSSemPend(&LCD_SEM, 0, //1000,
//				OS_OPT_PEND_BLOCKING, //Try making it non blocking
//				(CPU_TS*) 0, &err);
//		APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
//
//		//Acquire SETPOINT_LOCK
//		OSMutexPend(&SETPOINT_LOCK, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *) 0,
//				&err);
//		APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
//
//		//Update current speed
//		curr_speed = setpoint_data.speed;
//
//		//Release SETPOINT_LOCK
//		OSMutexPost(&SETPOINT_LOCK, OS_OPT_POST_NONE, &err);
//		APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
//
//		//Acquire DIRECTION_LOCK
//		OSMutexPend(&DIRECTION_LOCK, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *) 0,
//				&err);
//		APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
//
//		//Update current direction
//		curr_dir = v_dir_data.dir;
//
//		//Release DIRECTION_LOCK
//		OSMutexPost(&DIRECTION_LOCK, OS_OPT_POST_NONE, &err);
//		APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
//
//		//Update LCD
//		print_speed_dir(curr_speed, curr_dir);
//	}
//}
///***************************************************************************//**
// * @brief Tasks that sets the LEDs based upon the global variables set by the
// * 	push buttons and slider.
// * @param p_arg pointer to optional data parameter for the task.
// ******************************************************************************/
//static void LEDOutputTask(void *p_arg) {
//	RTOS_ERR err;
//	PP_UNUSED_PARAM(p_arg); /* Prevent compiler warning.                            */
//
//	//Hardware setup required
//	gpio_LED_setup();
//
//	//Variable Declaration and Initialization
//	OS_FLAGS events;
//
//	while (DEF_ON) {
//		//Pend on violation flags
//		events = OSFlagPend(
//				&ViolationEventGroup, //ButtonStatus event group
//				CLEAR_VIOLATION + SPEED_VIOLATION + DIRECTION_VIOLATION, //Look for flags set by the buttonTask and Vehicle Direction Task
//				0, //Wait until a flag is set
//				OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME
//						+ OS_OPT_PEND_BLOCKING, //Look for either and clear when done
//				(CPU_TS *) 0, &err);
//		APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
//
//		//Set LED0 to indicate speed violations
//		set_LEDs(events);
//
//	}
//}

/***************************************************************************//**
 * @brief Idle task that puts the MCU into EM1.
 * @param p_arg pointer to optional data parameter for the task.
 ******************************************************************************/
static void MyIdleTask(void *p_arg) {
	RTOS_ERR err;
	PP_UNUSED_PARAM(p_arg); /* Prevent compiler warning.                            */

	while (DEF_ON) {
		//Set the LEDS
		EMU_EnterEM1();
		//Delay for 100 OS Ticks
		OSTimeDly(100, OS_OPT_TIME_DLY, &err);
		//Check error code
		APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
	}
}

