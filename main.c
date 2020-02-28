#include "Globals.h"
#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include <stdbool.h>
#include "ADC_ISR.h"
#include "controller.h"
#include "speed.h"


// Prototype statements for functions found within this file.
//#pragma CODE_SECTION(epwm1_isr, "ramfuncs");
interrupt void epwm1_isr(void);

//#pragma CODE_SECTION(HallA_isr, "ramfuncs");
//#pragma CODE_SECTION(HallB_isr, "ramfuncs");
//#pragma CODE_SECTION(HallC_isr, "ramfuncs");
interrupt void HallA_isr(void);
interrupt void HallB_isr(void);
interrupt void HallC_isr(void);


//global vars for inputs
volatile unsigned int previous3pos=0;
Uint32 delay = 0; //for blinking led


void configureGPIO(){
    EALLOW;
    //debug led
    GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0;
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;

    //Enable pin
    GpioCtrlRegs.GPAPUD.bit.GPIO13 = 1;          // Disable pull up for enable pin
    GpioDataRegs.GPACLEAR.bit.GPIO13 = 1;        // set low (disable)
    GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 0;         // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO13 = 1;          // output

    //Three pos switch
    GpioCtrlRegs.GPAPUD.bit.GPIO9 = 1;          // Disable pull up for 3pos yellow-yellow down low
    GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 0;         // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO9 = 0;          // input

    GpioCtrlRegs.GPAPUD.bit.GPIO10 = 1;          // Disable pull up for 3pos green-green up low
    GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 0;         // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO10 = 0;          // input


    //HALL SENSORS    GPIO6 and GPIO7 and GPIO 8 are inputs
    //EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 0;         // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO6 = 0;          // input
    GpioCtrlRegs.GPAQSEL1.bit.GPIO6 = 0;        // XINT1 Synch to SYSCLKOUT only

    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 0;         // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO7 = 0;          // input
    GpioCtrlRegs.GPAQSEL1.bit.GPIO7 = 0;        // XINT2 Synch to SYSCLKOUT only

    GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 0;         // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO8 = 0;          // input
    GpioCtrlRegs.GPAQSEL1.bit.GPIO8 = 0;        // XINT3 Synch to SYSCLKOUT only
    //GpioCtrlRegs.GPACTRL.bit.QUALPRD0 = 0xFF;   // Each sampling window is 510*SYSCLKOUT


    //Nothing for ADC

   // PWM

   GpioCtrlRegs.GPAPUD.bit.GPIO0 = 1;    // Disable pull-up on GPIO0 (EPWM1A)
   GpioCtrlRegs.GPAPUD.bit.GPIO1 = 1;    // Disable pull-up on GPIO1 (EPWM1B)
   GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;   // Configure GPIO0 as EPWM1A
   GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1;   // Configure GPIO1 as EPWM1B


   GpioCtrlRegs.GPAPUD.bit.GPIO2 = 1;    // Disable pull-up on GPIO2 (EPWM2A)
   GpioCtrlRegs.GPAPUD.bit.GPIO3 = 1;    // Disable pull-up on GPIO3 (EPWM2B)
   GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;   // Configure GPIO2 as EPWM2A
   GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1;   // Configure GPIO3 as EPWM2B


   GpioCtrlRegs.GPAPUD.bit.GPIO4 = 1;    // Disable pull-up on GPIO4 (EPWM3A)
   GpioCtrlRegs.GPAPUD.bit.GPIO5 = 1;    // Disable pull-up on GPIO5 (EPWM3B)
   GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 1;   // Configure GPIO4 as EPWM3A
   GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 1;   // Configure GPIO5 as EPWM3B

   EDIS;
}

