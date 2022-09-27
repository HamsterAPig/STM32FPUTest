# Compare the speed-up performance of FPU and DSP
[简体中文](https://github.com/HamsterAPig/STM32FPUTest/blob/master/README_ZH.md)



## testing platform

* MCU: `STM32F401CCU6`, Freq = 84Mhz, internal crystal
* Compiler: `arm-none-eabi-gcc`, optimization option `-Og -g` (after optimization is turned on, the loop will be optimized out of no sense)
* Burning tool: `openOCD`
* Debug tool: `DAPLink`

## testing method

> The `DWT` counter in the `Arm` kernel records the beats since the CPU works. By comparing the beats before and after the algorithm runs, it is easy to calculate how many clock cycles have passed.

## Test floating point arithmetic

### Software floating point

Enable `ENABLE_TEST_FLOAT`

Disable the FPU. At this time, the word `mul` appears in the assembly file to indicate that the multiplier is called to multiply, instead of calculating the result at compile time

````assembly
.text:0800106E ; --------------------------------------------- ------------------------------
.text:0800106E
.text:0800106E loc_800106E ; CODE XREF: main+34↓j
.text:0800106E i = R4 ; int
.text:0800106E LDR R0, [SP,#0x28+a]
.text:08001070 LDR R1, [SP,#0x28+b]
.text:08001072 BL __mulsf3
.text:08001076 STR R0, [SP,#0x28+res]
````

The output at this point is as follows:

(Data obtained by calculating `res = float * float` 10000 times)

````
Calc res = 0.210063, CPU Cycle = 442087
Calc res = 0.210063, CPU Cycle = 442087
Calc res = 0.210063, CPU Cycle = 449550
Calc res = 0.210063, CPU Cycle = 442087
Calc res = 0.210063, CPU Cycle = 442087
Calc res = 0.210063, CPU Cycle = 442087
Calc res = 0.210063, CPU Cycle = 449550
Calc res = 0.210063, CPU Cycle = 442087
````

### Using fixed-point numbers

> Due to library file limitations, enabling fixed-point numbers on this platform requires the FPU to be turned on

#### ARM_MATH

````assembly
.text:08000F6E ; --------------------------------------------- ------------------------------
.text:08000F6E
.text:08000F6E loc_8000F6E ; CODE XREF: main+36↓j
.text:08000F6E i = R4 ; int ; blockSize
.text:08000F6E MOVS R3, #1
.text:08000F70 ADD R2, SP, #0x40+q_tmp+8 ; pDst
.text:08000F72 ADD R1, SP, #0x40+q_tmp+4 ; pSrcB
.text:08000F74 ADD R0, SP, #0x40+q_tmp ; pSrcA
.text:08000F76 BL arm_mult_q31
````

Calculation results

````
Calc res = 0.210063, CPU Cycle = 590317
Calc res = 0.210063, CPU Cycle = 590313
Calc res = 0.210063, CPU Cycle = 590317
Calc res = 0.210063, CPU Cycle = 590317
Calc res = 0.210063, CPU Cycle = 590313
Calc res = 0.210063, CPU Cycle = 590317
Calc res = 0.210063, CPU Cycle = 590317
````

#### IQ_MATH

Since the calculation result is not returned in the assembly code of `IQMath`, it cannot be tested

````C
  while ( 1 )
  {
    v5 = vcvts_n_s32_f32(0.5569, 0x14u);
    v6 = vcvts_n_s32_f32(0.3772, 0x14u);
    v4 = 0;
    rr = 0;
    before = DWT_TS_GET();
    while ( v4 <= 9999 )
    {
      rr = IQ20mpy(v5, v6);
      ++v4;
    }
    time = DWT_TS_GET() - before;
    IQ20toF(rr);
    printf_("Calc res = %f, CPU Cycle = %d\n", v3, time);
    HAL_Delay(0x1F4u);
  }
}
````

### FPU

````assembly
.text:08000E98 ; --------------------------------------------- ------------------------------
.text:08000E98
.text:08000E98 loc_8000E98 ; CODE XREF: main+4E↓j
.text:08000E98 FLDS S14, [R7,#0x18+a]
.text:08000E9C FLDS S15, [R7,#0x18+b]
.text:08000EA0 FMULS S15, S14, S15
.text:08000EA4 FSTS S15, [R7,#0x18+res]
.text:08000EA8 LDR R3, [R7,#0x18+i]
.text:08000EAA ADDS R3, #1
````

The calculation results are as follows

````
Calc res = 0.210063, CPU Cycle = 180138
Calc res = 0.210063, CPU Cycle = 180132
Calc res = 0.210063, CPU Cycle = 180131
Calc res = 0.210063, CPU Cycle = 180131
Calc res = 0.210063, CPU Cycle = 180131
Calc res = 0.210063, CPU Cycle = 180130
Calc res = 0.210063, CPU Cycle = 180132
````



## Test trigonometric operations

### Under normal FPU acceleration



````assembly
.text:08000F64 FMRRD R2, R3, D0
.text:08000F68 MOV R0, R4
.text:08000F6A MOV R1, R5
.text:08000F6C BL __aeabi_dadd
.text:08000F70 MOV R2, R0
.text:08000F72 MOV R3, R1
.text:08000F74 MOV R0, R2
.text:08000F76 MOV R1, R3
.text:08000F78 BL __truncdfsf2
````

At this time, the calculation result is

````
Calc res = 1.217216, CPU Cycle = 30270500
Calc res = 1.217216, CPU Cycle = 30270500
Calc res = 1.217216, CPU Cycle = 30270505
Calc res = 1.217216, CPU Cycle = 30270395
Calc res = 1.217216, CPU Cycle = 30270505
Calc res = 1.217216, CPU Cycle = 30270500
Calc res = 1.217216, CPU Cycle = 30270529
Calc res = 1.217216, CPU Cycle = 29863026
````



### ARM_MATH

````assembly
.text:08000F64 FMRRD R2, R3, D0
.text:08000F68 MOV R0, R4
.text:08000F6A MOV R1, R5
.text:08000F6C BL __aeabi_dadd
.text:08000F70 MOV R2, R0
.text:08000F72 MOV R3, R1
.text:08000F74 MOV R0, R2
.text:08000F76 MOV R1, R3
.text:08000F78 BL __truncdfsf2
````

At this time, the calculation result is

````
Calc res = 0.250030, CPU Cycle = 1100727
Calc res = 0.250030, CPU Cycle = 1100726
Calc res = 0.250030, CPU Cycle = 1100727
Calc res = 0.250030, CPU Cycle = 1100724
Calc res = 0.250030, CPU Cycle = 1100727
Calc res = 0.250030, CPU Cycle = 1100727
Calc res = 0.250030, CPU Cycle = 11007
