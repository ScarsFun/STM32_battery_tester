/*----------------------------------------------------------------------------
*   CMSIS-RTOS battery tester
*   buttons : PB12, PB13, PB14
*
*   NOKIA 3310 Pins
*   LCD_PORT          GPIOA
*   LCD_DC_PIN        GPIO_Pin_12
*   LCD_CE_PIN        GPIO_Pin_11
*   LCD_RST_PIN       GPIO_Pin_10
*   LCD_SDIN_PIN      GPIO_Pin_7
*   LCD_SCLK_PIN      GPIO_Pin_5
*
*   ADC Pins: 
*   PA1 : Vbattery 
*   PA2 : V charge monitor TP4056 pin 2
*
*   output pins:
*   PA8 : enable IRL3103
*   PA9 : enable TP4056 
*
*---------------------------------------------------------------------------*/
/*
if (on_off == 1)
{
__asm {bkpt 0};
}
*/

#define osObjectsPublic // define objects in main module
#include "osObjects.h" // RTOS object definitions
#include "stm32f10x.h"
#include "rl_usb.h"
#include "uc_memory.h"
#include "n3310.h"
#include "buttons.h"
#include "ADC.h"
#include "outpins.h"
#include <stdio.h>
#include <string.h>

const uint32_t LOAD_RESISTANCE = 2400; //mOhm
extern volatile uint32_t button_1, button_2, button_3; //  PB12, PB13
extern uint16_t ADC_ConvertedValue[2]; // ADC1, ADC2 (PA1,PA2 on STM32):  PA1 = V_Batt, PA2 = -V_Charger
// 10KOhm trimmer on v_batt 5V scale
uint32_t elapsed_seconds = 0, elapsed_minutes = 0, battVoltage = 0, battVoltageMedia = 0, milliAmpereSecond = 0, IcVoltage;
char tempString[7];
float volts;

uint32_t VirtAddVarTab = 0x0; //starting virtual address
uint16_t VarValue = 0;

extern uint8_t uart_tx_buf[64];
extern uint8_t uart_rx_buf[64];

static uint8_t deviceKey[] = { "XYZ" };

void Timer1_Callback(void const* arg);
osTimerId Timer1_id; // timer id
uint32_t exec1;
osTimerDef(Timer1, Timer1_Callback);

void control_Thread(void const* argument);
osThreadDef(control_Thread, osPriorityNormal, 1, 0);
osThreadId T_control_Thread;

void discharge_Thread(void const* argument);
osThreadDef(discharge_Thread, osPriorityNormal, 1, 0);
osThreadId T_discharge_Thread;

void Terminal_Thread(void const* argument);
osThreadDef(Terminal_Thread, osPriorityNormal, 1, 0);
osThreadId T_Terminal_Thread;

void DataSend_Thread(void const* argument);
osThreadDef(DataSend_Thread, osPriorityNormal, 1, 0);
osThreadId T_DataSend_Thread;

static void Timer1_Callback(void const* arg)
{
    osSignalSet(T_discharge_Thread, 0x01);
}

void DataSend_Thread(void const* argument)
{
    //uint16_t index;
    uint8_t ValueToSend[5];
    VirtAddVarTab = 0x0;
    while (VarValue != 0xFFFE) {

        osDelay(30);
        VarValue = FEE_ReadDataHalfWord(VirtAddVarTab);
        sprintf((char*)ValueToSend, "%d", VarValue);
        if (VarValue != 0xFFFE)
            USBD_CDC_ACM_WriteData(0U, ValueToSend, 5);
        VirtAddVarTab++;
    };
    osDelay(100);
    USBD_CDC_ACM_WriteData(0U, "99999", 6); //end of series
    //   USBD_CDC_ACM_WriteData(0U, "END", 3);
    USBD_Disconnect(0U); /* USB Device 0 Connect */
    USBD_Uninitialize(0U);
    osDelay(50);
    LcdClear();
    LcdGotoXYFont(1, 2);
    LcdStr(FONT_2X, (unsigned char*)"..OK..");
    LcdGotoXYFont(1, 4);
    LcdStr(FONT_1X, (unsigned char*)"transfer");
    LcdUpdate();
    osThreadTerminate(T_Terminal_Thread);
    osThreadTerminate(T_DataSend_Thread);
}

