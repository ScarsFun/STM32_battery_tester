#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

void OutPins_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	 // Configure PA8, PA9, as push-pull output 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_ResetBits(GPIOA, GPIO_Pin_8 | GPIO_Pin_9);
    /*
    // Configure PC13, as push-pull output
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13 ;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
    */
    
}
