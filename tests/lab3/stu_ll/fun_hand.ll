; 注释: .ll文件中注释以';'开头                             
source_filename = "fun.c"  

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @callee(i32) #0 {
;int callee(a)

%2= mul i32 %0,2
;2*a
ret i32 %2
;return ...

}

define dso_local i32 @main() #0 {
;int main()

%1= call i32 @callee(i32 110)
;通过call调用callee函数，并存储到%1中
ret i32 %1
;return ...

}