void Terminal_Thread(void const* arg)
{
    USBD_Initialize(0U); /* USB Device 0 Initialization */
    USBD_Connect(0U); /* USB Device 0 Connect */
    for (;;) {
        osSignalWait(0x01, osWaitForever);
        if (uart_rx_buf[0] == 'w' && uart_rx_buf[1] == 'h' && uart_rx_buf[2] == 'o') {
            GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
            USBD_CDC_ACM_WriteData(0U, deviceKey, 3);
        }
        else if (uart_rx_buf[0] == 'S')
            T_DataSend_Thread = osThreadCreate(osThread(DataSend_Thread), NULL);
        else if (uart_rx_buf[0] == 'E') {
            osThreadTerminate(T_DataSend_Thread);
            USBD_CDC_ACM_WriteData(0U, "END", 3);
            USBD_Disconnect(0U); /* USB Device 0 Connect */
            USBD_Uninitialize(0U);
            osDelay(50);
            LcdClear();
            LcdGotoXYFont(1, 2);
            LcdStr(FONT_2X, (unsigned char*)"..OK..");
            LcdGotoXYFont(1, 4);
            LcdStr(FONT_1X, (unsigned char*)"transfer");
            LcdUpdate();
            osThreadTerminate(T_Terminal_Thread);
        }
    }
}
void discharge_Thread(void const* arg)

{

    LcdClear();
    LcdGotoXYFont(1, 3);
    LcdStr(FONT_1X, (unsigned char*)"Check Battery!");
    LcdUpdate();
    //check charging current
    while (ADC_ConvertedValue[0] < 30)
        osDelay(500);
    LcdClear();
    LcdGotoXYFont(1, 0);
    LcdStr(FONT_1X, (unsigned char*)"DISCHARGING");
    LcdGotoXYFont(1, 1);
    LcdStr(FONT_1X, (unsigned char*)"time: ");
    LcdGotoXYFont(1, 2);
    LcdStr(FONT_1X, (unsigned char*)"V_Batt = ");
    LcdGotoXYFont(1, 4);
    LcdStr(FONT_1X, (unsigned char*)"mAh = ");
    LcdUpdate();
    Timer1_id = osTimerCreate(osTimer(Timer1), osTimerPeriodic, NULL);
    osTimerStart(Timer1_id, 1000);
    GPIO_SetBits(GPIOA, GPIO_Pin_8); //  enable FET
    while (ADC_ConvertedValue[0] > 2210) // 2210 stop discharge if vbatt < 2.7v
    {
        osSignalWait(0x0001, osWaitForever);
        /*
        //-------------------display ADC value ------------------
        LcdGotoXYFont(0, 0);
        LcdStr(FONT_1X, (unsigned char*)"     ");
        sprintf(tempString, "%4d", ADC_ConvertedValue[0]);
        LcdGotoXYFont(0, 0);
        LcdStr(FONT_1X, (unsigned char*)tempString);
		*/
        //-------------------display battery voltage--------------------
        battVoltage = (ADC_ConvertedValue[0]) * 1221; // 5000mV / 4096 = 1221 uV per step
        battVoltageMedia += battVoltage;
        LcdGotoXYFont(10, 2);
        LcdStr(FONT_1X, (unsigned char*)"     ");
        volts = (float)battVoltage / 1000000;
        sprintf(tempString, "%.2f", volts);
        LcdGotoXYFont(10, 2);
        LcdStr(FONT_1X, (unsigned char*)tempString);
        // -----------------display mAh-------------------
        LcdGotoXYFont(5, 4);
        LcdStr(FONT_2X, (unsigned char*)"    ");
        milliAmpereSecond += (battVoltage / LOAD_RESISTANCE); //milliampere per second
        // convert milliampere per second to milliampere per hour
        sprintf(tempString, "%4d", milliAmpereSecond / 3600);
        LcdGotoXYFont(5, 4);
        LcdStr(FONT_2X, (unsigned char*)tempString);
        LcdUpdate();
        //------------- time elapsed -------------------
        elapsed_seconds++;

        // store current values every 10 seconds
        if (elapsed_seconds % 10 == 0) {
            if ((uint32_t)arg)
                FEE_WriteDataHalfWord(VirtAddVarTab, (battVoltageMedia / 10 / 1000)); //save millvolt
            VirtAddVarTab++;
            battVoltageMedia = 0;
        }

        if (elapsed_seconds == 60) {
            elapsed_seconds = 0;
            elapsed_minutes++;
        }
        LcdGotoXYFont(7, 1);
        LcdStr(FONT_1X, (unsigned char*)"      ");
        sprintf(tempString, "%3d:%2d", elapsed_minutes, elapsed_seconds);
        LcdGotoXYFont(7, 1);
        LcdStr(FONT_1X, (unsigned char*)tempString);
    }
    osTimerStop(Timer1_id);
    osTimerDelete(Timer1_id);
    GPIO_ResetBits(GPIOA, GPIO_Pin_8); //  disable FET
    LcdGotoXYFont(1, 0);
    LcdStr(FONT_1X, (unsigned char*)"COMPLETED     ");
    LcdUpdate();
    if ((uint32_t)arg)
        FEE_WriteDataHalfWord(VirtAddVarTab, 0xFFFE); // write to last flash address END key

    osThreadTerminate(T_discharge_Thread);
}

