#include "cachelab.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
int s, E, b, S, B;
size_t x;//没用的量，保存内存操作需要的字节数
int hits = 0, misses = 0, evictions = 0;
unsigned long addr, target;
unsigned int s_index;
FILE *filename;
//定义链表节点，也就是行
typedef struct _Node
{
    struct _Node* pre;
    unsigned long value;
    struct _Node* next;
} Node;
//定义表头
 typedef struct Double_list
 {
    Node* head;
    Node* tail;
    int size;
 }List;

//读取命令行参数并分解
void get_opt(int argc, char * argv[]);
//分解地址得出t和组索引
void div_addr();
//判断某内存块是否在组中
Node* is_in_sets(unsigned long t, List* list);
//添加新块到链表头节点
void addto_head(unsigned long t, List* list);
//将原链表中的块提为头节点
void to_head(Node* node, List* list);
//删除尾结点
void del_tail(List* list);
//释放链表分配的空间
void free_list(List* sets);
//整体加载流程模拟
void L_simu(List* list, unsigned long t);

int main(int argc, char * argv[])
{
    char ope;
    get_opt(argc, argv);
    //创建并初始化组数组
    S = (int)pow(2,s);
    List* sets = (List*)malloc( S * sizeof(List));
    for (int i = 0; i < S; i++)
    {
        sets[i].head = NULL;
        sets[i].tail = NULL;
        sets[i].size = 0;
    }
    
    while ((fscanf(filename, " %c %lx,%zd\n", &ope, &addr, &x)) != EOF)
    //scanf的格式字符%c前面有个空格，用来跳过I指令开头的空格
    {
        switch (ope)
        {
        case 'S'://case不写break会接着执行到下一个case
        case 'L':
            div_addr();
            L_simu(&sets[s_index], target);
            break;
        case 'M':
            div_addr();
            L_simu(&sets[s_index], target);
            L_simu(&sets[s_index], target);
            break;
        default:
            break;
        }
    }
    fclose(filename);
    free_list(sets);
    printSummary(hits, misses, evictions);
    return 0;
}

void get_opt(int argc, char * argv[]){
    int opt;
    while ( (opt=getopt(argc, argv, "sEbt:hv")) != -1 )
    {
        switch (opt)
        {
        case 's':
        //atoi用来将数字字符转换为int类型数字
            s = atoi(argv[optind]);
            break;
        case 'E':
            E = atoi(argv[optind]);
            break;
        case 'b':
            b = atoi(argv[optind]);
            break;
        case 't':
            //将文件以只读模式打开，返回的文件指针赋给filename
            filename = fopen(argv[optind-1], "r");
            break;
        default:
            break;
        }
    }
}

void div_addr(){
    s_index = (addr >> b) & (int)(pow(2, s)-1);
    target = addr >> (b + s);
}


Node* is_in_sets(unsigned long t, List* list){
    //如果链表没有节点直接跳过
    if ((list->size) == 0)
    {
        return NULL;
    }
    //获取头节点
    Node* node = list->head;
    //遍历链表判断是否有标记值
    while (node != NULL)
    {
        if (node->value == t)
        {
            return node;
        }
        else
        {
            node = node->next;
        }
    }
    return node;
}
void addto_head(unsigned long t, List* list){
    //首先创造新节点，并赋值
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->pre = NULL;
    new_node->value = t;
    /*判断链表中是否已经有结点，有需要设置原头结点的前指针，没有说明链表为空
    需要设置表头的尾指针*/
    if ((list->size) != 0)
    {       
        list->head->pre = new_node;
        new_node->next = list->head;        
        list->head = new_node;
        list->size = list->size + 1;
    }
    else
    {
        new_node->next = NULL;
        list->head = new_node;
        list->tail = new_node;
        list->size = list->size + 1;
    }   
}
void to_head(Node* node, List* list){
    //需要判断该节点是否为头尾节点，不同位置所需操作不同
    if(node->pre == NULL)
   {
      return;
   }
   else if (node->next == NULL)
   {
      node->pre->next = NULL;
      list->tail = node->pre;
      node->pre = NULL;
      node->next = list->head;
      list->head->pre = node;
      list->head = node;
   }
   //不是头尾需要重新设置头指针，并且将节点上下重新连接
   else
   {
      node->next->pre = node->pre;
      node->pre->next = node->next;
      node->next = list->head;
      list->head->pre = node;
      list->head = node;
      node->pre = NULL;
   }
}
void del_tail(List* list){
    Node* tmp = list->tail;
    //判断是不是只有一个节点
    if (list->size == 1)
    {
        free(tmp);
        list->head = NULL;
        list->tail = NULL;
        list->size = list->size - 1;
    }
    else
    {
        list->tail = tmp->pre;
        //注意应该释放malloc分配的空间
        tmp->pre->next = NULL;
        free(tmp);
        tmp = NULL;
        list->size = list->size - 1;
    }
    }
void free_list(List* sets){
    //遍历数组中的每一组
    for (int i = 0; i < S; i++)
    {
        //对每一组中的链表节点分别释放空间
        while (sets[i].size)
    {
        Node* tmp = sets[i].head;
        sets[i].head = sets[i].head->next;
        free(tmp);
        tmp = NULL;
        sets[i].size--;
    }
    } 
}
void L_simu(List* list, unsigned long t){
    Node* cur_node = is_in_sets(target, list);
    if (cur_node != NULL)
    {
        hits++;
        to_head(cur_node, list);
    }
    else
    {
        if ((list->size) == E)
        {
            evictions++;
            del_tail(list);
        }
        misses++;
        addto_head(target, list);
    }

}

