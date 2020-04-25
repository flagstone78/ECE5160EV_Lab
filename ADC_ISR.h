#include "Globals.h"
#include "controller.h"

#define MinThrottleVoltage 0.88  //V
#define MaxThrottleVoltage 4.2   //V
#define ADCVoltageDividerRatio 0.772
#define MAXADC  (MaxThrottleVoltage*ADCVoltageDividerRatio*4095.0)/3.3
#define MINADC  (MinThrottleVoltage*ADCVoltageDividerRatio*4095.0)/3.3
#define MAX_SPEED_MPH 10.0 //MPH
#define MAX_SPEED_MPS MAX_SPEED_MPH*0.44704 // m/s

#define MAX_ADC_CURRENT 19.0   //A //3000 //15 A   //2500 //10A        //2110 //5A    //
#define CURRENT_OFFSET 1705.0  // adc count offset
//Scaling is most likely off
#define CURRENT_SCALE 81.5 //76.37806 //adc counts per amp

//#pragma CODE_SECTION(adc_isr, "ramfuncs");
double trip_A = 0;
double trip_B = 0;
double trip_C = 0;
interrupt void  adc_isr(void)
{
    controllerOldTime = CpuTimer0Regs.TIM.all;

    //double c = (double)(AdcResult.ADCRESULT1 - CURRENT_OFFSET)/CURRENT_SCALE;
    cur_C = ((double)AdcResult.ADCRESULT1*0.0125594816611205 - 25.7474436368839);
    cur_B = ((double)AdcResult.ADCRESULT2*0.0091 - 18.73);
    cur_A = ((double)AdcResult.ADCRESULT3*0.0126 - 25.8654); //t=34

    //Throttle to velocity command
    //ThrottleSetPoint = 0.0013755076923077*((double)AdcResult.ADCRESULT0-760.0); // 0 to 10 mph   //t=44

    //if(PeakCurrent > i_0){state = 0;} //freewheel
    switch(state){
    case 1: //Test verified
         p = controllerToPWM(ThrottleSetPoint,filteredSpeed,-cur_B); //cur_C
         EPwm1Regs.CMPA.half.CMPA    = 0; //AH
         EPwm1Regs.CMPB              = MAX_PWM; //AL
         EPwm2Regs.CMPA.half.CMPA    = 0; //BH          //high
         EPwm2Regs.CMPB              = 0; //BL
         EPwm3Regs.CMPA.half.CMPA    = p;//CH
         EPwm3Regs.CMPB              = p; //CL
         break;
    case 2:
         p = controllerToPWM(ThrottleSetPoint,filteredSpeed,-cur_A); //curB
         EPwm1Regs.CMPA.half.CMPA    = 0; //AH
         EPwm1Regs.CMPB              = 0; //AL
         EPwm2Regs.CMPA.half.CMPA    = p; //BH
         EPwm2Regs.CMPB              = p; //BL
         EPwm3Regs.CMPA.half.CMPA    = 0;//CH
         EPwm3Regs.CMPB              = MAX_PWM; //CL
         break;
    case 3:
         p = controllerToPWM(ThrottleSetPoint,filteredSpeed,-cur_A); //-curA
         EPwm1Regs.CMPA.half.CMPA    = 0; //AH
         EPwm1Regs.CMPB              = 0; //AL
         EPwm2Regs.CMPA.half.CMPA    = 0; //BH
         EPwm2Regs.CMPB              = MAX_PWM; //BL
         EPwm3Regs.CMPA.half.CMPA    = p;//CH
         EPwm3Regs.CMPB              = p; //CL
            break;
    case 4:
         p = controllerToPWM(ThrottleSetPoint,filteredSpeed,-cur_C); //curA
         EPwm1Regs.CMPA.half.CMPA    = p; //AH
         EPwm1Regs.CMPB              = p; //AL
         EPwm2Regs.CMPA.half.CMPA    = 0; //BH
         EPwm2Regs.CMPB              = MAX_PWM; //BL
         EPwm3Regs.CMPA.half.CMPA    = 0;//CH
         EPwm3Regs.CMPB              = 0; //CL
            break;
    case 5:
         p = controllerToPWM(ThrottleSetPoint,filteredSpeed,-cur_B); //-curB
         EPwm1Regs.CMPA.half.CMPA    = p; //AH
         EPwm1Regs.CMPB              = p; //AL
         EPwm2Regs.CMPA.half.CMPA    = 0; //BH
         EPwm2Regs.CMPB              = 0; //BL
         EPwm3Regs.CMPA.half.CMPA    = 0;//CH
         EPwm3Regs.CMPB              = MAX_PWM; //CL
            break;
    case 6:
         p = controllerToPWM(ThrottleSetPoint,filteredSpeed,-cur_C); //-curC
         EPwm1Regs.CMPA.half.CMPA    = 0; //AH
         EPwm1Regs.CMPB              = MAX_PWM; //AL
         EPwm2Regs.CMPA.half.CMPA    = p; //BH
         EPwm2Regs.CMPB              = p; //BL
         EPwm3Regs.CMPA.half.CMPA    = 0;//CH
         EPwm3Regs.CMPB              = 0; //CL
            break;
    case 15: //test
         EPwm1Regs.CMPA.half.CMPA    = MAX_PWM/2; //AH
         EPwm1Regs.CMPB              = MAX_PWM/2; //AL
         EPwm2Regs.CMPA.half.CMPA    = MAX_PWM/2; //BH
         EPwm2Regs.CMPB              = MAX_PWM/2; //BL
         EPwm3Regs.CMPA.half.CMPA    = MAX_PWM/2;//CH
         EPwm3Regs.CMPB              = MAX_PWM/2; //CL
         break;
    default:  //freewheel
         EPwm1Regs.CMPA.half.CMPA    = 0; //AH
         EPwm1Regs.CMPB              = MAX_PWM; //AL
         EPwm2Regs.CMPA.half.CMPA    = 0; //BH
         EPwm2Regs.CMPB              = MAX_PWM; //BL
         EPwm3Regs.CMPA.half.CMPA    = 0;//CH
         EPwm3Regs.CMPB              = MAX_PWM; //CL
    }

    controllerTime = CpuTimer0Regs.TIM.all;
    controllerDeltaTime = controllerOldTime-controllerTime;
    prepareforNext();


    time=CpuTimer0Regs.TIM.all;
    deltaTime = oldTime-time;

    if(deltaTime>50000000){filteredSpeed=0;}

    if(oldState!=state && startup==1){
        deltaTime = oldTime-time;
        double speedT=(double)deltaTime/90000000.0;
        filteredSpeed =/*.9*filteredSpeed+ 0.1*/0.0150341144/speedT;
        oldTime=time;
    }
    oldState=state;

    //output speed
    EPwm6Regs.CMPB = MAX_PWM*2.0*filteredSpeed/6.0;

    //lowpass for filtered speed; 0.0150341144 m/hall transition
    //filteredSpeed = 0.99*filteredSpeed + 0.01*((50000.0)*(double)HallCount*0.0150341144);
    //HallCount=0;


    AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;     //Clear ADCINT1 flag reinitialize for next SOC
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;   // Acknowledge interrupt to PIE
    startup=1;
    return;
}

