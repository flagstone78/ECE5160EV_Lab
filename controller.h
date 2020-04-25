/* Torque input
 * d output
 */


double currentController(double current, double targetCurrent){
    if (targetCurrent > 10){
        targetCurrent = 10;
    } else if(targetCurrent <-10){
        targetCurrent = -10;
    }

    te = targetCurrent-current; //new te

    //calculate new duty cycle
    d_0 = dConst*te + dConst1*te_1 + d_1; //preCalcCur;

    if (d_0 > 1){
        d_0 = 1;
    } else if(d_0 < 0){
        d_0 = 0;
    }

    d_1 = d_0; //move previous to old var
    te_1 = te;

    return d_0;
}

//speed to duty cycle transfer function
double speedController(double velRef, double vel, double cur){



    ve = velRef - vel;
    i_0 = iConst*ve + iConst1*ve_1+i_1;//preCalcVel;
    if (i_0 > 15){
        i_0 = 15;
    } else if(i_0 < 0){
        i_0 = 0;
    }

    i_1 = i_0;
    ve_1 = ve;
    /*
    if(abs(ve)>1){
        if (ve<0)
          {
              i_0-=0.0001;
          }else if (ve>0){
              i_0+=0.0001;
          }else{
              i_0=i_0;
          }
    }else if(abs(ve)>0.5||abs(ve)<1){
        if (ve<0)
          {
              i_0-=0.000001;
          }else if (ve>0){
              i_0+=0.000001;
          }else{
              i_0=i_0;
          }
    }else{
        if (ve<0)
        {
            i_0-=0.0000001;
        }else if (ve>0){
            i_0+=0.0000001;
        }else{
            i_0=i_0;
        }
    }
*/
    return currentController(cur,i_0);

}

Uint16 controllerToPWM( double ThrottleSetPoint, double wheelSpeed, double PhaseCurrent){


    double dFromController = MAX_PWM*speedController(ThrottleSetPoint,wheelSpeed,PhaseCurrent);//power;
    if(dFromController < 0){ dFromController = 0;} //clamp 0
    if(dFromController > MAX_PWM){ dFromController = MAX_PWM;}
/*
    ptest = MAX_PWM*dFromController;
    Uint16 p = ptest;

    if(p > MAX_PWM){ p = MAX_PWM;} //clamp max*/

    return dFromController;
}

void prepareforNext(){
    //current

    //anti windup / clamp


    /*d_1 = d_0; //move previous to old var
    te_1 = te;

    preCalcCur = dConst1*te_1 + d_1;


    //voltage


    //anti windup

    i_1 = i_0;
    ve_1 = ve;

    preCalcVel = iConst1*ve_1+i_1;
    */

}
