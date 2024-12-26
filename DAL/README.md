# 设备抽象层 (Device Abstraction Layer)

设备抽象层（dal，Device Abstraction Layer）是一个用于封装底层设备访问的接口层。它为应用层提供了一组统一的设备操作接口，使得应用层可以直接调用相关的读写等操作，而不需要关心底层设备的具体实现。

## 接口概述

设备抽象层提供了以下主要功能：

- **设备打开和关闭**：允许应用程序打开和关闭设备。
- **设备读写**：提供读取和写入设备数据的接口。
- **文件偏移**：支持文件指针的定位和调整。适用于FLASH等设备

## 接口函数

### 错误码说明

以下是此文件定义的错误码：

- `-6`：设备不存在。
- `-5`：设备被占用。
- `-4`：操作异常（例如对只读设备执行写操作）。
- `-3`：设备不可使用（例如设备未打开）。
- `-2`：打开设备过多。
- `-1`：无效参数。
- `0`：无错误。

### `int dal_open(const char *dev_name);`

打开指定的设备，并返回设备文件描述符 

**参数：**

- `dev_name`：设备名称的字符串。

**返回值：**

- 文件描述符有效值为3-63，0-2为框架保留值,大于等于64为无效值。
- 失败时返回错误码（<0）

### `int dal_close(int fd);`

关闭指定的设备文件描述符。

**参数：**

- `fd`：要关闭的文件描述符。

**返回值：**

- 成功时返回 0。
- 失败时返回错误码（<0），参考错误码说明。

### `int dal_read(int fd, uint8_t *buf, size_t len);`

从设备中读取数据。

**参数：**

- `fd`：设备文件描述符。
- `buf`：存储读取数据的缓冲区指针。
- `len`：要读取的数据长度。

**返回值：**

- 成功时返回读取的数据长度（>=0）。
- 失败时返回错误码（<0），参考错误码说明。

### `int dal_write(int fd, const uint8_t *buf, size_t len);`

向设备中写入数据。

**参数：**

- `fd`：设备文件描述符。
- `buf`：要写入的数据缓冲区指针。
- `len`：要写入的数据长度。

**返回值：**

- 成功时返回写入的数据长度（>=0）。
- 失败时返回错误码（<0），参考错误码说明。

### `int dal_ioctrl(uint8_t fd, int cmd, void *arg);`

从设备中读取数据。

**参数：**

- `fd`：设备文件描述符。
- `cmd`：指令码 由驱动程序定义。
- `arg`：通用参数指针，由驱动程序定义。

**返回值：**

- 成功时返回由驱动程序定义的值。
- 失败时返回错误码（<0），参考错误码说明。

### `int dal_lseek(int fd, int offset, enum dal_lseek_whence whence);`

设置文件偏移量。

**参数：**

- `fd`：设备文件描述符。
- `offset`：相对于 `whence` 的偏移量，可以为正负值。
- `whence`：偏移的基准位置，值可以为：
  - `DAL_LSEEK_WHENCE_HEAD`：文件的起始位置。
  - `DAL_LSEEK_WHENCE_SET`：当前文件指针的位置。
  - `DAL_LSEEK_WHENCE_TAIL`：文件的末尾位置。

**返回值：**

- 成功时返回 0。
- 失败时返回错误码（<0），参考错误码说明。

## 许可证

The MIT License (MIT)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.