void control_Thread(void const* argument)

{
    LcdClear();
    LcdGotoXYFont(5, 2);
    LcdStr(FONT_1X, (unsigned char*)"STM32");
    LcdGotoXYFont(0, 3);
    LcdStr(FONT_1X, (unsigned char*)"Battery Tester");
    LcdGotoXYFont(5, 5);
    LcdStr(FONT_1X, (unsigned char*)"V0.99");
    LcdUpdate();
    EXTI_ClearITPendingBit(EXTI_Line12 | EXTI_Line13 | EXTI_Line14);
    buttons_IRQ_ENABLE();
    for (;;) {
        osSignalWait(0x01, osWaitForever);
        if (button_1) {
            LcdClear();
            LcdGotoXYFont(3, 2);
            switch (button_1) {
            case 1:
                LcdStr(FONT_1X, (unsigned char*)"DISCHARGE");
                break;
            case 2:
                LcdStr(FONT_1X, (unsigned char*)"DISCHARGE");
                LcdGotoXYFont(7, 3);
                LcdStr(FONT_1X, (unsigned char*)"&");
                LcdGotoXYFont(3, 4);
                LcdStr(FONT_1X, (unsigned char*)"SAVE DATA");
                break;
            case 3:
                LcdStr(FONT_1X, (unsigned char*)"DATA to USB");
                break;
            }
        }
        if (button_2 && (button_1 != 0)) {
            //LcdClear();
            LcdGotoXYFont(1, 5);
            LcdStr(FONT_1X, (unsigned char*)"START...");
            LcdUpdate();
            button_2 = 0;
            osDelay(1000);
            switch (button_1) {
            case 1:
                T_discharge_Thread = osThreadCreate(osThread(discharge_Thread), (void*)0);
                break;
            case 2:
                //discharge();
                T_discharge_Thread = osThreadCreate(osThread(discharge_Thread), (void*)1);
                FEE_Init();
                FEE_Erase();
                break;
            case 3:
                LcdGotoXYFont(1, 5);
                LcdStr(FONT_1X, (unsigned char*)"Connect USB");
                LcdUpdate();

                T_Terminal_Thread = osThreadCreate(osThread(Terminal_Thread), NULL);
                break;
            }
            buttons_IRQ_DISABLE();
        }
        /*
            if (button_3) {
                LcdClear();
                LcdGotoXYFont(1, 2);
                LcdStr(FONT_1X, (unsigned char*)"Button 3");
                button_3 = 0;
            } */
        //
        LcdUpdate();
        osDelay(500);
    }
}

/*
 * main: initialize and start the system
 */
int main(void)
{

    osKernelInitialize(); // initialize CMSIS-RTOS
    // initialize peripherals here
    OutPins_Init();
    buttons_init();
    Adc_Init();
    LcdInit();

    // create 'thread' functions that start executing,
    // example: tid_name = osThreadCreate (osThread(name), NULL);
    T_control_Thread = osThreadCreate(osThread(control_Thread), NULL);
    //T_Terminal_Thread = osThreadCreate(osThread(Terminal_Thread), NULL);
    //USBD_Initialize(0U); /* USB Device 0 Initialization */
    //USBD_Connect(0U); /* USB Device 0 Connect */
    osDelay(200);
    osKernelStart(); // start thread execution
}
