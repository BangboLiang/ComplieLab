; 注释: .ll文件中注释以';'开头 
source_filename = "if.c"

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"


define dso_local i32 @main() #0{
%1=alloca i32;int a
%2=alloca i32;int i
store i32 10,i32* %1;a=10
store i32 0,i32* %2;i=0
br label %label1;跳转至判断语句label1

label1:
%3=load i32,i32* %2;将i的值赋给%3
%4 = icmp slt i32 %3, 10;判断i<10的真假，并将结果返回给%4
br i1 %4,label %label2,label %label3;为真跳转label2，否则跳转label3

label2:
%5=add i32 %3 , 1;i=i+1
store i32 %5,i32* %2
%6=load i32,i32* %1;a=a+i
%7=add i32 %6,%5
store i32 %7,i32* %1
br label %label1;跳回label1

label3:
%8=load i32,i32* %1;return a
ret i32 %8
}

