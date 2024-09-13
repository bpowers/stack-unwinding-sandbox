.syntax unified
.cpu cortex-m3
.thumb

.extern __Hard_Fault_Handler

.section .startup
.global _Hard_Fault_Handler

_Hard_Fault_Handler:
  TST LR, #4
  ITE EQ
  MRSEQ R0, MSP
  MRSNE R0, PSP
  B __Hard_Fault_Handler
