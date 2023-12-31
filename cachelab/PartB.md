# PARTB

## 实验目标：

* 优化矩阵的转置函数，使对于不同大小的矩阵能尽可能减少缓存丢失。矩阵大小分别为32x32, 64x64, 61x67。

* 每个转置函数最多定义12个int类型的变量。不允许用long类型和用位操作将多个变量存在一个变量中。辅助函数不能突破12个变量的限制。

* 不能用递归。

* 不能修改原数组。

* 模拟的缓存参数为(s = 5, E = 1, b = 5）。

* 需要将答案转置函数写到transpose_submit下注册

***

分析参数s = 5, E = 1, b = 5，说明模拟高速缓存使用直接映射高速缓存方式，每组只有一行，未命中会被直接替换。总组数为2^5 = 32组，块大小为2^5 = 32字节。

矩阵在转置时进行分块后，转置方式是先将每一块内的矩阵转置，在将块整体进行转置，可能有用。

### 因为markdown插图片和表格费劲，所以所有的分析都是文字，最好找张纸画图看看

***

### 1. 32 x 32

先看最原始的示范转置函数

```c
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp }}}
```

##### 分析原始的函数

可以看到矩阵是存储在2维数组中的，每个元素都是int类型占4字节，矩阵是按行的顺序储存在数组中的。此函数对于矩阵A是按先行后列访问，是步长为1的访问模式，对于B是先列后行的访问，是步长为n的访问模式。通过对此函数的测试，可以得到访问情况为“hits:869, misses:1184, evictions:1152”，现在分析是怎么得到这些数据的。

高速缓存只有32行，每行能存储8个元素，放不下一个矩阵。由于b有5位，所以每当地址增加32时会向组索引位置进位，就是每32字节会被分配到一个组中。则在2维数组中每8个元素会分配到同一个组中。所以每个矩阵第一行会被均分到前4组，高速缓存共32组，可以储存8行，每个高速缓存最多只能储存1/4个矩阵。

现在分析转置函数，对于A[0][0]的访问会将A矩阵1行的前8个元素加载到缓存第0组，造成一次冷不命中，然后对B[0][0]的访问同样会将B矩阵1行的前8个元素加载到第0组，造成一次不命中和一次驱逐。再对A[0][1]的访问会再将A矩阵1行的前8个元素加载到缓存第0组，造成一次不命中和驱逐，对B[1][0]的访问加载B矩阵1行的前8个元素加载到第1组，造成一次不命中。接下来直到A[0][7]的访问都会命中，对A[0][8]的访问会到加载第1组，对B[8][0]的仍然会映射到第1组，A[0][9]的访问会到第1组未命中，但对B[9][0]的访问加载第1组的数据，直到A[0][10]再加载数据到第1组，B[10][0]的访问会到第2组，错开对A访问的冲突，如此进行循环。

总结对于A的行访问每次都会坚持对于从组索引为0开始的每一组进行8次访问，再对组索引递增1，进行4次。对B的列访问于此同步对从0开始间隔组索引为4进行8次访问，索引再次归零，重复4次。（举例第一次访问每8个元素的访问对A来说映射的组是1,1,1,1,1,1,1,1， 对B来说是1,5,9,13,17,21,25,29）

假设A和B有单独的访问缓存，不会冲突，A只会有每块的冷不命中，共32x32/8 = 128次。B由于在每列前8次访问会按组索引增加4的模式逐一访问缓存会全部miss，因为缓存大小不足，有重复映射现象，所以下8次访问虽然组索引数与前8次相同，但数据块的标记不同，所以所有访问全部miss，共32x32 = 1024次。同时对于A每行访问也会将组数递增4，所以每行在访问时一定会与B的访问发生一次冲突，共计32次，所以函数总miss数是1184次。

***

##### 初步优化

由以上分析可以看出矩阵最主要的冲突还是在因为大小不足对B访问模式上的冲突，应该想办法消减。按照实验文档提到的分块思想将矩阵分成8x8的小块，共分为16块（设编号从左上开始为1，向右顺序增加）。可以确保矩阵对称，同时每块矩阵的行正好可以储存在同一组中，分块后矩阵每行正好可以完整储存在缓存中。根据矩阵转置的性质，分块后转置相当于以对角线为轴做了一次翻转，并且每块分别进行转置，先抛出对角线上的块，对以外的块先进行处理。可以看做优化8x8矩阵的转置函数，注意任意相对对角线对称的两块在高速缓存上组的映射是有差别的（举例对于A里面的2号块，转置之后的位置是5号块，2号块每行对应的组数为1,5,9,13……29；5号块每行对应的组数为0,4,8,12……28。因为转置之后行列的标号会互换，它们相对于矩阵每行分出的4块中的位置一定不同，所以组映射一定不同），所以对于各自的访问不会造成冲突的情况，缓存大小足够，不用优化。可以直接进行转置操作。先来写一个初步优化的代码。

```c
    int i, j, ii, jj, tmp;
        for (ii = 0; ii < N; ii +=8) {
            for ( jj = 0; jj < M; jj +=8)
            {
                for ( i = ii; i < ii+8; i++)
                {
                     for ( j = jj; j < jj+8; j++)
                    {
                        tmp = A[i][j];
                        B[j][i] = tmp;
                    } } } }
```

misses:344，在对角线以外的块中已经达成了每次访问只有一次冷不命中的最好情况，共8x2x12 = 192次，则在对角线8块的转置中总共消耗了152次，平均每块19次未命中，抛去8次冷不命中，还有11次，说明对角线上造成11次冲突。除开每次行列转换中会有一次的组索引的重复占了8次，还有3次，理论上来说这3次冲突是不该存在的，不知道为什么会这样。现在考虑怎么优化对角线的转置函数。最多12个变量，之前已经用过4个(tmp可以删除)，还能用8个变量，正可以储存整行的数据。最优解应该只有冷不命中，不应该出现组的冲突。

##### 优化对角线的块

可以这样考虑，对角线上的元素不需要转换，A每行只会读取一次，所以在B使用A读取过的缓存行时不会再被覆盖，对角线元素左边的元素需要沿对角线向上翻转，说明需要写到之前A已经用过的缓存行上，这样并不会被再次覆盖而其冲突；对角线元素右边的元素沿对角线向左下翻转，会占用将来A会用到的内存行，所以优化的方向可以是在读取到对应行的元素前不在B矩阵的对应行写入数据。所以可以在读取元素时不直接存到B中，而是存在变量中，等一行数据完整读完的时候再写入B。在A[0]读取完成不在需要此内存行时，将读取的元素写在B[0]中；接下来读取A[1]，将所有A[1]数存到变量中，将所有变量写在B[1]中，对对角线元素左边的所有数做对称，以此类推。***<u>注意：当矩阵能够完全加载到缓存中，矩阵每一行都能对应缓存的不同一行，这样对矩阵操作可以一直命中，所以以上操作实际上就是将矩阵A的元素直接复制到B里，然后在B里直接进行元素关于对角线的替换对称。</u>*** 代码如下(记错数量限制了，把临时变量c和t换成用不着的a0-a7的任意就行)

```c
    int i, j, ii, jj;
    int a0, a1, a2, a3, a4, a5, a6, a7, t;
    int c;
    for (ii = 0; ii < N; ii +=8) {
        for ( jj = 0; jj < M; jj +=8)
        {
            if (ii != jj)
            {
                for ( i = ii; i < ii+8; i++){
                    for ( j = jj; j < jj+8; j++)
                    {
                        B[j][i] = A[i][j]; }}}
            else
            {
                //保存A第一行到变量，然后写到B的第一行
                a0 = A[ii][jj]; a1 = A[ii][jj+1]; a2 = A[ii][jj+2];  a3 = A[ii][jj+3];
                a4 = A[ii][jj+4]; a5 = A[ii][jj+5];  a6 = A[ii][jj+6]; a7 = A[ii][jj+7];
                B[ii][jj] = a0; B[ii][jj+1] = a1; B[ii][jj+2] = a2; B[ii][jj+3] = a3;
                B[ii][jj+4] = a4; B[ii][jj+5] = a5; B[ii][jj+6] = a6; B[ii][jj+7] = a7;
                //读取A的第二行保存到变量，写到B的第二行
                a0 = A[ii+1][jj]; a1 = A[ii+1][jj+1]; a2 = A[ii+1][jj+2];  a3 = A[ii+1][jj+3];
                a4 = A[ii+1][jj+4]; a5 = A[ii+1][jj+5];  a6 = A[ii+1][jj+6]; a7 = A[ii+1][jj+7];
                B[ii+1][jj] = a0; B[ii+1][jj+1] = a1; B[ii+1][jj+2] = a2; B[ii+1][jj+3] = a3;
                B[ii+1][jj+4] = a4; B[ii+1][jj+5] = a5; B[ii+1][jj+6] = a6; B[ii+1][jj+7] = a7;
                //3
                a0 = A[ii+2][jj]; a1 = A[ii+2][jj+1]; a2 = A[ii+2][jj+2];  a3 = A[ii+2][jj+3];
                a4 = A[ii+2][jj+4]; a5 = A[ii+2][jj+5];  a6 = A[ii+2][jj+6]; a7 = A[ii+2][jj+7];
                B[ii+2][jj] = a0; B[ii+2][jj+1] = a1; B[ii+2][jj+2] = a2; B[ii+2][jj+3] = a3;
                B[ii+2][jj+4] = a4; B[ii+2][jj+5] = a5; B[ii+2][jj+6] = a6; B[ii+2][jj+7] = a7;
                //4
                a0 = A[ii+3][jj]; a1 = A[ii+3][jj+1]; a2 = A[ii+3][jj+2];  a3 = A[ii+3][jj+3];
                a4 = A[ii+3][jj+4]; a5 = A[ii+3][jj+5];  a6 = A[ii+3][jj+6]; a7 = A[ii+3][jj+7];
                B[ii+3][jj] = a0; B[ii+3][jj+1] = a1; B[ii+3][jj+2] = a2; B[ii+3][jj+3] = a3;
                B[ii+3][jj+4] = a4; B[ii+3][jj+5] = a5; B[ii+3][jj+6] = a6; B[ii+3][jj+7] = a7;
                //5
                a0 = A[ii+4][jj]; a1 = A[ii+4][jj+1]; a2 = A[ii+4][jj+2];  a3 = A[ii+4][jj+3];
                a4 = A[ii+4][jj+4]; a5 = A[ii+4][jj+5];  a6 = A[ii+4][jj+6]; a7 = A[ii+4][jj+7];
                B[ii+4][jj] = a0; B[ii+4][jj+1] = a1; B[ii+4][jj+2] = a2; B[ii+4][jj+3] = a3;
                B[ii+4][jj+4] = a4; B[ii+4][jj+5] = a5; B[ii+4][jj+6] = a6; B[ii+4][jj+7] = a7;
                //6
                a0 = A[ii+5][jj]; a1 = A[ii+5][jj+1]; a2 = A[ii+5][jj+2];  a3 = A[ii+5][jj+3];
                a4 = A[ii+5][jj+4]; a5 = A[ii+5][jj+5];  a6 = A[ii+5][jj+6]; a7 = A[ii+5][jj+7];
                B[ii+5][jj] = a0; B[ii+5][jj+1] = a1; B[ii+5][jj+2] = a2; B[ii+5][jj+3] = a3;
                B[ii+5][jj+4] = a4; B[ii+5][jj+5] = a5; B[ii+5][jj+6] = a6; B[ii+5][jj+7] = a7;
                //7
                a0 = A[ii+6][jj]; a1 = A[ii+6][jj+1]; a2 = A[ii+6][jj+2];  a3 = A[ii+6][jj+3];
                a4 = A[ii+6][jj+4]; a5 = A[ii+6][jj+5];  a6 = A[ii+6][jj+6]; a7 = A[ii+6][jj+7];
                B[ii+6][jj] = a0; B[ii+6][jj+1] = a1; B[ii+6][jj+2] = a2; B[ii+6][jj+3] = a3;
                B[ii+6][jj+4] = a4; B[ii+6][jj+5] = a5; B[ii+6][jj+6] = a6; B[ii+6][jj+7] = a7;
                //8
                a0 = A[ii+7][jj]; a1 = A[ii+7][jj+1]; a2 = A[ii+7][jj+2];  a3 = A[ii+7][jj+3];
                a4 = A[ii+7][jj+4]; a5 = A[ii+7][jj+5];  a6 = A[ii+7][jj+6]; a7 = A[ii+7][jj+7];
                B[ii+7][jj] = a0; B[ii+7][jj+1] = a1; B[ii+7][jj+2] = a2; B[ii+7][jj+3] = a3;
                B[ii+7][jj+4] = a4; B[ii+7][jj+5] = a5; B[ii+7][jj+6] = a6; B[ii+7][jj+7] = a7;
                //对对角线两边的元素做对称
                c = 1;
                for ( i = ii; i < ii+8; i++)
                {   
                    for ( j = jj; j+c < jj+8; j++)
                    {
                        t = B[i][j+c];
                        B[i][j+c] = B[j+c][i];
                        B[j+c][i] = t;
                    }
                    c++;                }            }                         }                        }
```

结果为：func 0 (Transpose submission): hits:2241, misses:260, evictions:228，已经接近理论极限256次。多了4次不知道在哪。

***

### 2. 64x64

对于64x64的矩阵，每行会分到8个组里，前4行会被分配到32个缓存行里，1/16的矩阵能被存在缓存里。

##### 8x8分块的分析

再考虑将矩阵分块，先试着分成8x8的块(依次编号为1-64)，整个矩阵会被分为8x8的大块，先看关于对角线块对称的块，因为每4行会映射到32组，分块时每块有8行，所以每块的前4行和后4行都会映射到同样的组中，如果像32x32仍然直接对于对称块进行转置，虽然对块行的访问和列的访问不会映射到同样的高速缓存行中，但是与原始转置32x32的情况类似，对行的前四个元素的访问会命中，但对应的列访问都会miss，然后访问行的后4个元素时由于列将要访问的块标记值与已经存在的不同，仍然会miss，总结下来和没优化区别不大。

* 所以我们可以发现，在8x8分块中，根据上一题最后的分析，当块可以完全装载缓存中，复制到B对应处直接进行相应的转置可以达到最优解。对于64x64的矩阵来说，每4行会分配一组缓存映射，所以对于4x4的块可以完全装载到缓存中，可以达到不冲突。

##### 对8x8矩阵进行4x4的处理

直接对矩阵分成4x4的块会造成浪费，因为缓存每行能有8个元素，每行会至少多访问一次。所以尝试对8x8的矩阵分成4x4的小块，运用分块矩阵转置的性质解决。

以关于对角线对称的两组为例。将4x4的块直接复制然后进行转置，但是对A的块每行可以读取到8个数，应该考虑怎么处理读取的后4个数。因为对B写入时每个缓存行可以写入8个数，可以将读取A的后4个数字写在相应位置的后面，并且按照转置好的位置写入，此时的状态为已经完全读取了A的前4行信息存储到B的前4行中且转置状态是正确的，B前4行对应的缓存行出于命中的状态，这些缓存行的后4位应该储存在B的后4行的前半部分，与当前组数相同但数据冲突，因此对于B后4行前半部分的写入会覆盖前4行，所以可以逐行读取到变量中再写入，并且因为A组后4行前半部分应该写入B前4行后半部分(对这样的分块A和B的访问会映射到不同的内存行中不会造冲突)，可以直接写入A中行的数据，不会造成浪费。完成后缓存行里存储的是B后4行的信息，然后再写入A组后4行后半的信息。对于每块都进行这样的操作，没有对对角线块进行优化，所以会造成一定浪费。

<mark>总结来简单说就是将8x8的代码块再细分为4x4的块，根据矩阵的性质分别对每块转置再将块</mark>

<mark>位置转置，需要考虑的是操作顺序的问题，以减少浪费。先把A块中的前4行的两小块复制到</mark>

<mark>B块的前4行上并完成转置。再将B块前4行的后半部分逐行复制到变量中，将A后四行的每一</mark>

<mark>列元素部分读取出来，覆盖到B前4行后半部分的每一行，然后将变量中的值写入B后4行的前</mark>

<mark>半部分。最后将A块后4行的后半部分复制到B块后4行的后半部分</mark>。

代码如下：

```c
int i, j, ii, jj;
    int a0, a1, a2, a3, a4, a5, a6, a7;

        for (ii = 0; ii < N; ii +=8) {
            for ( jj = 0; jj < M; jj +=8)
            {
                for ( i = ii, j = jj ; i < ii+4; i++)
                {
                    //将A的前4行给到B的前4行同时进行转置
                    a0 = A[i][j],   a1 = A[i][j+1], a2 = A[i][j+2], a3 = A[i][j+3];
                    a4 = A[i][j+4], a5 = A[i][j+5], a6 = A[i][j+6], a7 = A[i][j+7];
                    B[j][i] = a0,   B[j+1][i] = a1,   B[j+2][i] = a2,   B[j+3][i] = a3;
                    B[j][i+4] = a4, B[j+1][i+4] = a5, B[j+2][i+4] = a6, B[j+3][i+4] = a7;
                }
                for ( i = ii, j = jj; j < jj+4; j++)
                {   
                    //保存B前四行的后半部分到变量
                    a0 = B[j][i+4]; a1 = B[j][i+5], a2 = B[j][i+6], a3 = B[j][i+7];
                    //在B的前四行后半部分写入A后4行的前半部分
                    B[j][i+4] = A[i+4][j], B[j][i+5] = A[i+5][j], B[j][i+6] = A[i+6][j], B[j][i+7] = A[i+7][j];
                    //在B的后4行前半部分写入变量
                    B[j+4][i] = a0, B[j+4][i+1] = a1, B[j+4][i+2] = a2, B[j+4][i+3] = a3;
                }

                for ( i = ii+4, j = jj+4; i < ii+8; i++)
                {
                    //保存A后4行的后半部分转置到B的后4行后半部分
                    a0 = A[i][j], a1 = A[i][j+1], a2 = A[i][j+2], a3 = A[i][j+3];
                    B[j][i] = a0, B[j+1][i] = a1, B[j+2][i] = a2, B[j+3][i] = a3; 
                }               
            }                       
        }
```

最后结果为hits:9017, misses:1228, evictions:1196。算是满分。对于以上优化方法可以预见到在对角线块上会出现额外的冲突，参考上一个的思路应该也可以优化。

***

### 61x67 M = 61, N = 67  int A[N][M]

最后一个矩阵行和列不对称，难以像前两个一样分成对称的块，并且第一行的最后5个元素会和第二行的前3个元素放在一个组里，每行分配到的组数是不对齐的，增加了优化难度（<mark>或者说一定程度减少了优化难度</mark>）。

##### 总结对于之前矩阵的简要优化流程

依照之前的分析，因为每8个数会分配到同一缓存行，总共32缓存行，当行元素时8的倍数时，可以将行按`每8个元素分成n组`，注意`n若能被32整除`才能能用完整的行填满所有的高速缓存行，假设`32/n = c`，所以使得每隔c行矩阵会循环映射到相同的缓存行。专注于对这种特殊情况的考虑，此时当分块的块分的太大（指`超过了c`），在读取每块的行元素同步对列进行写入，一定会造成组映射上的冲突，会产生大量的浪费。

比如32x32的矩阵因为前8行的所有元素会被完全映射到缓存中，所以分组大小不能超过8,64x64的前4行能完全映射到缓存中，所以分组不能超过4。对此我们可以简略总结出对于此类矩阵针对`特定高速缓存`的简单优化策略：`当矩阵对称，假设矩阵大小为M，M能被8整除，设x = M/8，n = 32/x，则应当对矩阵进行nxn的分块.`

##### 对本题基础优化

对于本题这种行元素和列元素不同并且都不是8的倍数，可以知道其对于每隔几行会循环映射到不同的缓存行，那么基础的优化思路就是，先考虑将尽可能多的元素加载到缓存中，减少对角线上元素的冲突，缓存中可以存储的最大元素数量为256个，此时对于分块大小的限制在于B，应该考虑B经过几个循环后会与再与首次的映射的分组接近。由于B每行67个元素，缓存可以存3行+55个元素，第4行还有67-55=12个元素，能分1组后多4个元素，则这4个元素与第5行的前4个元素会分配到第二组。写个函数计算经过几次映射后行首元素会再次映射到第一组。

```c
    int line, a, sets;
    for (int i = 1; i < 20; i++)
    {
        sets = ((67 - (55 * i) % 67) / 8) + 1;
        a = 8- ((67 - (55 * i) % 67) % 8);
        line = 256 * i / 67 + 1;
        printf("loop %d, sets = %d, left %d, in line %d\n",i, sets, a, line);
    }
```

输出如下：

> loop 1, sets = 2, left 4, in line 4
> loop 2, sets = 4, left 8, in line 8
> loop 3, sets = 5, left 4, in line 12
> loop 4, sets = 7, left 8, in line 16
> loop 5, sets = 8, left 4, in line 20
> loop 6, sets = 1, left 3, in line 23

所以可以看到在第20行之后再进行一次映射就会再次回到第一组，造成冲突，在23行时已经再映射到第1组，所以分块的大小应该小于23，本着分块越大对缓存利用效率越高的原则，从23开始向低开始一个个测试分块。

```c
int i, j, ii, jj;
    int size = 23;
        for (ii = 0; ii < N; ii +=size) {
            for ( jj = 0; jj < M; jj +=size)
            {
                for ( i = ii; i < ii+size && i< N; i++)
                {
                     for ( j = jj; j < jj+size && j < M; j++)
                    {
                        B[j][i] = A[i][j];
                    }
                }                                  
            }                       
        }
```

hits:6250, misses:1929, evictions:1897，可以达到满分，不用试了。
