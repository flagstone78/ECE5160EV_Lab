#include "Globals.h"

#define MinThrottleVoltage 0.88  //V
#define MaxThrottleVoltage 4.2   //V
#define ADCVoltageDividerRatio 0.772
#define MAXADC  (MaxThrottleVoltage*ADCVoltageDividerRatio*4095.0)/3.3
#define MINADC  (MinThrottleVoltage*ADCVoltageDividerRatio*4095.0)/3.3
#define MAX_SPEED_MPH 10.0 //MPH
#define MAX_SPEED_MPS MAX_SPEED_MPH*0.44704 // m/s

#define MAX_ADC_CURRENT 15.0   //A //3000 //15 A   //2500 //10A        //2110 //5A    //
#define CURRENT_OFFSET 1705.0  // adc count offset
//Scaling is most likely off
#define CURRENT_SCALE 81.5 //76.37806 //adc counts per amp

//#pragma CODE_SECTION(adc_isr, "ramfuncs");
double trip = 0;
interrupt void  adc_isr(void)
{
    //Throttle to velocity command
    double t = MAX_SPEED_MPS*((double)AdcResult.ADCRESULT0-MINADC)/(MAXADC-MINADC); //(0 to 1)*10 mph

    if(t>MAX_SPEED_MPS){
        t = MAX_SPEED_MPS;
    } else if (t<0){
        t = 0;
    }

    /*if(t>1){ //transient test
        t = 3;
    } else {
        t =1;
    }*/
    ThrottleSetPoint = t; //set global

    //double c = (double)(AdcResult.ADCRESULT1 - CURRENT_OFFSET)/CURRENT_SCALE;
    double curAdcVal = AdcResult.ADCRESULT1;
    double c;
    if(curAdcVal < 1793){ //less than 1 amp
        c =  -0.0002214609511*curAdcVal*curAdcVal + 0.80599694921*curAdcVal - 732.39954476; //polynomial
    }else{
        c = 0.012473509071*curAdcVal - 21.465325397; //linear
    }

    if(c < 0.0){c *= -1;}

    PeakCurrent = c;

    //Current = AdcResult.ADCRESULT1;
    if (PeakCurrent >= MAX_ADC_CURRENT){
      GpioDataRegs.GPACLEAR.bit.GPIO13 = 1;  //Disable gate driver
      trip=PeakCurrent;
    }

    AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;     //Clear ADCINT1 flag reinitialize for next SOC
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;   // Acknowledge interrupt to PIE

    return;
}

void configureADC(){
    InitAdc(); //power up, enable adc with clk/2
    AdcOffsetSelfCal(); //must be called after enable global interrupt
    InitAdcAio();

    //AdcChanSelect(7);

    // ADC Configuration for Temp Sensor
    EALLOW;
    AdcRegs.ADCCTL2.bit.ADCNONOVERLAP = 1;  //Enable non-overlap mode
    AdcRegs.ADCCTL1.bit.INTPULSEPOS = 1;    //ADCINT1 trips after AdcResults latch
    AdcRegs.INTSEL1N2.bit.INT1E     = 1;    //Enabled ADCINT1
    AdcRegs.INTSEL1N2.bit.INT1CONT  = 0;    //Disable ADCINT1 Continuous mode
    AdcRegs.INTSEL1N2.bit.INT1SEL   = 0;    //setup EOC0 to trigger ADCINT1 to fire

    //simultaneous conversion SIMULENx = 1
    AdcRegs.ADCSAMPLEMODE.bit.SIMULEN0 = 1;

    AdcRegs.ADCSOC0CTL.bit.CHSEL    = 7;    //set SOC0 channel select to ADCINA7 for throttle
    AdcRegs.ADCSOC0CTL.bit.TRIGSEL  = 5;    //set SOC0 start trigger on EPWM1A
    AdcRegs.ADCSOC0CTL.bit.ACQPS    = 25;   //set SOC0 S/H Window to 26 ADC Clock Cycles, (25 ACQPS plus 1)

    AdcRegs.ADCSOC1CTL.bit.CHSEL    = 7;    //set SOC0 channel select to ADCINA5 (which is internally connected to the temperature sensor)
    AdcRegs.ADCSOC1CTL.bit.TRIGSEL  = 5;    //set SOC0 start trigger on EPWM1A
    AdcRegs.ADCSOC1CTL.bit.ACQPS    = 25;   //set SOC0 S/H Window to 26 ADC Clock Cycles, (25 ACQPS plus 1)

    EDIS;

    //start adc
    //AdcRegs.ADCSOCFRC1.bit.SOC7 = 1;

}