void configureADC(){
    InitAdc(); //power up, enable adc with clk/2
    AdcOffsetSelfCal(); //must be called after enable global interrupt
    InitAdcAio();

    //AdcChanSelect(7);
    EALLOW;
    //turn off all adc triggers
    AdcRegs.ADCINTSOCSEL1.bit.SOC0  = 0;    //ADCINT2 starts SOC0-7
    AdcRegs.ADCINTSOCSEL1.bit.SOC1  = 0;
    AdcRegs.ADCINTSOCSEL1.bit.SOC2  = 0;
    AdcRegs.ADCINTSOCSEL1.bit.SOC3  = 0;
    AdcRegs.ADCINTSOCSEL1.bit.SOC4  = 0;
    AdcRegs.ADCINTSOCSEL1.bit.SOC5  = 0;
    AdcRegs.ADCINTSOCSEL1.bit.SOC6  = 0;
    AdcRegs.ADCINTSOCSEL1.bit.SOC7  = 0;
    AdcRegs.ADCINTSOCSEL2.bit.SOC8  = 0;    //ADCINT1 starts SOC8-15
    AdcRegs.ADCINTSOCSEL2.bit.SOC9  = 0;
    AdcRegs.ADCINTSOCSEL2.bit.SOC10 = 0;
    AdcRegs.ADCINTSOCSEL2.bit.SOC11 = 0;
    AdcRegs.ADCINTSOCSEL2.bit.SOC12 = 0;
    AdcRegs.ADCINTSOCSEL2.bit.SOC13 = 0;
    AdcRegs.ADCINTSOCSEL2.bit.SOC14 = 0;
    AdcRegs.ADCINTSOCSEL2.bit.SOC15 = 0;


    AdcRegs.ADCCTL2.bit.ADCNONOVERLAP = 1;  //Enable non-overlap mode
    AdcRegs.ADCCTL1.bit.INTPULSEPOS = 1;    //ADCINT1 trips after AdcResults latch

    AdcRegs.INTSEL1N2.bit.INT1E     = 1;    //enable ADCINT1
    AdcRegs.INTSEL1N2.bit.INT1CONT  = 0;    //Disable ADCINT1 Continuous mode
    AdcRegs.INTSEL1N2.bit.INT1SEL   = 3;    //setup EOC0 to trigger ADCINT1 to fire


    //simultaneous conversion SIMULENx = 1
    AdcRegs.ADCSAMPLEMODE.bit.SIMULEN0 = 1;
    AdcRegs.ADCSAMPLEMODE.bit.SIMULEN2 = 1;

    AdcRegs.ADCSOC0CTL.bit.CHSEL    = 7;    //set SOC0 channel select to ADCINA7 for       throttle
    AdcRegs.ADCSOC0CTL.bit.TRIGSEL  = 5;    //set SOC0 start trigger on EPWM1A
    AdcRegs.ADCSOC0CTL.bit.ACQPS    = 25;   //set SOC0 S/H Window to 26 ADC Clock Cycles, (25 ACQPS plus 1)


    //AdcRegs.ADCSOC1CTL.bit.CHSEL    = 7;    //set SOC0 channel select to ADCINB7            Current phase c
    //AdcRegs.ADCSOC1CTL.bit.TRIGSEL  = 5;    //set SOC0 start trigger on EPWM1A
    //AdcRegs.ADCSOC1CTL.bit.ACQPS    = 25;   //set SOC0 S/H Window to 26 ADC Clock Cycles, (25 ACQPS plus 1)

    AdcRegs.ADCSOC2CTL.bit.CHSEL    = 5;    //set SOC2 channel select to ADCINA5           Current phase B
    AdcRegs.ADCSOC2CTL.bit.TRIGSEL  = 5;    //set SOC2 start trigger on EPWM1A
    AdcRegs.ADCSOC2CTL.bit.ACQPS    = 25;   //set SOC2 S/H Window to 26 ADC Clock Cycles, (25 ACQPS plus 1)
/*
    AdcRegs.ADCSOC1CTL.bit.CHSEL    = 5;    //set SOC1 channel select to ADCINB5           Current phase A
    AdcRegs.ADCSOC1CTL.bit.TRIGSEL  = 5;    //set SOC1 start trigger on EPWM1A
    AdcRegs.ADCSOC1CTL.bit.ACQPS    = 25;   //set SOC1 S/H Window to 26 ADC Clock Cycles, (25 ACQPS plus 1)
*/
    EDIS;

    //start adc
    //AdcRegs.ADCSOCFRC1.bit.SOC7 = 1;

}
