# 比较FPU与DSP的提速性能

## 测试平台

* MCU: `STM32F401CCU6`, Freq = 84Mhz, 内部晶振
* 编译器: `arm-none-eabi-gcc`,优化选项`-Og -g`(开启优化之后，循环会被优化掉没有意义)
* 烧录工具: `openOCD`
* Debug工具: `DAPLink`

## 测试方法

> `Arm`内核中的`DWT`计数器记录了CPU工作以来的节拍,通过比较算法运行前后节拍的对比，就额可以很容易的得出计算经过了多少个时钟周期了。

## 测试浮点数运算

### 软件浮点

启用`ENABLE_TEST_FLOAT`

禁用FPU，此时汇编文件中出现了`mul`字样表明调用了乘法器相乘，而不是在编译的时候就已经把结果计算出来了

```assembly
.text:0800106E ; ---------------------------------------------------------------------------
.text:0800106E
.text:0800106E loc_800106E                             ; CODE XREF: main+34↓j
.text:0800106E i = R4                                  ; int
.text:0800106E                 LDR     R0, [SP,#0x28+a]
.text:08001070                 LDR     R1, [SP,#0x28+b]
.text:08001072                 BL      __mulsf3
.text:08001076                 STR     R0, [SP,#0x28+res]
```

此时的输出如下:

(通过计算10000次`res = float * float`得出数据)

```
Calc res = 0.210063, CPU Cycle = 442087
Calc res = 0.210063, CPU Cycle = 442087
Calc res = 0.210063, CPU Cycle = 449550
Calc res = 0.210063, CPU Cycle = 442087
Calc res = 0.210063, CPU Cycle = 442087
Calc res = 0.210063, CPU Cycle = 442087
Calc res = 0.210063, CPU Cycle = 449550
Calc res = 0.210063, CPU Cycle = 442087
```

### 使用定点数

> 由于库文件的限制，在这个平台上面启用定点数得打开FPU

#### ARM_MATH

```assembly
.text:08000F6E ; ---------------------------------------------------------------------------
.text:08000F6E
.text:08000F6E loc_8000F6E                             ; CODE XREF: main+36↓j
.text:08000F6E i = R4                                  ; int ; blockSize
.text:08000F6E                 MOVS    R3, #1
.text:08000F70                 ADD     R2, SP, #0x40+q_tmp+8 ; pDst
.text:08000F72                 ADD     R1, SP, #0x40+q_tmp+4 ; pSrcB
.text:08000F74                 ADD     R0, SP, #0x40+q_tmp ; pSrcA
.text:08000F76                 BL      arm_mult_q31
```

计算结果

```
Calc res = 0.210063, CPU Cycle = 590317
Calc res = 0.210063, CPU Cycle = 590313
Calc res = 0.210063, CPU Cycle = 590317
Calc res = 0.210063, CPU Cycle = 590317
Calc res = 0.210063, CPU Cycle = 590313
Calc res = 0.210063, CPU Cycle = 590317
Calc res = 0.210063, CPU Cycle = 590317
```

#### IQ_MATH

由于`IQMath`的汇编代码里面没有把计算结果返回，故而无法测试

```C
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
```

### FPU

```assembly
.text:08000E98 ; ---------------------------------------------------------------------------
.text:08000E98
.text:08000E98 loc_8000E98                             ; CODE XREF: main+4E↓j
.text:08000E98                 FLDS    S14, [R7,#0x18+a]
.text:08000E9C                 FLDS    S15, [R7,#0x18+b]
.text:08000EA0                 FMULS   S15, S14, S15
.text:08000EA4                 FSTS    S15, [R7,#0x18+res]
.text:08000EA8                 LDR     R3, [R7,#0x18+i]
.text:08000EAA                 ADDS    R3, #1
```

计算结果如下

```
Calc res = 0.210063, CPU Cycle = 180138
Calc res = 0.210063, CPU Cycle = 180132
Calc res = 0.210063, CPU Cycle = 180131
Calc res = 0.210063, CPU Cycle = 180131
Calc res = 0.210063, CPU Cycle = 180131
Calc res = 0.210063, CPU Cycle = 180130
Calc res = 0.210063, CPU Cycle = 180132
```



## 测试三角函数运算

### 普通的FPU加速下



```assembly
.text:08000F64                 FMRRD   R2, R3, D0
.text:08000F68                 MOV     R0, R4
.text:08000F6A                 MOV     R1, R5
.text:08000F6C                 BL      __aeabi_dadd
.text:08000F70                 MOV     R2, R0
.text:08000F72                 MOV     R3, R1
.text:08000F74                 MOV     R0, R2
.text:08000F76                 MOV     R1, R3
.text:08000F78                 BL      __truncdfsf2
```

此时计算结果为

```
Calc res = 1.217216, CPU Cycle = 30270500
Calc res = 1.217216, CPU Cycle = 30270500
Calc res = 1.217216, CPU Cycle = 30270505
Calc res = 1.217216, CPU Cycle = 30270395
Calc res = 1.217216, CPU Cycle = 30270505
Calc res = 1.217216, CPU Cycle = 30270500
Calc res = 1.217216, CPU Cycle = 30270529
Calc res = 1.217216, CPU Cycle = 29863026
```



### ARM_MATH

```assembly
.text:08000F64                 FMRRD   R2, R3, D0
.text:08000F68                 MOV     R0, R4
.text:08000F6A                 MOV     R1, R5
.text:08000F6C                 BL      __aeabi_dadd
.text:08000F70                 MOV     R2, R0
.text:08000F72                 MOV     R3, R1
.text:08000F74                 MOV     R0, R2
.text:08000F76                 MOV     R1, R3
.text:08000F78                 BL      __truncdfsf2
```

此时计算结果为

```
Calc res = 0.250030, CPU Cycle = 1100727
Calc res = 0.250030, CPU Cycle = 1100726
Calc res = 0.250030, CPU Cycle = 1100727
Calc res = 0.250030, CPU Cycle = 1100724
Calc res = 0.250030, CPU Cycle = 1100727
Calc res = 0.250030, CPU Cycle = 1100727
Calc res = 0.250030, CPU Cycle = 1100724
Calc res = 0.250030, CPU Cycle = 1100733
```

### IQ_MATH

这里的IQMath还是没有返回值

```C
  while ( 1 )
  {
    rr = 0;
    before = DWT_TS_GET();
    for ( i = 0; i <= 9999; ++i )
    {
      v3 = IQ20cos((int)(float)(0.5569 * 1048600.0));
      rr = IQ20cos((int)(float)(0.3772 * 1048600.0)) + v3;
    }
    time = DWT_TS_GET() - before;
    IQ20toF(rr);
    printf_("Calc res = %f, CPU Cycle = %d\n", res, time);
    HAL_Delay(0x1F4u);
  }
}
```

## 结论

在启用`FPU`的情况下，浮点数运算得到明显的加速

且在`ARM_MATH`库的加持下，运算进步明显