void InitEPwm50khz(void)
{

    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;

   // Setup TBCLK

    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Count up/down
    EPwm1Regs.TBPRD = MAX_PWM;       // Set timer period
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;    // Disable phase loading
    EPwm1Regs.TBPHS.half.TBPHS = 0x0000;       // Phase is 0
    EPwm1Regs.TBCTR = 0x0000;                  // Clear counter
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;   // Clock ratio to SYSCLKOUT
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm1Regs.TBCTL.bit.FREE_SOFT = 2; //free run on debug
    EPwm1Regs.TBCTL.bit.PRDLD = TB_SHADOW;

    // Setup shadow register load on ZERO
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    // Set Compare values
    EPwm1Regs.CMPA.half.CMPA = 0;    // Set compare A value
    EPwm1Regs.CMPB = MAX_PWM;              // Set Compare B value

    // Set actions (active low)
    EPwm1Regs.AQCTLA.bit.CAD = AQ_CLEAR;           // Clear PWM1A on Zero         Swap these?
    EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;            // Set PWM1A on event A, up count

    EPwm1Regs.AQCTLB.bit.CBD = AQ_SET;            // Clear PWM1B on Zero
    EPwm1Regs.AQCTLB.bit.CBU = AQ_CLEAR;          // Set PWM1B on event B, up count

    // Setup TBCLK
    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Count up/down
    EPwm2Regs.TBPRD = MAX_PWM;       // Set timer period
    EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE;    // Disable phase loading
    EPwm2Regs.TBPHS.half.TBPHS = 0x0000;       // Phase is 0
    EPwm2Regs.TBCTR = 0x0000;                  // Clear counter
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;   // Clock ratio to SYSCLKOUT
    EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm2Regs.TBCTL.bit.FREE_SOFT = 2; //free run on debug
    EPwm2Regs.TBCTL.bit.PRDLD = TB_SHADOW;

   // Setup shadow register load on ZERO
    EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

   // Set Compare values
    EPwm2Regs.CMPA.half.CMPA = 0;    // Set compare A value
    EPwm2Regs.CMPB = MAX_PWM;              // Set Compare B value

    // Set actions (active low)
    EPwm2Regs.AQCTLA.bit.CAD = AQ_CLEAR;           // Clear PWM1A on Zero         Swap these?
    EPwm2Regs.AQCTLA.bit.CAU = AQ_SET;            // Set PWM1A on event A, up count

    EPwm2Regs.AQCTLB.bit.CBD = AQ_SET;            // Clear PWM1B on Zero
    EPwm2Regs.AQCTLB.bit.CBU = AQ_CLEAR;          // Set PWM1B on event B, up count

    // Setup TBCLK
    EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Count up
    EPwm3Regs.TBPRD = MAX_PWM;       // Set timer period
    EPwm3Regs.TBCTL.bit.PHSEN = TB_DISABLE;    // Disable phase loading
    EPwm3Regs.TBPHS.half.TBPHS = 0x0000;       // Phase is 0
    EPwm3Regs.TBCTR = 0x0000;                  // Clear counter
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;   // Clock ratio to SYSCLKOUT
    EPwm3Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm3Regs.TBCTL.bit.FREE_SOFT = 2; //free run on debug
    EPwm3Regs.TBCTL.bit.PRDLD = TB_SHADOW;

   // Setup shadow register load on ZERO
    EPwm3Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm3Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm3Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm3Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

   // Set Compare values
    EPwm3Regs.CMPA.half.CMPA = 0;    // Set compare A value
    EPwm3Regs.CMPB = MAX_PWM;              // Set Compare B value

    // Set actions (active low)
    EPwm3Regs.AQCTLA.bit.CAD = AQ_CLEAR;           // Clear PWM1A on Zero         Swap these?
    EPwm3Regs.AQCTLA.bit.CAU = AQ_SET;            // Set PWM1A on event A, up count

    EPwm3Regs.AQCTLB.bit.CBD = AQ_SET;            // Clear PWM1B on Zero
    EPwm3Regs.AQCTLB.bit.CBU = AQ_CLEAR;          // Set PWM1B on event B, up count


    //PWM interrupt
    EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;     // Select INT on Zero event
    EPwm1Regs.ETSEL.bit.INTEN = 1;                // Enable INT
    EPwm1Regs.ETPS.bit.INTPRD = ET_3RD;           // Generate INT on 3rd event

    //Trigger ADC from PWM
    EPwm1Regs.ETSEL.bit.SOCAEN  = 1;        // Enable SOC on A group
    EPwm1Regs.ETSEL.bit.SOCASEL  = 1;        // Select SOC from from CPMA on 0
    EPwm1Regs.ETPS.bit.SOCAPRD   = 1;        // Generate pulse on 1st event


    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;
}


