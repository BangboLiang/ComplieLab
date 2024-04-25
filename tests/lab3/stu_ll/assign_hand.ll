; 注释: .ll文件中注释以';'开头
; ModuleID = 'assign.c'                                
source_filename = "assign.c"  
; 注释: target的开始
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
; 注释: target的结束

; 	注释: 全局main函数的定义
; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
; 	注释: 第一个基本块的开始
%1 =  alloca [10 x i32]
;	参考手册，用alloca为a[10]分配内存
%2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 0
;	getelementptr指令用于获取数组结构的元素的地址
store i32 10, i32* %2 
;	利用store，将10赋值给a[0]



;	下面几行代码对应a[1] = a[0] * 2;
%3 = load i32, i32* %2	
;	取出a[0],存到%3中,之后*2并存储到%4中
%4 = mul i32 %3, 2
%5 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 1	
;	获取a[1]指针
store i32 %4, i32* %5	
;	将%4存储的值写入a[1]中


;	最后对应return a[1]
ret i32 %4				
;	返回a[1],


}

