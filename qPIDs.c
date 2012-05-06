
#include "qPIDs.h"
#include <stdio.h>
#include <math.h>

void qPID_Init(qPID * q){
	q->ctx.Ui_old = 0.0;
	q->ctx.Ud_old = 0.0;
	q->ctx.PV_old = 0.0;
	q->ctx.SP_old = 0.0;
}

float qPID_Process(qPID * q, float Input, float PV, float terms[]){


	// =====================================
	//   MANUAL
	// =====================================
	// Input = manual input -> CO

	// =====================================
	//   AUTOMATIC
	// =====================================
	// Input = Setpoint

	// For local use
	float ControllerOutput;
	float Up, Ui, Ud;
	float Kp, Ki, Kd_a, Kd_b,Kd_c;


	if (fabs(q->Ti)<EPSILON){
		q->Ti = EPSILON;
	}
	Kp = q->K;
	Ki = ((q->K) * (q->Ts) )/ (q->Ti);
	Kd_a = q->Td/(q->Td + q->N*q->Ts) ;
	Kd_b = (q->K*q->Td*q->N)/(q->Td + q->N*q->Ts);
	Kd_c = (q->c*q->K*q->Td*q->N)/(q->Td + q->N*q->Ts);


	// Proportional gain
	Up = Kp*( ( (q->b)*(Input) ) - PV);

	// Deriative gain with filter
	Ud = Kd_a * (q->ctx.Ud_old) - Kd_b*(PV-q->ctx.PV_old) + Kd_c*(Input-q->ctx.SP_old);

	// Get last integral
	Ui =  q->ctx.Ui_old;

	// Calculate controler output for Automatic or manual mode

	switch (q->Mode){
		case MANUAL:
			ControllerOutput = Input;

			if (q->Bumpless == ENABLED){
				q->ctx.Ui_old = PV;
			}

			break;

		case AUTOMATIC:
			ControllerOutput =  Up + Ui + Ud;
			break;

		case RELAY:
			if ((Input-PV)>=0){
				ControllerOutput = q->OutputMax;
			}else{
				ControllerOutput = q->OutputMin;
			}
			break;
		default:
			// ERROR
			ControllerOutput = NAN;
			break;
	}

	// Output parameters for debug
	if (terms!=NULL){
		terms[0] = Up;
		terms[1] = Ui;
		terms[2] = Ud;
	}

	// Calc de integral for the next step
	Ui = q->ctx.Ui_old + Ki*((Input)-PV);

	// Anti Windup
	if ( (q->AntiWindup == ENABLED) && (q->Mode=AUTOMATIC) ){
		if (Ui>=q->OutputMax){
			Ui = q->OutputMax;
		}else if (Ui<=q->OutputMin){
			Ui = q->OutputMin;
		}
	}

	// Save context for next step.
	q->ctx.Ui_old = Ui;
	q->ctx.Ud_old = Ud;
	q->ctx.PV_old = PV;
	q->ctx.SP_old = Input;

	return ControllerOutput;
}
