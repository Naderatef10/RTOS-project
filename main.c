#include <stdio.h>
#include <stdlib.h>
#include "diag/trace.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#define CCM_RAM __attribute__((section(".ccmram")))
#include "led.h"
#define BLINK_PORT_NUMBER         (3)
#define BLINK_PIN_NUMBER_GREEN    (12)
#define BLINK_PIN_NUMBER_ORANGE   (13)
#define BLINK_PIN_NUMBER_RED      (14)
#define BLINK_PIN_NUMBER_BLUE     (15)
#define BLINK_ACTIVE_LOW          (false)
struct led blinkLeds[4];
TimerHandle_t sendertimer;
TimerHandle_t receivertimer;
BaseType_t xTimer1Started, xTimer2Started;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"
TaskHandle_t sendertaskhandler=NULL;
TaskHandle_t receivertaskhandler=NULL;
QueueHandle_t myQueue;
#define Treceiver 200
int counter_successful;
int counter_failed;
int counter_received;
int Tsender[6]={100,140,180,220,260,300};
char message[100]="Time is ";
SemaphoreHandle_t mysem_send;
SemaphoreHandle_t mysem_receive;
void sendertask(){
int currenttime=0;
while(1){
	if(xSemaphoreTake(mysem_send,(TickType_t)0)){
	currenttime=xTaskGetTickCount();
	sprintf(message,"Time is %d",currenttime);
	if(xQueueSend(myQueue,(void*)message,(TickType_t) 0)){
		counter_successful++;
	}
	else{
		counter_failed++;
	}

	}
	}

}
void Init(){
static int index_period_timer=0;
int period_timer=0;
if(index_period_timer==0){
	goto label;

}
period_timer=Tsender[index_period_timer];
if( xTimerChangePeriod( sendertimer, period_timer/ portTICK_PERIOD_MS,0)){

}
label:
printf("Total number of successfully sent messages: %d\n",counter_successful);
printf("Total number of received messages:%d\n",counter_received);
printf("Total number of failed messages:%d\n",counter_failed);
if(index_period_timer==6){
	printf("Game over\n");
	  xTimerDelete( sendertimer, 0 );
	  xTimerDelete( receivertimer, 0 );

}
index_period_timer++;
counter_successful=0;
counter_failed=0;
counter_received=0;
xQueueReset(myQueue);

}
void receivertask(){
	char received_message[100]; // string to receive message
	while(1){

		if(xSemaphoreTake(mysem_receive,(TickType_t)5)){

	if(xQueueReceive(myQueue,(void*)received_message,(TickType_t) 5)&&counter_received<500){
		counter_received++;
		//printf("%s\n",received_message);


	}
	else{
		continue;
	}


		}
	}
     }
void sender_timer_callback(TimerHandle_t sendertimer){
	xSemaphoreGive(mysem_send);
}
void receiver_timer_callback(TimerHandle_t receivertimer){
	xSemaphoreGive(mysem_receive);
    if(counter_received==500){
        Init();
    }
}
int
main(int argc, char* argv[])
{myQueue=xQueueCreate(20,sizeof(message));
mysem_send=xSemaphoreCreateBinary(); // creation of semaphore for sender task
mysem_receive=xSemaphoreCreateBinary(); // creation of semaphore for receiver task
// creation of sendertimer in the next line
sendertimer=xTimerCreate("sendertimer",pdMS_TO_TICKS(Tsender[0]),pdTRUE,(void*)0,sender_timer_callback);
//creation of receivertimer in the next line
receivertimer=xTimerCreate("receivertimer",pdMS_TO_TICKS(Treceiver),pdTRUE,(void*)1,receiver_timer_callback);
// creation of the 2 tasks in the following lines
xTaskCreate(sendertask,"sendtask",200,(void*)0,tskIDLE_PRIORITY,&sendertaskhandler);
xTaskCreate(receivertask,"receivetask",200,(void*)0,tskIDLE_PRIORITY+1,&receivertaskhandler);
Init();
// blocking_time in timer sender is 100ms
xTimerStart(sendertimer,0);
// delay in receiver timer is 200ms
xTimerStart(receivertimer,0);
//period of receiver timer is also 200ms
// start scheduler
vTaskStartScheduler();
return 0;
}
#pragma GCC diagnostic pop
void vApplicationMallocFailedHook( void )
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
volatile size_t xFreeStackSpace;

	/* This function is called on each cycle of the idle task.  In this case it
	does nothing useful, other than report the amout of FreeRTOS heap that
	remains unallocated. */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if( xFreeStackSpace > 100 )
	{
		/* By now, the kernel has allocated everything it is going to, so
		if there is a lot of heap remaining unallocated then
		the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		reduced accordingly. */
	}
}

void vApplicationTickHook(void) {
}

StaticTask_t xIdleTaskTCB CCM_RAM;
StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE] CCM_RAM;

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
  /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
  state will be stored. */
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

  /* Pass out the array that will be used as the Idle task's stack. */
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;

  /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
  Note that, as the array is necessarily of type StackType_t,
  configMINIMAL_STACK_SIZE is specified in words, not bytes. */
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

static StaticTask_t xTimerTaskTCB CCM_RAM;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH] CCM_RAM;

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

