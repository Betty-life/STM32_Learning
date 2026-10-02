# 6-3 PWM驱动LED呼吸灯学习记录

## 一、实验功能

使用 TIM2（Timer 2，定时器 2）通道 1（PA0）输出 PWM（Pulse Width Modulation，脉冲宽度调制）波，通过调节 CCR 值改变占空比，控制 LED（Light-Emitting Diode，发光二极管）亮度，实现呼吸灯效果。

## 二、硬件接线

| 外设 | 引脚 | 说明 |
| --- | --- | --- |
| LED | PA0 | TIM2 通道 1（CH1）PWM 输出，引脚配置为复用推挽输出（AF_PP，Alternate Function Push-Pull） |
| OLED | PB8 | SCL（Serial Clock Line，串行时钟线） |
| OLED | PB9 | SDA（Serial Data Line，串行数据线） |

## 三、核心概念

### 1. 输出比较（Output Compare）

输出比较是定时器的一个功能：计数器 CNT（Counter，计数器）不断计数，当 CNT 的值与捕获/比较寄存器 CCR（Capture/Compare Register）的值相等时，输出引脚电平发生翻转。

**关键寄存器**：
- **CNT**：当前计数值
- **CCR**：捕获/比较寄存器，你写入的值
- **ARR**（Auto-Reload Register，自动重装载寄存器）：自动重装载值，决定 PWM 周期

### 2. PWM（脉冲宽度调制）

PWM 是一种用数字信号模拟模拟量的技术：
- **频率**：由 PSC（Prescaler，预分频器）和 ARR 决定
- **占空比**：由 CCR 和 ARR 决定

**占空比公式**：

\[
占空比 = \frac{CCR}{ARR+1} \times 100\%
\]

**频率公式**：

\[
PWM频率 = \frac{CK\_PSC}{(PSC+1) \times (ARR+1)}
\]

下图是 PWM 原理示意：上方锯齿波表示 CNT 计数过程，红色水平线对应 CCR 的值，两者相等时输出翻转，形成下方绿色矩形脉冲波；图中还给出频率、占空比、分辨率三个公式。

