# 3\-3 按键控制LED 程序错误记录

## 一、程序整体说明

本次按键控制LED程序整体代码结构、时钟配置、GPIO初始化框架、按键消抖逻辑基本正确，能够实现基础功能逻辑。但存在**三处关键代码错误**，导致按键无法正常控制LED翻转、功能完全失效。

## 二、详细错误分析与修正

### 错误1：LED引脚模式配置错误（输入模式代替输出模式）

**错误位置**：LED\_Init\(\) 初始化函数

**错误代码**：

```Plain Text
GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU; // 错误：上拉输入模式
```

**错误原因**：LED 属于输出外设，需要 GPIO 主动输出高低电平驱动亮灭。配置为上拉输入模式时，引脚为高阻输入状态，无驱动能力，LED 永远无法点亮。

**修正代码**：

```Plain Text
GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP; // 正确：通用推挽输出模式
```

### 错误2：LED状态读取寄存器使用错误

**错误位置**：LED1\_Turn\(\)、LED2\_Turn\(\) 状态翻转函数

**错误代码**：

```Plain Text
GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0); // 错误：读取输入寄存器IDR
```

**错误原因**：`GPIO_ReadInputDataBit()` 读取的是**输入数据寄存器\(IDR\)**，仅用于读取外部引脚输入电平。引脚配置为输出模式时，IDR 数值与输出寄存器\(ODR\)状态不一致，无法准确获取LED当前状态，导致翻转逻辑错乱。

**修正代码**：

```Plain Text
// 读取输出寄存器ODR，获取引脚输出状态
if(GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_0) == 1)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_0);
}
else
{
    GPIO_SetBits(GPIOA, GPIO_Pin_0);
}
```

### 错误3：按键读取函数无返回值、逻辑不严谨

**错误位置**：Key\_GetNum\(\) 按键扫描函数

**错误问题**：

1. 函数定义为有返回值 `uint8_t`，但无 `return` 语句，返回随机垃圾值，按键逻辑失效；

2. 局部变量 `keyNum` 未初始化，存在编译警告；

3. 双独立 if 判断，多按键同时触发时逻辑冲突。

**错误原代码**：

```Plain Text
uint8_t Key_GetNum(void)
{
    uint8_t keyNum;
    if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)
    {
        // 按键消抖逻辑
        keyNum = 1;
    }
    if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0)
    {
        // 按键消抖逻辑
        keyNum = 2;
    }
    // 无返回值
}
```

**修正代码**：

```Plain Text
uint8_t Key_GetNum(void)
{
    uint8_t keyNum = 0; // 初始化为0，默认无按键
    if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)
    {
        Delay_ms(10);
        while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0);
        Delay_ms(10);
        keyNum = 1;
    }
    else if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0) // 互斥判断
    {
        Delay_ms(10);
        while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0);
        Delay_ms(10);
        keyNum = 2;
    }
    return keyNum; // 返回按键状态
}
```

## 三、错误修改汇总

|文件|错误内容|修改方案|
|---|---|---|
|LED\.c|LED引脚配置为上拉输入模式|改为通用推挽输出模式（GPIO\_Mode\_Out\_PP）|
|LED\.c|读取LED状态使用输入寄存器函数|替换为输出寄存器读取函数 GPIO\_ReadOutputDataBit|
|Key\.c|按键函数无返回值、变量未初始化、双if逻辑冲突|变量初始化、改为else if、添加return返回值|

## 四、总结

本次功能失效的核心原因是：GPIO输入输出模式混淆、寄存器读写混用、函数语法逻辑不规范。修正以上三处错误后，程序可正常实现按键消抖、按键识别、LED电平翻转功能。

> （注：部分内容可能由 AI 生成）
