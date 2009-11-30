// vim:fdm=marker:nu:nowrap:encoding=gbk
#ifndef _DT_H_
#define _DT_H_

#include "config.h"
#include "types.h"

// 节点
typedef struct {
	real x;
	real y;
} node;

// +y -x 为先
#define NODE_ORD_CMP(n1, n2) ((n1)->y > (n2)->y || ( (n1)->y == (n2)->y && (n1)->x < (n2)->x ))

typedef void (*edgeHandler)(const node *, const node *);

// main struct
typedef void* myDt;

// dt struct creation and destruction
boolean dt_create(myDt * pdt);
void dt_destroy(myDt * pdt);

/* 以下两组api不可混用, 同时在调用的整个过程, 必须保证所有点的坐标不能被改变*/

void dt_begin(myDt dt, edgeHandler handler);
/* 第一组api
 * dt_next/dt_end 用法
 * dt_begin 初始化dt结构, 表示准备开始一个新的一次运行，之前的数据将会被清除
 * dt_next 逐个输入点集中的每一个点
 * dt_end 运行 dt_next 输入的点集
 */
void dt_next(myDt dt, node * n);
void dt_end(myDt dt);

/* 第二组api
 * 用法基本和第一组一致，唯一不同之处在于使用这一组api必须保证
 * dt_next_sorted 输入的点集必须按照 NODE_ORD_CMP 指定的顺序输入
 * 有些应用场景下点集可能已经以某种形式排好序, 使用
 * 这一组api由于省却排序会比第一组要更高效一些, 同时占用内存也更少
 */
void dt_next_sorted(myDt dt, node * pt);
void dt_end_sorted(myDt dt);

#ifdef COUNT_CALL
// after_break_point count
extern uint32 abp_cnt;
// candidate_circle_event count
extern uint32 cce_cnt;
// handle_site_event count
extern uint32 hse_cnt;
// handle_cirl_event count
extern uint32 hce_cnt;
#endif

#endif
