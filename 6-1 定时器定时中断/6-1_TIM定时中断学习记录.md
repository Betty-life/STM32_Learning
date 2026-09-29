# 6-1 TIM定时中断学习记录

## 一、实验功能

使用 TIM2 定时器，每秒触发一次更新中断，在中断里让 `Timer_num` 自增，OLED 显示计数值。

---

## 二、硬件接线

| 外设 | 引脚 |
|---|---|
| OLED SCL | PB8 |
| OLED SDA | PB9 |

---

## 三、代码结构

### Timer.c

```c
#include "stm32f10x.h"

extern uint16_t Timer_num;

void Timer_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_InternalClockConfig(TIM2);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 10000 - 1;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 7200 - 1;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

    TIM_ClearFlag(TIM2, TIM_IT_Update);        // 清除更新标志位，防止初始化时产生假中断
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); // 使能更新中断

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM2, ENABLE);
}

void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {
        Timer_num++;
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}
```

### main.c

```c
#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "Timer.h"

uint16_t Timer_num;

int main(void)
{
    OLED_Init();
    Timer_Init();

    OLED_ShowString(1, 1, "HelloWorld!");
    OLED_ShowString(2, 1, "Num:");

    while (1)
    {
        OLED_ShowNum(2, 5, Timer_num, 5);
    }
}
```

---

## 四、关键函数详解

### 1. `TIM_InternalClockConfig(TIM2);`

**作用**：选择定时器的时钟源为内部时钟（CK_INT）。
**什么时候用**：定时器默认就是用内部时钟，这行代码可以省略，但写上更清晰。如果要用外部时钟，才需要改成 `TIM_ETRClockMode2Config()` 等函数。

---

### 2. `TIM_ClearFlag(TIM2, TIM_IT_Update);`

**作用**：清除更新标志位（SR 寄存器里的 UIF 位）。
**什么时候用**：在 `TIM_TimeBaseInit()` 之后、`TIM_ITConfig()` 之前调用。
**为什么必须用**：`TIM_TimeBaseInit()` 初始化时，可能会产生一次"假的"更新事件，把 UIF 标志位置 1。如果不先清除，一开启中断就会立刻进一次中断，导致第一次计数不准。

---

### 3. `TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);`

**作用**：使能定时器的更新中断（UIE 位）。
**什么时候用**：在 `TIM_TimeBaseInit()` 之后调用。
**不调用会怎样**：定时器溢出时不会触发中断，`TIM2_IRQHandler` 永远不会执行。

---

### 4. `TIM_Cmd(TIM2, ENABLE);`

**作用**：启动定时器（使能计数器 CNT 开始计数）。
**什么时候用**：所有配置完成后，最后一步调用。
**不调用会怎样**：定时器配置全对，但 CNT 不计数，永远不会溢出，中断永远不触发。

---

## 五、定时器溢出频率公式

\[
CK\_CNT\_OV = \frac{CK\_PSC}{(PSC+1) \times (ARR+1)}
\]

### 本实验配置

- CK_PSC = 72MHz（TIM2 挂在 APB1 总线上）
- PSC = 7200 - 1 = 7199
- ARR = 10000 - 1 = 9999

### 计算过程

\[
CK\_CNT = \frac{72,000,000}{7199+1} = \frac{72,000,000}{7200} = 10,000Hz
\]

\[
CK\_CNT\_OV = \frac{10,000}{9999+1} = \frac{10,000}{10,000} = 1Hz
\]

**结论**：定时器每秒溢出一次，即每秒触发一次中断。

---

## 六、正确的初始化顺序

```
1. 开时钟        RCC_APB1PeriphClockCmd
2. 选时钟源      TIM_InternalClockConfig
3. 配时基单元    TIM_TimeBaseInit
4. 清标志位      TIM_ClearFlag
5. 使能中断      TIM_ITConfig
6. 配 NVIC       NVIC_Init
7. 启动定时器    TIM_Cmd
```

**顺序不能乱**：`TIM_Cmd` 必须放最后，否则定时器可能在配置未完成时就开始计数。

---

## 七、踩坑记录

### 错误 1：缺少 `TIM_ClearFlag`

**现象**：程序一启动，`Timer_num` 就变成 1，第一次计数不准。
**原因**：`TIM_TimeBaseInit()` 初始化时可能产生一次假的更新事件，把 UIF 标志位置 1。如果不先清除，一开启中断就会立刻进一次中断。
**正确**：在 `TIM_TimeBaseInit()` 之后、`TIM_ITConfig()` 之前调用 `TIM_ClearFlag(TIM2, TIM_IT_Update);`。

### 错误 2：`Timer_num` 类型不够大

`uint16_t` 最大 65535，每秒加一，约 18 小时后溢出归零。如果做长时间计时，建议改成 `uint32_t`。

### 错误 3：中断函数名写错

TIM2 对应的中断函数名是 `TIM2_IRQHandler`，必须和启动文件一致。

### 错误 4：忘记清除中断标志位

在 `TIM2_IRQHandler` 里必须调用 `TIM_ClearITPendingBit(TIM2, TIM_IT_Update);`，否则中断会连续触发，主程序卡死。

---

## 八、经验总结

1. **初始化顺序不能乱**：开时钟 → 选时钟源 → 配时基 → 清标志 → 使能中断 → 配 NVIC → 启动定时器。
2. **`TIM_ClearFlag` 容易被忽略**，但不加可能导致第一次计数不准。
3. **`TIM_ITConfig` 管"要不要触发中断"，`TIM_Cmd` 管"定时器要不要开始计数"**，两者作用不同，不能混淆。
4. **中断函数名、NVIC 通道号、定时器编号，三者必须匹配**。
5. **中断里必须清除标志位**，否则中断连续触发。
6. **PSC 决定计数频率，ARR 决定溢出频率**，两者配合决定定时时间。
