/**
  ******************************************************************************
  * File Name          : tasks_platemotor.h
  * Description        : 拨盘电机任务
  ******************************************************************************
  *
  * Copyright (c) 2017 Team TPP-Shanghai Jiao Tong University
  * All rights reserved.
  *
	* 定时循环
	* 编码器模式对编码器脉冲计数
	* PWM波控制速度
  ******************************************************************************
  */
#ifndef FRAMEWORK_TASKS_PLATECONTROL_H
#define FRAMEWORK_TASKS_PLATECONTROL_H

#include "cmsis_os.h"
#include "tasks_timed.h"
#include "pid_regulator.h"

#define OneShoot (722)
#define PID_SHOOT_MOTOR_SPEED      (30)

#define SHOOT_MOTOR_POSITION_PID_DEFAULT \
{\
	0,\
	0,\
	{0,0},\
	7.5f,\
	0.05f,\
	0.6f,\
	0,\
	0,\
	0,\
	4900,\
	3500,\
	1500,\
	0,\
	1000,\
	0,\
	0,\
	0,\
	&PID_Calc,\
	&PID_Reset,\
}\

#define SHOOT_MOTOR_SPEED_PID_DEFAULT \
{\
	0,\
	0,\
	{0,0},\
	50.0f,\
	0.5f,\
	0.0f,\
	0,\
	0,\
	0,\
	1000,\
	200,\
	100,\
	0,\
	4950,\
	0,\
	0,\
	0,\
	&PID_Calc,\
	&PID_Reset,\
}\


void PlateMotorTask(void const * argument);
void ShootOneBullet(void);
int32_t GetQuadEncoderDiff(void);
#endif