#include "buttons.h"
#include "ADC.h"
#include "outpins.h"

volatile uint32_t button_1=0, button_2=0, button_3=0;
extern osThreadId T_control_Thread;

void  buttons_init(void)
{
    

    GPIO_InitTypeDef   GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  	/* Configure PB.12, PB.13, PB.14 pin as input floating */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 |  GPIO_Pin_14;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   //input pull up
  	GPIO_Init(GPIOB, &GPIO_InitStructure);
  	/* Enable AFIO clock */

    EXTI_InitTypeDef EXTI_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    /* Connect EXTI12, EXTI13, EXTI14 Line to PB12, PB13, PB14 pin */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource12);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource13);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);
    /* Configure EXTI line */
    EXTI_InitStructure.EXTI_Line = EXTI_Line12 | EXTI_Line13 | EXTI_Line14;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    /* Disable AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, DISABLE);
	
	NVIC_InitTypeDef NVIC_InitStructure;
    /* Enable and set EXTI0 Interrupt to the lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);
	
}

void EXTI15_10_IRQHandler()
{
    if (EXTI_GetITStatus(EXTI_Line12) != RESET) {
    	button_1+=1;
		  if (button_1==4) button_1 = 1;
      //button_2 = 0;
    }
    if (EXTI_GetITStatus(EXTI_Line13) != RESET) {
    	button_2=1;
    }
    if (EXTI_GetITStatus(EXTI_Line14) != RESET) {
    	button_3=1;
    }
     /* Clear the  EXTI  pending bits */
     EXTI_ClearITPendingBit(EXTI_Line12 | EXTI_Line13 | EXTI_Line14);
     osSignalSet(T_control_Thread, 0x01); // button triggered. release thread;
}

void buttons_IRQ_DISABLE(void)
{
    NVIC_DisableIRQ(EXTI15_10_IRQn);
}

void buttons_IRQ_ENABLE(void)
{
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}
