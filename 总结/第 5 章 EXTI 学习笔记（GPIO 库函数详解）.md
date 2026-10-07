# 第 5 章：EXTI 外部中断（红外计次、编码器计次）学习笔记

> 这一章用中断，但先要把 GPIO 库函数看懂。下面 18 个函数一句话讲清。

## 常用类型

- `GPIO_TypeDef* GPIOx`：端口，取值 `GPIOA`~`GPIOG`
- `GPIO_InitTypeDef` 结构体：`GPIO_Pin`（哪个引脚，可 `|` 组合）+ `GPIO_Mode`（模式）+ `GPIO_Speed`（速度）
- 输入模式常用 `GPIO_Mode_IPU`（上拉输入，红外/编码器信号常用）

## 18 个函数速查

| 函数 | 作用 |
|------|------|
| `GPIO_DeInit(GPIOx)` | 复位整个端口到默认值 |
| `GPIO_AFIODeInit()` | 复位 AFIO（重映射、EXTI 选线等） |
| `GPIO_Init(GPIOx, &s)` | **核心**：按结构体配置引脚模式/速度 |
| `GPIO_StructInit(&s)` | 结构体填默认值（只赋值，不写寄存器） |
| `GPIO_ReadInputDataBit(GPIOx, Pin)` | 读某个引脚输入电平（0/1） |
| `GPIO_ReadInputData(GPIOx)` | 读整个端口输入（16 位） |
| `GPIO_ReadOutputDataBit(GPIOx, Pin)` | 读某个引脚输出电平 |
| `GPIO_ReadOutputData(GPIOx)` | 读整个端口输出 |
| `GPIO_SetBits(GPIOx, Pin)` | 输出置高 |
| `GPIO_ResetBits(GPIOx, Pin)` | 输出置低 |
| `GPIO_WriteBit(GPIOx, Pin, BitVal)` | 按 BitVal 置高(Bit_SET)/低(Bit_RESET) |
| `GPIO_Write(GPIOx, val)` | 整端口一次写 16 位 |
| `GPIO_PinLockConfig(GPIOx, Pin)` | 锁定引脚配置（锁定后不可改） |
| `GPIO_EventOutputConfig/Cmd` | 事件输出配置/开关（少用） |
| `GPIO_PinRemapConfig(Remap, 状态)` | 引脚重映射（AFIO） |
| `GPIO_EXTILineConfig(端口, 引脚)` | **EXTI 选线（第5章核心）** |
| `GPIO_ETH_MediaInterfaceConfig(..)` | 以太网接口配置（可忽略） |

## 第 5 章必背三步（以 PA0 外部中断为例）

```c
GPIO_Init(GPIOA, ...);                                        // 1. 引脚配成输入
GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);   // 2. 选 EXTI0 线
EXTI_Init(...);                                               // 3. 配触发方式
```

## 记忆要点

1. `GPIO_Init` 才是真配置寄存器，`GPIO_StructInit` 只是给结构体填默认值。
2. 读输入看 IDR、读写输出看 ODR；置高 `SetBits`、置低 `ResetBits`。
3. 带 `Bit` 的管单个引脚，不带的管整个端口。
4. 红外/编码器计次：引脚配输入 → `GPIO_EXTILineConfig` 选线 → 进中断函数里 `Count++`。
