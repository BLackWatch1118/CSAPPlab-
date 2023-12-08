/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    //32x32 submission
    /*
    int i, j, ii, jj;
    int a0, a1, a2, a3, a4, a5, a6, a7;
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
                a0 = 1;
                for ( i = ii; i < ii+8; i++)
                {   
                    for ( j = jj; j+a0 < jj+8; j++)
                    {
                        a1 = B[i][j+a0];
                        B[i][j+a0] = B[j+a0][i];
                        B[j+a0][i] = a1;
                    }
                    a0++;
                }
            }                        
            }                       
        }
        */  
    
    //64x64 opti
    /*
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
        */
    //67x61 opti
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


}



/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "ori 32x32 transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, ii, jj;

        for (ii = 0; ii < N; ii +=8) {
            for ( jj = 0; jj < M; jj +=8)
            {
                for ( i = ii; i < ii+8; i++)
                {
                     for ( j = jj; j < jj+8; j++)
                    {
                        B[j][i] = A[i][j];
                    }
                }                                  
            }                       
        }
    }    
//64x64ori optimization
void _64x64trans(int M, int N, int A[N][M], int B[M][N])
{
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
    }    

//67x61 ori optimization
void _67x61trans(int M, int N, int A[N][M], int B[M][N]){
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


}



/*char trans_desc[] = "opti 32x32 transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
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
                //对对角线左边的元素做对称
                c = 1;
                for ( i = ii; i < ii+8; i++)
                {   
                    for ( j = jj; j < jj+8; j++)
                    {
                        t = B[i][j+c];
                        B[i][j+c] = B[j+c][i];
                        B[j+c][i] = t;
                    }
                    c++;
                }
                
                
            }     
            
            
            
                               
            }                       
        }
    
}    
*/

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    //registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

