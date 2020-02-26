/*
 * speed.h
 *
 *  Created on: Feb 19, 2020
 *      Author: A01700270
 */

#ifndef SPEED_H_
#define SPEED_H_
void speedTimerSetUp(){
    CpuTimer0Regs.TCR.bit.FREE=1; //allow free run when debug
    CpuTimer0Regs.PRD.all = 0xFFFFFFFF; //47sec period @ 90MHz
}

void updateSpeed(){
    HallCount++;
    //oldTime=time;
    //time=CpuTimer0Regs.TIM.all;

    //unsigned long int deltaTime = oldTime-time;
    //speed = 94247779.608/deltaTime; // electrical rad/s  (pi/3) rad * (90000000/counts) (1/s)
    //speed = 1353070.2968/deltaTime;// m/s

    // 46 transistions per turn per hall sensor
    // 3 hall sensors =138 transition per turn
    // 2.07470778843 m/turn
    // 0.0150341144 m/hall transition
     //1 count per (1/90000000)seconds
}



#endif /* SPEED_H_ */
