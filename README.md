# exediff
Compare binaries with DIFF like format

rx.c: get_rx (char *elf_path)
disasm.c: disassemble (char *to_disasm)

```
--- FILE
+++ FILE
@@ -0x1234,3 +4567,3 @@
 a3
-c3
+90
 00
!! -0x404030,3 +0x404030,3 !!
 5d                 # pop rbp
-c3                 # ret
+90                 # nop
 90                 # nop
 90                 # nop
# 选择下面这种方案，当patch长度长于原始长度时，错误；当patch长度短于原始长度，用对应架构的nop填充
!! -0x404030,3 +0x404030,3 !!
 pop rbp
-ret
+nop
 00
# 更高级的特性
!! -0x4041f0,4 +0x4041f0,4 !!
 lea rdi, [rsp - 0x30]
-add rsp, 8
-jmp free@PLT
+{mov [rsp - 0x30], 0
  call free@plt
  ret}
 int3
```
