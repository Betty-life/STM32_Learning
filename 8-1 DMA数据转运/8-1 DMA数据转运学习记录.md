# 8-1 DMA数据转运学习记录

## 一、实验功能

使用 DMA1 通道 1，把 DataA 数组的 4 个字节搬运到 DataB 数组。OLED 显示搬运前后 DataA 和 DataB 的值，验证 DMA 是否正常工作。

---

## 二、硬件接线

| 外设 | 引脚 |
|---|---|
| OLED SCL | PB8 |
| OLED SDA | PB9 |

**本实验不需要外部硬件，纯内存到内存的 DMA 搬运。**

---

## 三、核心知识点

### 1. DMA 是什么

DMA（Direct Memory Access，直接存储器访问）让数据在外设和内存之间**自动搬运**，不需要 CPU 参与。

**三个搬运方向**：
- 外设 → 内存（ADC 采样）
- 内存 → 外设（串口发送）
- **内存 → 内存（本实验）**

### 2. DMA 的核心参数

| 参数 | 含义 | 本实验值 |
|---|---|---|
| **外设地址** | 数据从哪来 | `DataA` 的地址 |
| **内存地址** | 数据到哪去 | `DataB` 的地址 |
| **数据宽度** | 每次搬多少字节 | `DMA_PeripheralDataSize_Byte`（8 位） |
| **地址自增** | 搬完后地址是否自动加一 | `DMA_PeripheralInc_Enable` |
| **传输方向** | 从外设到内存，还是反过来 | `DMA_DIR_PeripheralSRC`（外设是源） |
| **传输数量** | 搬多少个数据 | 4 |
| **模式** | 普通模式或循环模式 | `DMA_Mode_Normal`（普通模式） |
| **M2M** | 内存到内存模式 | `DMA_M2M_Enable` |
| **优先级** | 多个 DMA 同时请求时的优先级 | `DMA_Priority_Medium` |

### 3. 内存到内存模式（M2M）

**普通 DMA** 是"外设 ↔ 内存"，需要外设触发。
**M2M 模式**是"内存 ↔ 内存"，**软件触发，不需要外设**。

**本实验用 M2M 模式**，所以不需要 ADC、串口等外设触发，直接调用 `DMA_Cmd` 就能搬。

### 4. 普通模式 vs 循环模式

| 模式 | 特点 |
|---|---|
| **普通模式（Normal）** | 搬完一轮就停，等待下次触发 |
| **循环模式（Circular）** | 搬完一轮自动重新开始，一直搬 |

**本实验用普通模式**，每次手动触发搬运。

### 5. DMA 传输流程

```
1. 初始化 DMA：配置地址、宽度、方向、数量、模式
2. 关闭 DMA 通道：等待触发
3. 触发搬运：
   - 关闭 DMA
   - 重置传输数量
   - 使能 DMA
4. 等待传输完成：检查 TC 标志位
5. 清除 TC 标志位
```

### 6. 关键函数

| 函数 | 作用 |
|---|---|
| `RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE)` | 开 DMA1 时钟 |
| `DMA_Init(DMA1_Channel1, &DMA_InitStructure)` | 初始化 DMA 通道 |
| `DMA_Cmd(DMA1_Channel1, ENABLE)` | 使能 DMA 通道 |
| `DMA_SetCurrDataCounter(DMA1_Channel1, Size)` | 设置传输数量 |
| `DMA_GetFlagStatus(DMA1_FLAG_TC1)` | 检查传输完成标志 |
| `DMA_ClearFlag(DMA1_FLAG_TC1)` | 清除传输完成标志 |

---

## 四、参数设置详解

### 1. DMA_PeripheralBaseAddr 和 DMA_MemoryBaseAddr

```c
void MyDMA_Init(uint32_t AddrA, uint32_t AddrB, uint16_t Size)
{
    ...
    DMA_InitStructure.DMA_PeripheralBaseAddr = AddrA;   // 源地址，main 里传 (uint32_t)DataA
    DMA_InitStructure.DMA_MemoryBaseAddr = AddrB;       // 目标地址，main 里传 (uint32_t)DataB
    ...
}
```

**M2M 模式下**，"外设地址"和"内存地址"只是两个普通地址，一个当源，一个当目标。

### 2. DMA_DIR

```c
DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;   // 外设是源
```

- `DMA_DIR_PeripheralSRC`：外设是源，内存是目标。
- `DMA_DIR_PeripheralDST`：外设是目标，内存是源。

**M2M 模式下**，`PeripheralSRC` 表示从"外设地址"搬到"内存地址"。

### 3. DMA_PeripheralInc 和 DMA_MemoryInc

```c
DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;   // 源地址自增
DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;           // 目标地址自增
```

**自增的含义**：每搬一个数据，地址自动加一。

- 搬 `DataA[0]` 到 `DataB[0]`，地址加一。
- 搬 `DataA[1]` 到 `DataB[1]`，地址加一。
- …

**如果外设是固定地址**（比如 ADC 数据寄存器），`DMA_PeripheralInc` 要设成 `Disable`，因为地址不变。

### 4. DMA_Mode

```c
DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;   // 普通模式
```

**普通模式**：搬完 4 个数据就停。
**循环模式**：搬完 4 个数据自动重新开始。

**本实验用手动触发，用普通模式。**

### 5. DMA_M2M

```c
DMA_InitStructure.DMA_M2M = DMA_M2M_Enable;   // 内存到内存模式
```

**只有 M2M 模式才能做内存到内存的搬运。** 普通 DMA 需要外设触发。

---

## 五、重点

1. **DMA 是自动搬运工**，不占用 CPU，适合大量数据传输。
2. **M2M 模式是内存到内存的搬运**，软件触发，不需要外设。
3. **普通模式搬完一轮就停**，循环模式自动重新开始。
4. **地址自增**：搬完一个数据，地址自动加一。
5. **传输完成检查 TC 标志位**，搬完后必须清除标志位。
6. **每次触发搬运前要重置传输数量**，否则数量为 0，搬不了。
7. **`DataA++` 是为了改变源数据**，方便观察 DMA 搬运前后的对比。

---

## 六、注意事项

1. **普通模式搬完一轮就停**，适合手动触发场景；循环模式会自动重新开始。
2. **每次触发搬运前要重置传输数量**，否则数量为 0，搬不了。
3. **搬运完成后要清除 TC 标志位**，否则下次检查会误判。
4. **`DataA++` 在显示之前执行**，所以显示的是自增后的值，这是教程设计，目的是对比搬运前后。
5. **M2M 模式不需要外设触发**，软件调用 `DMA_Cmd` 就能搬。

---

## 七、经验总结

1. **DMA 适合大量数据传输**，CPU 不参与，效率高。
2. **M2M 模式做内存到内存搬运**，软件触发。
3. **普通模式搬完一轮就停**，适合手动触发场景。
4. **每次触发前重置传输数量**，否则第二次搬不了。
5. **搬运完成后清除 TC 标志位**，否则下次检查会误判。
6. **地址自增**：搬完一个数据地址加一，适合数组搬运。
7. **DMA 是 STM32 里非常强大的外设**，配合 ADC、串口、SPI 能实现自动采集和传输。
