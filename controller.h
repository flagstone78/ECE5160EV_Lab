/* Torque input
 * d output
 */


double currentController(double current, double targetCurrent){
    if (targetCurrent > MAX_ADC_CURRENT-5){
        targetCurrent = (MAX_ADC_CURRENT-5);
    } else if(targetCurrent < -(MAX_ADC_CURRENT-5)){
        targetCurrent = -(MAX_ADC_CURRENT-5);
    }

    d_1 = d_0; //move previous to old var
    te_1 = te;

    te = targetCurrent-current; //new te

    //calculate new duty cycle
    d_0 = (d_1 + dConst*te + dConst1*te_1);

    //anti windup / clamp
    if (d_0 > 1){
        d_0 = 1;
    } else if(d_0 < -1){
        d_0 = -1;
    }


    return d_0;
}

//speed to duty cycle transfer function
double controller(double velRef, double vel, double cur){
    i_1 = i_0;
    ve_1 = ve;

    ve = velRef - vel;

    //calculate new current
    //i_0 = i_1+.00001*ve;
    //i_0 = i_1 + 0.1*ve -
    i_0 = (i_1 + 21800*ve + -21550*ve_1);

    //anti windup
    if (i_0 > 10){
        i_0 = 10;
    } else if(i_0 < -10){
        i_0 = -10;
    }


    return currentController(cur, i_0);

}

Uint16 controllerToPWM( double ThrottleSetPoint, double filteredSpeed, double PhaseCurrent){
    double dFromController = controller(ThrottleSetPoint,filteredSpeed,PhaseCurrent);//power;

    ptest = MAX_PWM*dFromController;
    Uint16 p = ptest;

    if(p > MAX_PWM){
        p = MAX_PWM;
    } else if (p < 0){
        p = 0;
    }

    return p;
}