void main(void)
{

// Step 1. Initialize System Control:
   InitSysCtrl();  // PLL, WatchDog, enable Peripheral Clocks

   setGlobals();

   speedTimerSetUp();

   //memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (Uint32)&RamfuncsLoadSize); //for delay in adc init

// Step 2. Initalize GPIO:
   configureADC(); //ADCINT 1 ISR handler must be not enabled or not set to default

// Step 3. Clear all interrupts and initialize PIE vector table:
// Disable CPU interrupts
   DINT;

// Initialize the PIE control registers to their default state.
// The default state is all PIE interrupts disabled and flags
// are cleared.
// This function is found in the F2806x_PieCtrl.c file.

   InitPieCtrl();

// Disable CPU interrupts and clear all CPU interrupt flags:
   IER = 0x0000;
   IFR = 0x0000;

// Initialize the PIE vector table with pointers to the shell Interrupt
// Service Routines (ISR).
// This will populate the entire table, even if the interrupt
// is not used in this example.  This is useful for debug purposes.
// The shell ISR routines are found in F2806x_DefaultIsr.c.
// This function is found in F2806x_PieVect.c.
   InitPieVectTable();


// Interrupts that are used in this example are re-mapped to
// ISR functions found within this file.
   EALLOW;  // This is needed to write to EALLOW protected registers

   //GPIO interrupts -----------------------------
   //set interrupt handle in table
   PieVectTable.XINT1 = &HallA_isr;
   PieVectTable.XINT2 = &HallB_isr;
   PieVectTable.XINT3 = &HallC_isr;
   PieVectTable.EPWM1_INT = &epwm1_isr;
   PieVectTable.ADCINT1 = &adc_isr;

   //set interrupt pin


   // Enable XINT1 and XINT2 in the PIE: Group 1 interrupt 4 & 5
   // Enable INT1 which is connected to WAKEINT:
      PieCtrlRegs.PIECTRL.bit.ENPIE = 1;          // Enable the PIE block

     //Group 1

      //Hall A
      PieCtrlRegs.PIEIER1.bit.INTx4 = 1;          // Enable PIE Group 1 INT4
      //Hall B
      PieCtrlRegs.PIEIER1.bit.INTx5 = 1;          // Enable PIE Group 1 INT5
      //ADC
      PieCtrlRegs.PIEIER1.bit.INTx1 = 1;          // ADC Enable INT 1.1 in the PIE
      IER |= M_INT1;                              // Enable CPU INT1

      //Group 3
      //PWM
      PieCtrlRegs.PIEIER3.bit.INTx1 = 1;            //pwm 1
      IER |= M_INT3;                                //enable pwm interrupt

      //Group 12
      //Hall C
      PieCtrlRegs.PIEIER12.bit.INTx1 = 1;          // Enable PIE Group 12 INT1
      IER |= M_INT12;                              // Enable CPU INT1

      EINT;                                       // Enable Global Interrupts
      ERTM;   // Enable Global real time interrupt DBGM

     EDIS;


  // GPIO0 is XINT1, GPIO1 is XINT2
     EALLOW;
     GpioIntRegs.GPIOXINT1SEL.bit.GPIOSEL = 6;   // XINT1 is GPIO6
     GpioIntRegs.GPIOXINT2SEL.bit.GPIOSEL = 7;   // XINT2 is GPIO7
     GpioIntRegs.GPIOXINT3SEL.bit.GPIOSEL = 8;   // XINT3 is GPIO8
     EDIS;

  // Configure XINT1 and XINT2
     XIntruptRegs.XINT1CR.bit.POLARITY = 3;      // Falling/Rising edge interrupt
     XIntruptRegs.XINT2CR.bit.POLARITY = 3;      // Falling/Rising edge interrupt
     XIntruptRegs.XINT3CR.bit.POLARITY = 3;      // Falling/Rising edge interrupt

  // Enable XINT1 and XINT2 and XINT3
     XIntruptRegs.XINT1CR.bit.ENABLE = 1;        // Enable XINT1
     XIntruptRegs.XINT2CR.bit.ENABLE = 1;        // Enable XINT2
     XIntruptRegs.XINT3CR.bit.ENABLE = 1;        // Enable XINT3

   EDIS;    // This is needed to disable write to EALLOW protected registers


// Step 4. Setup PWM

   configureGPIO();

   InitEPwm50khz();

   GpioDataRegs.GPASET.bit.GPIO13 = 1; //Enable gate drivers


   for(;;)
   {
       while(delay>0){
           delay--;
       }
       delay = 300000; //about 1 per second
       GpioDataRegs.GPBTOGGLE.bit.GPIO34 = 1;
   }

}