![PWM 原理示意图](https://aka.doubaocdn.com/s/8YU8IAyWCd)

### 3. PWM 模式

STM32 的 PWM 模式 1 和模式 2：

| 模式 | 说明 |
|---|---|
| PWM 模式 1 | CNT < CCR 时输出有效电平，CNT ≥ CCR 时输出无效电平 |
| PWM 模式 2 | CNT < CCR 时输出无效电平，CNT ≥ CCR 时输出有效电平 |

**常用 PWM 模式 1**，配合有效电平为高，实现"CCR 越大，高电平时间越长"。

## 四、关键函数详解

### 1. `GPIO_Mode_AF_PP`（复用推挽输出）

PWM 输出引脚必须配成**复用推挽输出**（AF_PP），因为信号由定时器外设控制，不是普通 GPIO（General-Purpose Input/Output，通用输入/输出）控制。
**如果配成 `GPIO_Mode_Out_PP`**（普通推挽输出），PWM 信号出不来，LED 不会呼吸。

### 2. `TIM_OCInitTypeDef` 结构体

（OC，Output Compare，输出比较）

| 字段 | 作用 | 常用值 |
|---|---|---|
| `TIM_OCMode` | PWM 模式 | `TIM_OCMode_PWM1` |
| `TIM_OCPolarity` | 输出极性 | `TIM_OCPolarity_High` |
| `TIM_OutputState` | 输出使能 | `TIM_OutputState_Enable` |
| `TIM_Pulse` | CCR 初始值 | 0 ~ ARR |

### 3. `TIM_OC1Init(TIM2, &TIM_OCInitStructure)`

**作用**：初始化 TIM2 的通道 1。
**通道对应关系**：
- `TIM_OC1Init` → 通道 1 → PA0
- `TIM_OC2Init` → 通道 2 → PA1
- `TIM_OC3Init` → 通道 3 → PA2
- `TIM_OC4Init` → 通道 4 → PA3

### 4. `TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable)`

**作用**：使能 CCR1 的预装载功能。
**为什么需要**：不使能预装载，`TIM_SetCompare1()` 修改 CCR 时会立即生效，可能在 PWM 周期中间导致输出异常脉冲。使能后，CCR 的修改在下一个更新事件时生效，波形更平滑。

### 5. `TIM_SetCompare1(TIM2, Compare)`

**作用**：设置通道 1 的 CCR 值，改变占空比。
**参数**：`Compare` 范围 0 ~ ARR。
**占空比**：`Compare / (ARR+1) × 100%`。

## 五、参数计算

### 本实验配置

- CK_PSC = 72MHz
- PSC = 720 - 1 = 719
- ARR = 100 - 1 = 99

### PWM 频率

\[
PWM频率 = \frac{72,000,000}{(719+1) \times (99+1)} = \frac{72,000,000}{720 \times 100} = 1000Hz = 1kHz
\]

### 占空比

| CCR | 占空比 |
|---|---|
| 0 | 0% |
| 25 | 25% |
| 50 | 50% |
| 75 | 75% |
| 99 | 99% |

## 六、踩坑记录

### 错误 1：GPIO 模式配成 `GPIO_Mode_Out_PP`

**现象**：PWM 信号出不来，LED 不呼吸。
**原因**：PWM 输出必须配成**复用推挽输出** `GPIO_Mode_AF_PP`。

### 错误 2：忘记调用 `TIM_Cmd(TIM2, ENABLE)`

**现象**：配置全对，但 PWM 不输出。
**原因**：`TIM_Cmd` 是定时器总开关，不调用计数器不计数，PWM 不输出。

### 错误 3：`TIM_OCStructInit` 没调用

**现象**：结构体里有随机值，配置异常。
**原因**：`TIM_OCStructInit` 把结构体填充为默认值，不调用可能残留随机值。

### 错误 4：CCR 值超过 ARR

**现象**：占空比异常。
**原因**：CCR 应小于等于 ARR，超过后占空比恒为 100%。

### 错误 5：缺少 `TIM_OC1PreloadConfig`

**现象**：呼吸灯过渡有卡顿感。
**原因**：不使能预装载，CCR 修改在周期中间生效，可能出现异常脉冲。使能后波形更平滑。

## 七、经验总结

1. **PWM 输出引脚必须配成 `GPIO_Mode_AF_PP`**（复用推挽输出）。
2. **PWM 频率由 PSC 和 ARR 决定**，占空比由 CCR 和 ARR 决定。
3. **PWM 模式 1 是最常用的模式**，CCR 越大高电平时间越长。
4. **通道 1 对应 PA0，通道 2 对应 PA1**，不同通道对应不同引脚。
5. **`TIM_SetCompare1` 可以动态改变占空比**，实现呼吸灯效果。
6. **呼吸灯的本质是 CCR 值从小到大再到小循环变化**，配合延时，人眼看到的就是亮度渐变。
7. **`TIM_OC1PreloadConfig` 使能预装载**，让 CCR 修改在周期边界生效，波形更平滑。
8. **万用表测不出占空比**，只能测平均电压估算。要精确测频率和脉宽，需要逻辑分析仪或示波器。

## 附：调试端口（SWJ_CFG）引脚占用参考

STM32 的调试接口（SWJ，Serial Wire/JTAG）默认占用 PA13/PA14/PA15/PB3/PB4 五个引脚，通过配置 SWJ_CFG[2:0]（位于 AFIO_MAPR 寄存器）可以改变占用情况：

- **000（默认）**：JTAG-DP + SW-DP 全开，5 个引脚全部不可用作 GPIO。
- **010**：关闭 JTAG-DP、保留 SW-DP，PA15/PB3/PB4 释放为 GPIO（PB3 仅在不使用异步跟踪时可用），最常用。
- **100**：JTAG-DP 与 SW-DP 全部关闭，5 个引脚全部释放，但之后无法再用调试器调试。

![表35 调试端口映像](https://aka.doubaocdn.com/s/uzo3fIPi00)

> 排查提示：若某个引脚"怎么配置都不生效"，先查它是否被调试接口占用（见上表引脚列）。
