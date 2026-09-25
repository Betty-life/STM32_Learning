# 3\-1 LED流水灯错误记录

## 一、错误现象

LED 流水灯不亮，或只有部分 LED 亮，或亮灯顺序混乱。

---

## 二、常见错误与原因

### 错误 1：GPIO 模式配置错误

**错误代码：**

```Plain Text
GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;  // ❌ 上拉输入
```

**原因：**

LED 是输出设备，必须配成推挽输出。配成输入模式后，GPIO 引脚处于高阻态，无法驱动 LED，LED 永远不会亮。

**正确代码：**

```Plain Text
GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;  // ✅ 推挽输出
```

---

### 错误 2：时钟开启与 GPIO 初始化端口不一致

**错误代码：**

```Plain Text
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);  // 开的是C口时钟
GPIO_Init(GPIOA, &GPIO_InitStruct);                    // 初始化的是A口
```

**原因：**

时钟、初始化、操作三者必须使用同一个 GPIO 端口。开 GPIOC 时钟却初始化 GPIOA，GPIOA 没有时钟，初始化不生效；GPIOC 有时钟但没被初始化，引脚无法输出。

**正确代码：**

```Plain Text
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  // 开A口时钟
GPIO_Init(GPIOA, &GPIO_InitStruct);                    // 初始化A口
GPIO_ResetBits(GPIOA, GPIO_Pin_0);                     // 操作A口
```

---

### 错误 3：复位函数误用为时钟函数

**错误代码：**

```Plain Text
RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOB, ENABLE);  // ❌ 这是复位，不是开时钟
```

**原因：**

`RCC_APB2PeriphResetCmd` 的作用是复位外设寄存器，不是开启时钟。写错这个词，GPIO 根本没有时钟，后面所有配置都不会生效。

**正确代码：**

```Plain Text
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  // ✅ 开时钟
```

---

### 错误 4：LED 引脚与代码不一致

**错误现象：**

代码操作 PB12，但板子上 LED 接在 PC13，灯不亮。

**原因：**

江协板子 LED 通常接 **PC13**，不是 PB12。写代码前必须确认 LED 实际接在哪个引脚。

**正确做法：**

看板子丝印或原理图，确认 LED 对应的引脚，代码里操作对应的 GPIO 端口和引脚号。

---

### 错误 5：延时函数未包含头文件

**错误代码：**

```Plain Text
#include "stm32f10x.h"
// 缺少 #include "Delay.h"
```

**原因：**

使用 `Delay_ms()` 函数必须包含 `Delay.h`，否则编译器报错或链接失败。

**正确代码：**

```Plain Text
#include "stm32f10x.h"
#include "Delay.h"  // ✅ 必须包含
```

---

### 错误 6：流水灯顺序逻辑错误

**错误代码：**

```Plain Text
GPIO_ResetBits(GPIOC, GPIO_Pin_1);
Delay_ms(50);
GPIO_SetBits(GPIOC, GPIO_Pin_1);
Delay_ms(50);
GPIO_ResetBits(GPIOC, GPIO_Pin_2);  // ❌ 没有先关掉前一个
```

**原因：**

流水灯的核心是“**亮一个，灭一个，再亮下一个**”。如果忘记关掉前一个 LED，所有 LED 会同时亮，看不出流水效果。

**正确代码：**

```Plain Text
// 流水灯标准写法：先关全部，再逐个点亮
GPIO_SetBits(GPIOC, GPIO_Pin_All);  // 先全部熄灭
while(1)
{
    GPIO_ResetBits(GPIOC, GPIO_Pin_1);  // 点亮LED1
    Delay_ms(50);
    GPIO_SetBits(GPIOC, GPIO_Pin_1);    // 熄灭LED1
    GPIO_ResetBits(GPIOC, GPIO_Pin_2);  // 点亮LED2
    Delay_ms(50);
    GPIO_SetBits(GPIOC, GPIO_Pin_2);    // 熄灭LED2
    // ... 依此类推
}
```

---

### 错误 7：GPIO\_Pin\_All 初始化所有引脚，但只操作部分引脚

**错误代码：**

```Plain Text
GPIO_InitStruct.GPIO_Pin = GPIO_Pin_All;  // 初始化所有引脚
// 后面只操作 PC1、PC2、PC3、PC4
```

**原因：**

如果板子上 PC13 接了 LED，`GPIO_Pin_All` 会把 PC13 也初始化成推挽输出，但后面没操作它，PC13 保持默认状态，可能导致意外亮灭。

**正确做法：**

只初始化需要用到的引脚：

```Plain Text
GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
```

---

## 三、修改汇总

|文件|错误|修改|
|---|---|---|
|main\.c|GPIO\_Mode\_IPU 输入模式配置LED|改成 GPIO\_Mode\_Out\_PP 推挽输出模式|
|main\.c|时钟和初始化端口不一致|统一成同一个 GPIO 端口（时钟、初始化、操作一致）|
|main\.c|误用复位函数开启外设时钟|RCC\_APB2PeriphResetCmd 改为 RCC\_APB2PeriphClockCmd|
|main\.c|LED 引脚代码与硬件不一致|对照板子丝印/原理图，修改代码引脚为实际硬件引脚|
|main\.c|缺少延时函数头文件|添加 \#include "Delay\.h"|
|main\.c|流水灯逻辑缺失灭灯操作|优化逻辑，先灭全灯，逐一点亮熄灭，实现标准流水效果|
|main\.c|GPIO\_Pin\_All 初始化全部引脚|仅初始化项目需要使用的LED引脚|

---

## 四、经验总结

1\. **输出设备必须配成输出模式**：LED 用 GPIO\_Mode\_Out\_PP，禁止误用输入模式，否则无驱动能力。

2\. **时钟、初始化、操作三者必须统一端口**：开启哪个端口时钟，就初始化、操作哪个端口，一一对应。

3\. **区分时钟与复位函数**：开启外设时钟固定使用 RCC\_APB2PeriphClockCmd，禁止使用复位函数。

4\. **代码跟随硬件引脚**：编写GPIO代码前，必须核对开发板原理图和丝印，不凭记忆写引脚。

5\. **流水灯核心逻辑**：遵循“亮一个、灭一个”原则，先清空所有灯状态，再逐位跑动，避免全灯常亮。

6\. **精准初始化引脚**：不使用 GPIO\_Pin\_All 全局初始化，仅初始化所需引脚，避免影响其他外设。

> （注：部分内容可能由 AI 生成）
