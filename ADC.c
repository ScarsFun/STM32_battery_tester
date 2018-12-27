#include "stm32f10x.h"

uint16_t ADC_ConvertedValue[2] = { 0, 0 };

void Adc_Init(void)
{
   

    /* Enable ADC1 and GPIOA clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    /* DMA Channel1 Configuration ----------------------------------------------*/
    DMA_InitTypeDef DMA_InitStructure;
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADC_ConvertedValue;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 2;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    /* Enable DMA Channel1 */
    DMA_Cmd(DMA1_Channel1, ENABLE);

    //ADC
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    // input of ADC (it doesn't seem to be needed, as default GPIO state is floating input)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2; // that's ADC1, ADC2 (PA1,PA2 on STM32)
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //clock for ADC (max 14MHz --> 72/6=12MHz)
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);

    // define ADC config
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; // we work in continuous sampling mode
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 2;

    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_13Cycles5); // define regular conversion config
    ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2, ADC_SampleTime_13Cycles5); // define regular conversion config
    ADC_Init(ADC1, &ADC_InitStructure); //set config of ADC1

    /* Enable ADCx's DMA interface */
    ADC_DMACmd(ADC1, ENABLE);
    // enable ADC
    ADC_Cmd(ADC1, ENABLE); //enable ADC1

    //	ADC calibration (optional, but recommended at power on)
    ADC_ResetCalibration(ADC1); // Reset previous calibration
    while (ADC_GetResetCalibrationStatus(ADC1))
        ;
    ADC_StartCalibration(ADC1); // Start new calibration (ADC must be off at that time)
    while (ADC_GetCalibrationStatus(ADC1))
        ;

    // start conversion
    ADC_Cmd(ADC1, ENABLE); //enable ADC1

    ADC_SoftwareStartConvCmd(ADC1, ENABLE); // start conversion (will be endless as we are in continuous mode)
}
