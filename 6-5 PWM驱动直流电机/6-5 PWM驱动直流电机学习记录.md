# 6-5 PWM驱动直流电机学习记录

## 一、实验功能

使用 TIM2 通道 3（PA2）输出 PWM 波，配合方向控制引脚 PA4、PA5，控制直流电机的转速和方向。按键按下时速度增加 20，超过 99 后变为 -100，实现正反转切换，OLED 显示当前速度值。

---

## 二、硬件接线

| 外设 | 引脚 |
|---|---|
| 电机驱动模块 PWMA | PA2 |
| 电机驱动模块 AIN1 | PA4 |
| 电机驱动模块 AIN2 | PA5 |
| OLED SCL | PB8 |
| OLED SDA | PB9 |
| 按键 | PB11 |

---

## 三、核心知识点

### 1. 直流电机控制原理

直流电机的转速由 **PWM 占空比**决定，方向由**两个方向控制引脚的高低电平组合**决定。

以 TB6612 为例：

| AIN1 | AIN2 | 电机状态 |
|---|---|---|
| 0 | 0 | 停止（刹车） |
| 0 | 1 | 反转 |
| 1 | 0 | 正转 |
| 1 | 1 | 停止（刹车） |

**PWM 占空比越大，转速越快。**

### 2. PWM 频率选择

直流电机推荐 PWM 频率为 **10kHz~20kHz**，原因：
- 频率太低（<1kHz）：电机有啸叫，转动不平稳。
- 频率太高（>40kHz）：驱动模块可能响应不过来。
- **20kHz 高于人耳听觉上限**，电机运行安静。

本实验配置：PSC = 36 - 1 = 35，ARR = 100 - 1 = 99。
PWM 频率 = 72MHz / 36 / 100 = **20kHz**。

### 3. CCR 与转速的映射

ARR = 99，CCR 范围 0~99，对应占空比 0%~100%。

| CCR | 占空比 | 转速 |
|---|---|---|
| 0 | 0% | 停止 |
| 25 | 25% | 慢速 |
| 50 | 50% | 中速 |
| 99 | 99% | 全速 |

### 4. 方向控制逻辑

```
Speed > 0：PA4=1，PA5=0，PWM_SetCompare3(Speed)      → 正转
Speed < 0：PA4=0，PA5=1，PWM_SetCompare3(-Speed)     → 反转
Speed = 0：PA4=1，PA5=0，PWM_SetCompare3(0)          → 停止
```

**注意**：`PWM_SetCompare3()` 的参数是 `uint16_t`，不能传负数，所以反转时要传 `-Speed`。

---

## 四、参数设置详解

### 1. PSC 和 ARR 怎么设？

**目标**：PWM 频率 = 20kHz，CCR 范围 0~100。

**设置思路**：
- 先定 ARR = 100 - 1 = 99，让 CCR 范围 0~99，和速度值 0~100 对应。
- 算 PSC：72MHz / (PSC+1) / (ARR+1) = 20kHz → PSC+1 = 36 → **PSC = 35**。

**结论**：PSC = 35，ARR = 99，PWM 频率 = 20kHz。

### 2. 为什么用通道 3 + PA2？

TIM2 通道 3 默认对应 PA2。江协教程里电机实验用的就是 PA2 + 通道 3。

**通道和引脚对应关系**：
- 通道 1 → PA0
- 通道 2 → PA1
- 通道 3 → PA2
- 通道 4 → PA3

### 3. 方向控制引脚怎么配？

PA4 和 PA5 是**普通 GPIO 输出**，配成 `GPIO_Mode_Out_PP`（推挽输出），不是复用功能。

**PWM 引脚 PA2 配成 `GPIO_Mode_AF_PP`（复用推挽输出）。**

---

## 五、重点

1. **PWM 频率选 20kHz**：高于人耳听觉上限，电机运行安静。
2. **方向控制引脚配成普通推挽输出**：PA4、PA5 用 `GPIO_Mode_Out_PP`。
3. **PWM 引脚配成复用推挽输出**：PA2 用 `GPIO_Mode_AF_PP`。
4. **`Motor_SetSeed()` 参数必须是 `int8_t`**：因为速度有正负，表示方向。
5. **头文件声明和源文件定义必须一致**：类型不一致会导致隐蔽 bug。
6. **`PWM_SetCompare3()` 参数不能传负数**：反转时传 `-Speed`。

---

## 六、错误记录

### 错误 1：`Motor_SetSeed()` 参数类型写成 `uint8_t`

**现象**：Speed 显示 -20，但 PA4 一直 3.3V，PA5 一直 0V，电机不分方向。

**原因**：`MOTOR.h` 里声明的是 `void Motor_SetSeed(uint8_t Speed);`，`MOTOR.c` 里定义的是 `int8_t`。编译器按头文件声明调用，-20 被转成 236，`if(Speed >= 0)` 永远成立，走正转分支。

**解决**：把 `MOTOR.h` 里的 `uint8_t` 改成 `int8_t`，声明和定义保持一致。

**教训**：**头文件声明、源文件定义、函数调用，三处参数类型必须完全一致。**

### 错误 2：PWM 引脚和通道号不匹配

**现象**：PWM 不输出，电机不转。

**原因**：`GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;`（PA1）配的是通道 2 的引脚，但代码用的是 `TIM_OC3Init`（通道 3），通道 3 对应 PA2。

**解决**：引脚和通道必须匹配：
- PA0 → 通道 1 → `TIM_OC1Init` + `TIM_SetCompare1`
- PA1 → 通道 2 → `TIM_OC2Init` + `TIM_SetCompare2`
- PA2 → 通道 3 → `TIM_OC3Init` + `TIM_SetCompare3`
- PA3 → 通道 4 → `TIM_OC4Init` + `TIM_SetCompare4`

### 错误 3：`PWM_Init()` 里缺少 GPIOA 时钟

**现象**：电机不转。

**原因**：`PWM_Init()` 里配置了 PA2，但没开 GPIOA 时钟。

**解决**：在 `PWM_Init()` 里加上 `RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);`。

### 错误 4：方向控制引脚配成 `GPIO_Mode_AF_PP`

**现象**：方向控制失效，电机只单向转。

**原因**：PA4、PA5 是普通 GPIO 输出，不是复用功能。配成 `GPIO_Mode_AF_PP` 后，引脚不受 ODR 控制，`GPIO_SetBits()` 和 `GPIO_ResetBits()` 无效。

**解决**：PA4、PA5 配成 `GPIO_Mode_Out_PP`。

---

## 七、经验总结

1. **PWM 频率选 20kHz**，高于人耳听觉上限，电机运行安静。
2. **PWM 引脚配 `GPIO_Mode_AF_PP`，方向控制引脚配 `GPIO_Mode_Out_PP`**。
3. **引脚和通道必须匹配**：PA0→通道1，PA1→通道2，PA2→通道3，PA3→通道4。
4. **`Motor_SetSeed()` 参数必须是 `int8_t`**，头文件声明和源文件定义必须一致。
5. **`PWM_SetCompareX()` 参数不能传负数**，反转时传 `-Speed`。
6. **头文件声明和源文件定义不一致是隐蔽 bug**，编译器可能不报错，但运行结果完全错误。
