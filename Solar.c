// Sun Tracking Solar Panel
// Mete Han Kahraman            21485438
// Diyala Nabeel Ata Erekat     21403012

//Digital Output PB0 - 5 - connected to LEDs
//Analog Input PE1 - connected to pot

#include "..//tm4c123gh6pm.h"
#include "TExaS.h"
#include <math.h>

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void  WaitForInterrupt(void);	
void PortD_Init(void);				
void SysTick_Init(void); 			// Initialize SysTick
void PortB_Init(void);
void PortF_Init(void);
void ADC_Init(void);
void ADC_In9821(volatile int *ain3, volatile int *ain2, volatile int *ain1, volatile int *ain0);
unsigned long ADC0_InSeq3(void);
unsigned long PulseState = 0x1;
unsigned long H1 = 150;
unsigned long H2 = 150;
unsigned long counter = 0;
unsigned long PulseWidth = 2000; 
unsigned long Degree0H = 100;
unsigned long Degree180H = 200;;
unsigned long Degree90H = 150;
unsigned long bottom = 0xFF;
unsigned long _1degree = 1;
volatile int In9 = 0;
volatile int In1 = 0;
volatile int In2 = 0;
volatile int In8 = 0;
unsigned long Hcounter1 = 0;
unsigned long Hcounter2 = 0;
unsigned long PulseCounter = 0;
int letThereBeLight = 0;
int main(void){


	DisableInterrupts();
    SysTick_Init(); //intialize SysTick
		PortD_Init();
		PortF_Init();
		ADC_Init();
    EnableInterrupts();     //  enable after everything initialized
    while(1){
            WaitForInterrupt(); // interrupts every time 10ms passes or on the falling edge of PC4
    }
}

void SysTick_Init(){
  NVIC_ST_CTRL_R = 0;               // disables SysTick during setup
  NVIC_ST_CURRENT_R = 0;            // Clears any write to current
  NVIC_ST_RELOAD_R = 160 - 1;      
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0xA0000000; //with priority 5
  NVIC_ST_CTRL_R = 0x07;                    // enable SysTick with core clock and interrupts
}

//Port D initialization
void PortD_Init(void){
		volatile unsigned long delay;
		SYSCTL_RCGC2_R  |= 0x08;	
		delay = SYSCTL_RCGC2_R;
		GPIO_PORTD_DIR_R |= 0x03;  // make PD0,1 output 
		GPIO_PORTD_DEN_R |= 0x03;   // enable digital I/O 
		GPIO_PORTD_AFSEL_R = 0;			//no alternate func
		GPIO_PORTD_AMSEL_R &= ~0x03; // disable analog
}
void PortF_Init(void){
		volatile unsigned long delay;
		SYSCTL_RCGC2_R  |= 0x20;	
		delay = SYSCTL_RCGC2_R;
		GPIO_PORTF_DIR_R |= 0x0E;  // make PD0,1,2,3 output 
		GPIO_PORTF_DEN_R |= 0x0E;   // enable digital I/O 
		GPIO_PORTF_AFSEL_R = 0;			//no alternate func
		GPIO_PORTF_AMSEL_R &= ~0x0E; // disable analog
}


void SysTick_Handler(void){
	if(Hcounter1>0)
		Hcounter1--;
	else
		GPIO_PORTD_DATA_R &= ~0x02;
	if(Hcounter2>0)
		Hcounter2--;
	else
		GPIO_PORTD_DATA_R &= ~0x01;
	
	if(PulseCounter > 0){
		PulseCounter--;
	}else{
		GPIO_PORTD_DATA_R = 0x3;
		counter++;
		if(counter == 20){
			counter = 0;
			ADC_In9821(&In9,&In8, &In2 , &In1);
			if (!((fabs(In8 - In9) < 400))){
				if(In9 > In8){
					bottom = In8;
					if(H1 < Degree180H)
						H1+=1;
				}
				if(In9 < In8){
					bottom = In9;
					if(H1> Degree0H)
						H1-=1;
				}
				if(In9 == In8)
					bottom = In9;
			}
			if(!((fabs(bottom - In1) < 400))){
				if(bottom < In1){
					if(H2 < Degree180H)
						H2+=1;
				}
				if(bottom > In1){
					if(H2 > Degree0H)
						H2-=1;
				}
			}
		}
		PulseCounter = PulseWidth - 1;
		Hcounter1 = H1 - 1;
		Hcounter2 = H2 - 1;
	}
}



//Port E intialization 
void ADC_Init(void){
  volatile unsigned long delay;
  SYSCTL_RCGCADC_R |= 0x00000001; // 1) activate ADC0
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4; // 1) activate clock for Port E
  delay = SYSCTL_RCGCGPIO_R;      // 2) allow time for clock to stabilize
  delay = SYSCTL_RCGCGPIO_R;
  GPIO_PORTE_DIR_R &= ~0x36;      // 3) make PE1, PE2, PE4, PE5 inputs  
  GPIO_PORTE_AFSEL_R |= 0x36;     // 4) enable alternate function on PE0, PE1, PE2 , PE3
  GPIO_PORTE_DEN_R &= ~0x36;      // 5) disable digital I/O on PE0, PE1, PE2, and PE3
                                  
  GPIO_PORTE_PCTL_R = GPIO_PORTE_PCTL_R&0xFFFFF00F;
  GPIO_PORTE_AMSEL_R |= 0x36;     // 6) enable analog functionality on PE0, PE1, PE2, PE3
  ADC0_PC_R &= ~0xF;              // 8) clear max sample rate field
  ADC0_PC_R |= 0x1;               //    configure for 125K samples/sec
  ADC0_SSPRI_R = 0x3210;          // 9) Sequencer 2 is lowest priority
  ADC0_ACTSS_R &= ~0x0004;        // 10) disable sample sequencer 2
  ADC0_EMUX_R &= ~0x0F00;         // 11) seq2 is software trigger
  ADC0_SSMUX2_R = 0x1289;         // 12) set channels for SS2
  ADC0_SSCTL2_R = 0x0600;         // 13) no D0 END0 IE0 TS0 D1 END1 IE1 TS1 D2 TS2, yes END2 IE2
  ADC0_IM_R &= ~0x0004;           // 14) disable SS2 interrupts
  ADC0_ACTSS_R |= 0x0004;         // 15) enable sample sequencer 2
}

//------------ADC_In298------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: three 12-bit result of ADC conversions
// Samples AIN1, and AIN2
// 125k max sampling
// software trigger, busy-wait sampling
// data returned by reference
// ain2 (PE1) 0 to 4095
// ain1 (PE2) 0 to 4095
void ADC_In9821(volatile int *ain9, volatile int *ain8, volatile int *ain2, volatile int *ain1){
  ADC0_PSSI_R = 0x0004;            // 1) initiate SS2
  while((ADC0_RIS_R&0x04)==0){};   // 2) wait for conversion done
	*ain9 = ADC0_SSFIFO2_R&0xFFF;    // 3B) read first result
  *ain8 = ADC0_SSFIFO2_R&0xFFF;    // 3A) read second result
  *ain2 = ADC0_SSFIFO2_R&0xFFF;    // 3B) read third result
	*ain1 = ADC0_SSFIFO2_R&0xFFF;    // 3B) read forth result
  ADC0_ISC_R = 0x0004;             // 4) acknowledge completion
}
