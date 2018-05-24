/*********************Smart Home System using MPC2140***********************/
/********************************By Irfanuddin******************************/
/*The system will monitor the temperature and show it using the Onboard LEDs. 
When the temperature rises beyond 7 points, there will be a warning sound.*/
/*The ADC POT in the board is used instead of actual temperature Sensor*/

                  
#include <LPC213x.H>                        /* LPC213x definitions            */

#define CPU_CLOCK   60000000                /* CPU Clock                      */
#define DATA_FREQ       8000                /* Sound Data Frequency           */

#define VPB_CLOCK   (CPU_CLOCK/4)           /* VPB Clock                      */
#define TIM_TICK    (VPB_CLOCK/DATA_FREQ)   /* Timer Tick                     */


extern const char SoundData[];              /* Sound Data (Signed 16-bit)     */
extern const int  SoundCount;               /* Sound Data Count               */

unsigned int  Index  = 0;                   /* Sound Data Index               */
unsigned char Temperature = 0;              /*Temperature Level             */

/*Array of LED initialization*/
const    char LEDTemp[8] = {							
  0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF
};


/* Timer Counter 0 Interrupt executes @ DATA_FREQ Rate                        */
__irq void tc0 (void) {
  unsigned long val;
  here:
  if (Index < SoundCount) {                 /* Check if Data is available     */
    if ((Index & 0x3F) == 0) {              /* On every 64th Sample           */
      AD0CR |= 0x01000000;                  /* Start A/D Conversion           */
      do {
        val = AD0DR;                        /* Read A/D Data Register         */
      } while ((val & 0x80000000) == 0);    /* Wait for end of A/D Conversion */
      AD0CR &= ~0x01000000;                 /* Stop A/D Conversion            */
      Temperature = val >> 8;                    /* Extract Temperature Level (0-255)   */
			
      IOCLR1 = 0x00FF0000;                  /* Turn Off all LEDs              */
      IOSET1 = LEDTemp[Temperature >> 5] << 16;   /* LEDs show Temperature Level         */
    }
    val    = 0x8000;                        /* Sound Data: Middle Point       */
    val   += SoundData[Index];              /* Sound Data:  Lower 8-bits      */
    val   += SoundData[Index + 1] << 8;     /* Sound Data: Higher 8-bits      */
    val   &= 0xFFFF;
    val   *= Temperature;                        /* Temperature Level             */
    val  >>= 8;
		val = val;
		if(Temperature > 200){
		    DACR   = val;  }
		else
				goto here;													/* Set Speaker Output             */   
    Index += 2;                             /* Next Sample                    */
  } else {
    IOCLR1 = 0x00FF0000;                    /* Turn Off all LEDs              */
  }
  T0IR        = 1;                          /* Clear interrupt flag           */
  VICVectAddr = 0;                          /* Acknowledge Interrupt          */
}


int main (void) {

  PINSEL1 = 0x01080000;                     /* Select DAC (AOUT) & AIN1       */
  IODIR1  = 0x00FF0000;                     /* P1.16..23 defined as Outputs   */

  AD0CR   = 0x00200402;                     /* ADC1: 10-bit AIN1 @ 3MHz       */ 
  DACR    = 0x00008000;                     /* DAC Output set to Middle Point */

  /* Setup Timer 0: Periodic Interrupt @ DATA_FREQ Rate                       */
  T0MR0 = TIM_TICK - 1;                     /* Timer0 Match Value 0           */
  T0MCR = 3;                                /* Interrupt and Reset on MR0     */
  T0TCR = 1;                                /* Timer0 Enable                  */
  VICVectAddr0 = (unsigned long)tc0;        /* Set Interrupt Vector in 0      */
  VICVectCntl0 = 0x20 | 4;                  /* Use it for Timer0 Interrupt    */
  VICIntEnable = 0x00000010;                /* Enable Timer0 Interrupt        */

  while (1) {                               /* Loop forever                   */
    if (Index == SoundCount) {              /* Check if Sound is over         */
          
        Index = 0;                          /* Play the Sound again           */
      
    }
  }
}
