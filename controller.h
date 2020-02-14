#define T 0.00002 //s; sampling period

//current controller variables
#define ravg 0.2435        //ohms
#define lavg 0.000356      //h
#define Vdc  26.0            //volts
#define wc   (2.0*3.1415927*1000.0) //cross over fq rad/sec


#define Kpi ((2*wc*lavg)/Vdc) //0.172076
#define Kii ((2*wc*ravg)/Vdc) //117.70

#define dConst   (((Kii*T)/2) + Kpi)
#define dConst1 (((Kii*T)/2) - Kpi)


//voltage controller variables
#define wc2    (2.0*3.1415927*100.0)              //rad/sec
#define Gphy0  0.7038
#define Wphy   0.041008           //rad/sec
#define wz2   (wc2/10.0)

#define Kp_v (wc2/(Gphy0*wphy))//21770.0
#define Ki_v ((wc2*wz2)/(Gphy0*wphy))//136800.0




/* Torque input
 * d output
 */
double d = 0; //duty cycle d[k]
double d_1 = 0; //old duty cycle d[k-1]
double te = 0; //torque error
double te_1 = 0;

Uint16 currentController(long current, long targetCurrent){
    d_1 = d; //move previous to old var
    te_1 = te;

    te = targetCurrent-current; //new te

    //calculate new duty cycle
    d = (d_1 + dConst*te + dConst1*te_1);

    //anti windup / clamp
    if (d > 1){
        d = 1;
    } else if(d < 0){
        d = 0;
    }

    return d;
}


//speed to duty cycle transfer function
Uint16 controller(Uint16 velRef, Uint16 vel, Uint16 cur){
    return currentController(cur,1);
}
