/**
  ******************************************************************************
  * File Name          : tasks_platemotor.c
  * Description        : 拨盘电机控制任务
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
#include <tim.h>
#include <stdint.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <cmsis_os.h>
#include <task.h>
#include <usart.h>
#include "tasks_timed.h"
#include "pid_Regulator.h"
#include "drivers_uartrc_low.h"
#include "drivers_uartrc_user.h"
#include "tasks_remotecontrol.h"
#include "application_motorcontrol.h"
#include "drivers_canmotor_low.h"
#include "drivers_canmotor_user.h"
#include "utilities_debug.h"
#include "rtos_semaphore.h"
#include "rtos_task.h"
#include "peripheral_define.h"
#include "drivers_platemotor.h"
#include "tasks_platemotor.h"

PID_Regulator_t ShootMotorPositionPID = SHOOT_MOTOR_POSITION_PID_DEFAULT;      //shoot motor
PID_Regulator_t ShootMotorSpeedPID = SHOOT_MOTOR_SPEED_PID_DEFAULT;

void PlateMotorTask(void const * argument)
{
	int RotateAdd = 0;
	int Stuck = 0;//卡弹标志位
	ShootMotorPositionPID.ref = 0x0;
	ShootMotorPositionPID.fdb = 0x0;
	int32_t last_fdb = 0x0;
	int32_t this_fdb = 0x0;
	
	while(1)
	{
		//每单击一次鼠标左键，便会有一次Shooting状态
		if(GetShootState() == SHOOTING && GetInputMode()==KEY_MOUSE_INPUT && Stuck==0)
		{
			ShootOneBullet();
		}

		//遥控器输入模式下，只要处于发射态，就一直转动
		if(GetShootState() == SHOOTING && GetInputMode() == REMOTE_INPUT && Stuck==0)
		{
			RotateAdd += 4;
			//fw_printfln("ref = %f",ShootMotorPositionPID.ref);
			if(RotateAdd>OneShoot)
			{
				//ShootMotorPositionPID.ref = ShootMotorPositionPID.ref+OneShoot;
				ShootOneBullet();
				RotateAdd = 0;
			}
		}
		else if(GetShootState() == NOSHOOTING && GetInputMode() == REMOTE_INPUT)
		{
			RotateAdd = 0;
		}

		if(GetFrictionState()==FRICTION_WHEEL_ON)//拨盘转动前提条件：摩擦轮转动
		{
			this_fdb = GetQuadEncoderDiff(); 
			//fw_printfln("this_fdb = %d",this_fdb);
			
			//卡弹处理
	//		if(abs(ShootMotorPositionPID.ref-ShootMotorPositionPID.fdb)>100 && (abs(this_fdb-last_fdb)<5 || abs(this_fdb+65535-last_fdb)<5)) //认为卡弹
	//		{//ShootMotorPositionPID.ref = ShootMotorPositionPID.ref - OneShoot;
	//		}
		//	else
			//{
			if(this_fdb<last_fdb-10000 && getPlateMotorDir()==FORWARD)	//cnt寄存器溢出判断 正转
			{
				ShootMotorPositionPID.fdb = ShootMotorPositionPID.fdb+(65535+this_fdb-last_fdb);
			}
			else if(this_fdb>last_fdb+10000 && getPlateMotorDir()==REVERSE)	//cnt寄存器溢出判断 反转
			{
				ShootMotorPositionPID.fdb = ShootMotorPositionPID.fdb-(65535-this_fdb+last_fdb);
			}
			else
				ShootMotorPositionPID.fdb = ShootMotorPositionPID.fdb + this_fdb-last_fdb;
		//	}
			last_fdb = this_fdb;
			//fw_printfln("fdb = %f",ShootMotorPositionPID.fdb);
			ShootMotorPositionPID.Calc(&ShootMotorPositionPID);
	//		ShootMotorSpeedPID.ref = ShootMotorPositionPID.output;
	//		ShootMotorSpeedPID.fdb = 
			if(ShootMotorPositionPID.output<0) //反转
			{
				setPlateMotorDir(REVERSE);
				ShootMotorPositionPID.output = -ShootMotorPositionPID.output;
			}
			else
				setPlateMotorDir(FORWARD);
			
			__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, ShootMotorPositionPID.output);
		}
		
		else
		{
			__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);//摩擦轮不转，立刻关闭拨盘
		}
	}
	
}


void ShootOneBullet()
{
	ShootMotorPositionPID.ref = ShootMotorPositionPID.ref+OneShoot;
}

int32_t GetQuadEncoderDiff(void)
{
  int32_t cnt = 0;    
	cnt = __HAL_TIM_GET_COUNTER(&htim5) - 0x0;
	//fw_printfln("%x",cnt);
	 //__HAL_TIM_SET_COUNTER(&htim5, 0x7fff);
	return cnt;
}