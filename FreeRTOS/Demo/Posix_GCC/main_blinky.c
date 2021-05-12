/* System include. */
#include <stdio.h>
#include <pthread.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Local includes. */
#include "console.h"


#define	mainQUEUE_MONITOR_TASK_PRIORITY		( tskIDLE_PRIORITY + 3 )
#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define	mainQUEUE_SEND_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define mainTASK_SEND_FREQUENCY_MS			pdMS_TO_TICKS( 200UL )
#define mainTASK_RECEIVE_FREQUENCY_MS			pdMS_TO_TICKS( 400UL )
#define mainTIMER_SEND_FREQUENCY_MS			pdMS_TO_TICKS( 2000UL )
#define mainQUEUE_LENGTH					( 10 )
#define mainVALUE_SENT_FROM_TASK			( 100UL )
#define mainVALUE_SENT_FROM_TIMER			( 200UL )

static void prvQueueReceiveTask( void *pvParameters );
static void prvQueueSendTask( void *pvParameters );
static void prvQueueSendTimerCallback( TimerHandle_t xTimerHandle );
static void prvQueueMonitorTask( void *pvParameters );

static QueueHandle_t xQueue = NULL;
static TimerHandle_t xTimer = NULL;


void main_blinky( void )
{
const TickType_t xTimerPeriod = mainTIMER_SEND_FREQUENCY_MS;

	xQueue = xQueueCreate( mainQUEUE_LENGTH, sizeof( uint32_t ) );

	if( xQueue != NULL )
	{
		
		xTaskCreate( prvQueueReceiveTask,			/* The function that implements the task. */
					"Rx", 							/* The text name assigned to the task - for debug only as it is not used by the kernel. */
					configMINIMAL_STACK_SIZE, 		/* The size of the stack to allocate to the task. */
					NULL, 							/* The parameter passed to the task - not used in this simple case. */
					mainQUEUE_RECEIVE_TASK_PRIORITY,/* The priority assigned to the task. */
					NULL );							/* The task handle is not required, so NULL is passed. */

		xTaskCreate( prvQueueSendTask, "TX", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );

		xTaskCreate( prvQueueMonitorTask, "Monitor", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_MONITOR_TASK_PRIORITY, NULL );

		// xTimer = xTimerCreate( "Timer",				/* The text name assigned to the software timer - for debug only as it is not used by the kernel. */
		// 						xTimerPeriod,		/* The period of the software timer in ticks. */
		// 						pdTRUE,				/* xAutoReload is set to pdTRUE. */
		// 						NULL,				/* The timer's ID is not used. */
		// 						prvQueueSendTimerCallback );/* The function executed when the timer expires. */

		// if( xTimer != NULL )
		// {
		// 	xTimerStart( xTimer, 0 );
		// }

		vTaskStartScheduler();
	}

	for( ;; );
}



#define SUCCESS 1
#define FAIL 0
static void prvQueueSendTask( void *pvParameters )
{
TickType_t xNextWakeTime;
const TickType_t xBlockTime = mainTASK_SEND_FREQUENCY_MS;
uint32_t ulValueToSend = 0UL;
uint32_t ret=0;

	/* Prevent the compiler warning about the unused parameter. */
	( void ) pvParameters;

	/* Initialise xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		/* Place this task in the blocked state until it is time to run again.
		The block time is specified in ticks, pdMS_TO_TICKS() was used to
		convert a time specified in milliseconds into a time specified in ticks.
		While in the Blocked state this task will not consume any CPU time. */
		vTaskDelayUntil( &xNextWakeTime, xBlockTime );

		/* Send to the queue - causing the queue receive task to unblock and
		write to the console.  0 is used as the block time so the send operation
		will not block - it shouldn't need to block as the queue should always
		have at least one space at this point in the code. */

		ulValueToSend ++;
		ret = xQueueSend( xQueue, &ulValueToSend, 0U );
		if(ret==SUCCESS){
			console_print( "send success\n");
		}else{
			console_print( "send fail\n");
		}
		
	}
}



static void prvQueueSendTimerCallback( TimerHandle_t xTimerHandle )
{
	const uint32_t ulValueToSend = mainVALUE_SENT_FROM_TIMER;

	/* Avoid compiler warnings resulting from the unused parameter. */
	( void ) xTimerHandle;

	xQueueSend( xQueue, &ulValueToSend, 0U );
}




static void prvQueueReceiveTask( void *pvParameters )
{
uint32_t ulReceivedValue;
const TickType_t xBlockTime = mainTASK_RECEIVE_FREQUENCY_MS;
TickType_t xNextWakeTime;

	/* Prevent the compiler warning about the unused parameter. */
	( void ) pvParameters;

	xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{

		vTaskDelayUntil( &xNextWakeTime, xBlockTime );
		xQueueReceive( xQueue, &ulReceivedValue, portMAX_DELAY );

		console_print( "Message received from task:%d\n",ulReceivedValue);

		// if( ulReceivedValue == mainVALUE_SENT_FROM_TASK )
		// {
		// 	console_print( "Message received from task\n" );
		// }
		// else if( ulReceivedValue == mainVALUE_SENT_FROM_TIMER )
		// {
		// 	console_print( "Message received from software timer\n" );
		// }
		// else
		// {
		// 	console_print( "Unexpected message\n" );
		// }
	}
}


static void prvQueueMonitorTask( void *pvParameters )
{
uint32_t ulReceivedValue;
const TickType_t xBlockTime = 100UL;
TickType_t xNextWakeTime;

	/* Prevent the compiler warning about the unused parameter. */
	( void ) pvParameters;

	xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{

		vTaskDelayUntil( &xNextWakeTime, xBlockTime );
		console_print( "Queue Messages Waiting:%ld\n",uxQueueMessagesWaiting(xQueue));
	}
}
