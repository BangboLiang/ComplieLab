; 注释: .ll文件中注释以';'开头 
source_filename = "if.c"

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"


define dso_local i32 @main() #0{
;float a = 5.555
%1=alloca float
store float 0x40163851E0000000, float* %1
	
;if(a > 1)
%2=load float,float* %1;取出%1的值赋给%2
%3=fcmp ugt float %2,1.000;将其和1进行比较,返回结果到%3中
br i1 %3, label %lebel1, label %lebel2;如果%3为1,跳转到label1,否则跳转到label2

lebel1:	
ret i32 233;return 23

lebel2:
ret i32 0;return 0
}

