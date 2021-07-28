//Generating PWM at 20ms (50Hz), with pulse widths of 1ms, 1.5ms and 2ms.
#include <mkl25z4.h>
void init_LED(void);
void init_Timer(void);
void init_pin(void);

/*Uncomment only one of these to select your desired pulse width*/
int pulse_width = 0xA4; //1ms
//int	pulse_width = 0xF6; //1.5ms
//int	pulse_width = 0x148; //2ms

#define EXT1		(3)		//PTD3
#define MASK(X)  (1<<X)
#define PTB2		(2)		//PTB2 to monitor on scope

int main()
{
	init_LED();
	init_Timer();
	init_pin();
	while(1){}
}

void init_LED()
{
	SIM->SCGC5 |=SIM_SCGC5_PORTB_MASK;
	PORTB->PCR[PTB2 ] &= ~PORT_PCR_MUX_MASK;	//Clear mux
	PORTB->PCR[PTB2] |= PORT_PCR_MUX(3);	//***setup to be output of TPM2_CH0****
}

void init_Timer()
{
	//Clock gate
	SIM->SCGC6 |=SIM_SCGC6_TPM2_MASK;	//*******TPM2 channel 0
	//Select clock source in SIM_SOPT
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);	//1- MCGPLLCLK/2, or MCGFLLCLK 01 (ext?), 2-ext OSCERCLK, 3-internal MCGIRCLK
	//Configure registers
	TPM2->MOD= 0xCCD; //20ms

	//working with TPM2_C0SC
	//output compare + edge aligned PWM MSBA: 10, ELSBA:10
	TPM2->CONTROLS[0].CnSC |= TPM_CnSC_MSB(1) | TPM_CnSC_ELSB(1);
	TPM2->CONTROLS[0].CnSC |= TPM_CnSC_CHF_MASK;  //clear spurious interrupts
	TPM2->CONTROLS[0].CnV = pulse_width;	//pulse width defined
	TPM2->SC |=  TPM_SC_TOF_MASK | TPM_SC_PS(7) | TPM_SC_TOIE_MASK  ;
	TPM2->SC |= TPM_SC_CMOD(1); //enable internal clock to run

	NVIC_ClearPendingIRQ(TPM2_IRQn);
	NVIC_SetPriority(TPM2_IRQn, 3);
	NVIC_EnableIRQ(TPM2_IRQn);
}

void TPM2_IRQHandler()
{
	//optionally check the flag
	if (TPM2->STATUS & TPM_STATUS_CH0F_MASK)
	{
		TPM2->CONTROLS[0].CnSC |=TPM_CnSC_CHF_MASK;//clear flag
	}
	PTD->PTOR |= MASK(EXT1 ) ; //THis is for debugging purposes. not needed.
	TPM2->SC |= TPM_SC_TOF_MASK ; //clear the interrupt
}

void init_pin()
{
	SIM->SCGC5 |=SIM_SCGC5_PORTD_MASK;
	PORTD->PCR[EXT1] &= ~PORT_PCR_MUX_MASK;	//Clear mux
	PORTD->PCR[EXT1] |= PORT_PCR_MUX(1);	//setup to be GPIO
	PTD->PDDR |= MASK(EXT1 ) ;
	PTD->PCOR |= MASK(EXT1 ) ; //set to 0
}
