#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h" // IdleTask
#include <croutine.h>
#include <queue.h>
#include "stm32f30x.h"
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"
#include "portmacro.h"


void Port_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE); //тактирование светодиода
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = 0xFF00; //все пины порта ≈ 8...15 на выход
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;

	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; //все пины порта ≈ 8...15 на выход
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;

	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

volatile xQueueHandle queue;

unsigned char i = 0;
uint32_t f = 0;
unsigned char j = 0;

void LED(xCoRoutineHandle xHandle, unsigned portBASE_TYPE uxIndex)
{
	portBASE_TYPE xResult;

	crSTART(xHandle);

	while(1)
	{
		crQUEUE_RECEIVE(xHandle, queue, &j, portMAX_DELAY, &xResult);
		if(xResult == pdTRUE)
		{
			if(j) GPIOE->ODR |= 1<<9;
			else GPIOE->ODR &= ~ (1<<9);
		}
	}

	crEND();
}

void BUTTON(xCoRoutineHandle xHandle, unsigned portBASE_TYPE uxIndex)
{
	portBASE_TYPE xResult;

	crSTART(xHandle);

	while(1)
	{
		if( (GPIOA->IDR & (1<<0) == 1) && (f == 0))
		{
			crDELAY(xHandle, 100);
			if((GPIOA->IDR & (1<<0) == 1) && (f == 0))
			{
				i ^= 1;
				f = 1;
				crQUEUE_SEND(xHandle, queue, &i, portMAX_DELAY, &xResult);
			}
		}

		if((GPIOA->IDR & (1<<0) == 0) && (f==1))
		{
			f = 0;
		}

		crDELAY(xHandle, 100);
	}

	crEND();
}

int main(void)
{
	Port_Init();
	queue = xQueueCreate(1, sizeof(unsigned char));

	xCoRoutineCreate(LED, 1, 0); // 1 - приоритет, 0 - индекс сопрограммы (если создано несколько сопрограмм по одной фу-ии)
	xCoRoutineCreate(BUTTON, 1, 0);

	vTaskStartScheduler(); // запуск операционной системы (нужен IdleTask)
    while(1)
    {
    }
}

void vApplicationIdleHook ( void )
{
	while(1)
	{
		vCoRoutineSchedule();
	}
}

void vApplicationMallocFailedHook ( void ){for ( ;; );}
void vApplicationStackOverflowHook ( xTaskHandle pxTask, char *pcTaskName ){
	( void ) pcTaskName;
	( void ) pxTask;
	for ( ;; );
	}
//void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName );
void vApplicationTickHook ( void ){}
