#ifndef GLOBALS_H
#define GLOBALS_H

#define MAX_PWM  900 // Period register



//Controller.h
double d_0 = 0; //duty cycle d[k]
double d_1 = 0; //old duty cycle d[k-1]
double te = 0; //torque error
double te_1 = 0;

double i_0 = 0; //current i[k]
double i_1 = 0; //old current i[k-1]
double ve = 0; //velocity error
double ve_1 = 0;

double test=0;

double T,ravg,lavg,Vdc,wc,Kpi,Kii,wc2,Gphy0,wphy,wz2,Kpv,Kiv; //s; sampling period
double Mv;

unsigned short p=0;

volatile double  iConst,iConst1,dConst,dConst1,preCalcVel,preCalcCur;

void setGlobals(){
    //physical variables
    T= 1.0/50000.0; //s; sampling frequency
    Mv = 1.0;//80.0+25.0; //kg; mass of bike and rider

    //current controller variables
    ravg = 0.2435;        //ohms
    lavg = 0.000356;      //h
    Vdc  =26.0;            //volts
    wc = (2.0*3.1415927*1000.0); //cross over fq rad/sec


    Kpi = ((2.0*wc*lavg)/Vdc); //0.172076
    Kii = ((2.0*wc*ravg)/Vdc); //117.70
    dConst = (((Kii*T)/2.0) + Kpi);
    dConst1 = (((Kii*T)/2.0) - Kpi);
    preCalcCur=0;


    //voltage controller variables
    wc2 =   (2.0*3.1415927*50.0);//50.0);              //rad/sec
    Gphy0 = (2.0*46.0*0.274)/(1.0*1.204*10.0*0.447*0.3302);//0.7038;  // 1/(Cd*rho*Av*V0*mphtoms*rw)
    wphy  = (1.0*1.204*10.0*0.447)/Mv;    //Cd*rho*Av*v0/Mv   Cd = 1, Av=0.4; Cr=0.01; rho =1.204    //rad/sec
    wz2   =(wc2/10.0);

    Kpv = (wc2/(Gphy0*wphy));//21770.0
    Kiv = ((wc2*wz2)/(Gphy0*wphy));//1368000.0

    iConst = (((Kiv*T)/2.0) + Kpv);
    iConst1= (((Kiv*T)/2.0) - Kpv);
    preCalcVel=0;

    i_0 = 0;
    i_1 = 0;
    ve = 0;
    ve_1 = 0;
}

//speed.h
volatile unsigned long int time     = 0;
volatile unsigned long int oldTime  = 0;
volatile unsigned long int deltaTime= 0;
volatile unsigned short oldState=0;

volatile unsigned long int controllerTime     = 0;
volatile unsigned long int controllerOldTime  = 0;
volatile unsigned long int controllerDeltaTime= 0;

double speed=0;
double filteredSpeed=0;
volatile double speedT=0;
volatile unsigned int HallCount = 0;

//set in the ADC isr and read elswhere
volatile double ThrottleSetPoint = 0;     //A
volatile double cur_A = 0;          //A
volatile double cur_B = 0;          //A
volatile double cur_C = 0;          //A
volatile unsigned char startup=0;
volatile unsigned int previous3pos=0;

volatile unsigned short state = 0; //hall sensor state

#endif
