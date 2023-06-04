/*
 * File: Subsystem.c
 *
 * Code generated for Simulink model 'Subsystem'.
 *
 * Model version                  : 1.3
 * Simulink Coder version         : 9.0 (R2018b) 24-May-2018
 * C/C++ source code generated on : Thu May 21 10:20:12 2020
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#include "Subsystem.h"
#include "Subsystem_private.h"

/* Block states (default storage) */
DW_Subsystem_T Subsystem_DW;

/* External inputs (root inport signals with default storage) */
ExtU_Subsystem_T Subsystem_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_Subsystem_T Subsystem_Y;

/* Real-time model */
RT_MODEL_Subsystem_T Subsystem_M_;
RT_MODEL_Subsystem_T *const Subsystem_M = &Subsystem_M_;

/* Model step function */
void Subsystem_step(void)
{
  /* DiscreteTransferFcn: '<S1>/Discrete Transfer Fcn' */
  //Subsystem_Y.Out1 = 3.2 * Subsystem_DW.DiscreteTransferFcn_states;

  /* Update for DiscreteTransferFcn: '<S1>/Discrete Transfer Fcn' incorporates:
   *  Gain: '<S1>/Gain'
   *  Inport: '<Root>/In1'
   *  Sum: '<S1>/Sum1'
   */
  Subsystem_DW.DiscreteTransferFcn_states = ((Subsystem_U.In1 - Subsystem_Y.Out1)
    * 3 - Subsystem_DW.DiscreteTransferFcn_states) / 4.0;
}

/* Model initialize function */
void Subsystem_initialize(void)
{
  /* Registration code */

  /* initialize error status */
  rtmSetErrorStatus(Subsystem_M, (NULL));

  /* states (dwork) */
  (void) memset((void *)&Subsystem_DW, 0,
                sizeof(DW_Subsystem_T));

  /* external inputs */
  Subsystem_U.In1 = 0.0;

  /* external outputs */
  Subsystem_Y.Out1 = 0.0;
}

/* Model terminate function */
void Subsystem_terminate(void)
{
  /* (no terminate code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
