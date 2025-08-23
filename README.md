# exediff
本项目想填补上diff 二进制/ELF 的空缺，开发过程借助kimi

## 应用场景
1. awd
2. 恶意文件分析(也许
3. 简单的二进制文件比较

## 实际功能
1. 分析比对纯二进制文件中的差异，并且像diff一样打印出来
2. 分析比对ELF文件中的差异，将rx段中的汇编代码及剩余二进制部分diff打印出来 (TODO)
3. 根据diff的结果，将patch补丁应用至文件得到patch后的文件

## 安装

首先需要在系统中存在`libelf`(libelf1t64+libelf-dev), `libcapstone`(libcapstone5+libcapstone-dev) 和 `libkeystone`(MANULLY)，然后可以执行以下语句：

```sh
git clone https://github.com/RocketMaDev/exediff.git
cd exediff
cmake . -DDEBUG=1
make -j$(nproc)
```

然后`exediff/`下会存在`exediff`和`exepatch`两个binary

## 屏幕截图

### exediff
<img width="1072" height="823" alt="exediff" src="https://github.com/user-attachments/assets/2cb19edd-2968-4815-ab21-7b22b8a0cb8c" />

### exepatch
<img width="1636" height="243" alt="exepatch" src="https://github.com/user-attachments/assets/cbfeda14-d26e-4add-8c76-f84642bcbb89" />

## Credit
特别致谢 Kimi
![kimi](https://raw.githubusercontent.com/MoonshotAI/Kimi-Dev/refs/heads/master/assets/moonshot_logo.png)

## License
GNU General Public License v3.0