interrupt void HallA_isr(void){
    //TODO speed stuff
    updateSpeed();
    //clear interrupt
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

interrupt void HallB_isr(void){
    updateSpeed();
    //clear interrupt
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

interrupt void HallC_isr(void){
    updateSpeed();
    //clear interrupt
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;
}

interrupt void epwm1_isr(void)
{
    //read hall sensors
    bool ha = GpioDataRegs.GPADAT.bit.GPIO6;
    bool hb = GpioDataRegs.GPADAT.bit.GPIO7;
    bool hc = GpioDataRegs.GPADAT.bit.GPIO8;

    //calculate next pwm cmp vals

    //lowpass for filtered speed; 0.0150341144 m/hall transition
    filteredSpeed = 0.999*filteredSpeed + 0.001*((50000.0/3)*(double)HallCount*0.0150341144);
    HallCount=0;



    //Uint16 p_inv = max_pwm - p;
    Uint16 state=(ha<<2)|(hb<<1)|(hc);



    Uint16 p = 0;

    double ctest=0;
    if((cur_A >= cur_B) && (cur_A >= cur_C)){
        ctest = cur_A;
    } else if((cur_B >= cur_A) && (cur_B >= cur_C)){
        ctest = cur_B;
    } else {
        ctest = cur_C;
    }

    /*p = controllerToPWM(ThrottleSetPoint,filteredSpeed, ctest);*/

    //if(PeakCurrent > i_0){state = 0;} //freewheel
    switch(state){
    case 1: //Test verified
         p = controllerToPWM(ThrottleSetPoint,filteredSpeed,cur_C);
         EPwm1Regs.CMPA.half.CMPA    = 0; //AH
         EPwm1Regs.CMPB              = MAX_PWM; //AL
         EPwm2Regs.CMPA.half.CMPA    = 0; //BH          //high
         EPwm2Regs.CMPB              = 0; //BL
         EPwm3Regs.CMPA.half.CMPA    = p;//CH
         EPwm3Regs.CMPB              = p; //CL
         break;
    case 2:
         p = controllerToPWM(ThrottleSetPoint,filteredSpeed,cur_B);
         EPwm1Regs.CMPA.half.CMPA    = 0; //AH
         EPwm1Regs.CMPB              = 0; //AL
         EPwm2Regs.CMPA.half.CMPA    = p; //BH
         EPwm2Regs.CMPB              = p; //BL
         EPwm3Regs.CMPA.half.CMPA    = 0;//CH
         EPwm3Regs.CMPB              = MAX_PWM; //CL
         break;
    case 3:
         p = controllerToPWM(ThrottleSetPoint,filteredSpeed,cur_C);
         EPwm1Regs.CMPA.half.CMPA    = 0; //AH
         EPwm1Regs.CMPB              = 0; //AL
         EPwm2Regs.CMPA.half.CMPA    = 0; //BH
         EPwm2Regs.CMPB              = MAX_PWM; //BL
         EPwm3Regs.CMPA.half.CMPA    = p;//CH
         EPwm3Regs.CMPB              = p; //CL
            break;
    case 4:
         p = controllerToPWM(ThrottleSetPoint,filteredSpeed,cur_A);
         EPwm1Regs.CMPA.half.CMPA    = p; //AH
         EPwm1Regs.CMPB              = p; //AL
         EPwm2Regs.CMPA.half.CMPA    = 0; //BH
         EPwm2Regs.CMPB              = MAX_PWM; //BL
         EPwm3Regs.CMPA.half.CMPA    = 0;//CH
         EPwm3Regs.CMPB              = 0; //CL
            break;
    case 5:
         p = controllerToPWM(ThrottleSetPoint,filteredSpeed,cur_A);
         EPwm1Regs.CMPA.half.CMPA    = p; //AH
         EPwm1Regs.CMPB              = p; //AL
         EPwm2Regs.CMPA.half.CMPA    = 0; //BH
         EPwm2Regs.CMPB              = 0; //BL
         EPwm3Regs.CMPA.half.CMPA    = 0;//CH
         EPwm3Regs.CMPB              = MAX_PWM; //CL
            break;
    case 6:
         p = controllerToPWM(ThrottleSetPoint,filteredSpeed,cur_B);
         EPwm1Regs.CMPA.half.CMPA    = 0; //AH
         EPwm1Regs.CMPB              = MAX_PWM; //AL
         EPwm2Regs.CMPA.half.CMPA    = p; //BH
         EPwm2Regs.CMPB              = p; //BL
         EPwm3Regs.CMPA.half.CMPA    = 0;//CH
         EPwm3Regs.CMPB              = 0; //CL
            break;
    default:  //freewheel
         EPwm1Regs.CMPA.half.CMPA    = 0; //AH
         EPwm1Regs.CMPB              = MAX_PWM; //AL
         EPwm2Regs.CMPA.half.CMPA    = 0; //BH
         EPwm2Regs.CMPB              = MAX_PWM; //BL
         EPwm3Regs.CMPA.half.CMPA    = 0;//CH
         EPwm3Regs.CMPB              = MAX_PWM; //CL
    }

    //three position switch
    unsigned int current3pos = (GpioDataRegs.GPADAT.bit.GPIO9<<1) | (GpioDataRegs.GPADAT.bit.GPIO10);
    if(current3pos == 2 ){ //change to high position
        if(previous3pos != 2){
            GpioDataRegs.GPASET.bit.GPIO13 = 1; //enable inverter
        }
    } else {
        GpioDataRegs.GPACLEAR.bit.GPIO13 = 1; //disable inverter
    }
    previous3pos = current3pos;



   // Clear INT flag for this timer
   EPwm1Regs.ETCLR.bit.INT = 1;

   // Acknowledge this interrupt to receive more interrupts from group 3
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}



