# exediff
本项目想填补上diff 二进制/ELF 的空缺，开发过程借助kimi

# 应用场景
1. awd
2. 恶意文件分析(也许
3. 简单的二进制文件比较

# 实际功能
1. 分析比对纯二进制文件中的差异，并且像diff一样打印出来
2. 分析比对ELF文件中的差异，将rx段中的汇编代码及剩余二进制部分diff打印出来 (TODO)
3. 根据diff的结果，将patch补丁应用至文件得到patch后的文件

# 屏幕截图

## exediff

## exepatch

# Credit
特别致谢 Kimi
![kimi](https://raw.githubusercontent.com/MoonshotAI/Kimi-Dev/refs/heads/master/assets/moonshot_logo.png)

# License
GNU General Public License v3.0
