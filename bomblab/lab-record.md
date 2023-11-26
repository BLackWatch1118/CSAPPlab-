# Bomblab实验记录

***

## 实验环境

> apt policy {name} 和 lsb_release 获取到版本信息

- ubuntu-22.04.3-desktop-amd64.isoVMvituralbox 上运行的Ubuntu系统，版本Ubuntu 22.04.3 LTS Desktop版，安装包名ubuntu-22.04.3-desktop-amd64.iso

- gcc版本 4:11.2.0-1ubuntu1
* gdb版本12.1-0ubuntu1~22.04.1

***

## GDB常用命令

```
r                    #运行程序
b {函数名/行数}       #打断点
stepi                #单步汇编指令执行
finish               #执行完当前函数并在跳出后暂停执行，提供返回值
quit                 #退出gdb
d {num}               #删除指定编号断点
disas {fun/add}       #反汇编某函数、地址
layout asm            #在终端实时显示汇编指令执行
ctrl+x+a  ctrl+l      #退出layout 刷新显示
p (int*)($rax+0x8)    #以int打印根据%rax寄存器储存地址偏移0x8处储存的值
x/d x/c x/s + {addr/$寄存器}   #以十进制整数、字符、字符串打印地址出的值
```

***

**另附发现的git分支切换问题，当分支修改并未整合到master支中时不能直接切换到主支，解决方法应先储藏分支，切换至主支行动完成后再弹回分支**

```
git stash
git switch master
''''
git switch dev
git stash pop
```

***

## 正式实验：phase_1

首先反汇编bomb文件得到bomb.s备用

> objdump -d bomb > bomb.s

先看汇编代码中的main函数，可以看到

```asmatmel
0000000000400da0 <main>:
""""
  400e2d:	e8 de fc ff ff       	call   400b10 <puts@plt>
  400e32:	e8 67 06 00 00       	call   40149e <read_line>
  400e37:	48 89 c7             	mov    %rax,%rdi
  400e3a:	e8 a1 00 00 00       	call   400ee0 <phase_1>
  400e3f:	e8 80 07 00 00       	call   4015c4 <phase_defused>
  400e44:	bf a8 23 40 00       	mov    $0x4023a8,%edi
  400e49:	e8 c2 fc ff ff       	call   400b10 <puts@plt>
  400e4e:	e8 4b 06 00 00       	call   40149e <read_line>
  400e53:	48 89 c7             	mov    %rax,%rdi
  400e56:	e8 a1 00 00 00       	call   400efc <phase_2>
  400e5b:	e8 64 07 00 00       	call   4015c4 <phase_defused>
  400e60:	bf ed 22 40 00       	mov    $0x4022ed,%edi
```

可以看到在call phase_1函数前输入是通过read_line函数获取的，返回值%rax给到%rdi，初步判断输入是通过寄存器%rdi传递的，如果符合要求就跳转到phase_defused后进入下一关。

先用gdb调试bomb程序，在phase_1处打断点，然后反汇编，得到结果

```
(gdb) b phase_1
(gdb) r
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
sdsd
Breakpoint 1, 0x0000000000400ee0 in phase_1 ()
(gdb) disas
Dump of assembler code for function phase_1:
=> 0x0000000000400ee0 <+0>:     sub    $0x8,%rsp
   0x0000000000400ee4 <+4>:     mov    $0x402400,%esi
   0x0000000000400ee9 <+9>:     call   0x401338 <strings_not_equal>
   0x0000000000400eee <+14>:    test   %eax,%eax
   0x0000000000400ef0 <+16>:    je     0x400ef7 <phase_1+23>
   0x0000000000400ef2 <+18>:    call   0x40143a <explode_bomb>
   0x0000000000400ef7 <+23>:    add    $0x8,%rsp
   0x0000000000400efb <+27>:    ret    
End of assembler dump.
```

可以看到函数1首先将一个立即数放到%esi里，之后call函数<strings_not_equal>，从名称可以推断是判断输入字符与规定字符串是否相等，相等会返回0，则推测立即数$0x402400所在存放的是要求字符串。输入指令

```
(gdb) x/s $esi
0x402400:       "Border relations with Canada have never been better."
```

则可以得到第一关的答案。

可以将答案统一写到一个文件中ans.txt,避免多次输入答案，用如下命令运行：

> ./bomb < ans.txt

***

## phase_2
