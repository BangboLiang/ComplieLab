#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"

#include <iostream>
#include <memory>

#ifdef DEBUG  // 用于调试信息,大家可以在编译过程中通过" -DDEBUG"来开启这一选项
#define DEBUG_OUTPUT std::cout << __LINE__ << std::endl;  // 输出行号的简单示例
#else
#define DEBUG_OUTPUT
#endif

#define CONST_INT(num) \
    ConstantInt::get(num, module)

#define CONST_FP(num) \
    ConstantFP::get(num, module) // 得到常数值的表示,方便后面多次用到
    
int main(){
	auto module = new Module("Cminus code");  // module name是什么无关紧要
	auto builder = new IRBuilder(nullptr, module);      // 创建IRBuilder,该类提供了独立的接口创建各种 IR 指令
	Type* Int32Type = Type::get_int32_type(module);

	// main函数;
	auto mainFun = Function::create(FunctionType::get(Int32Type, {}),"main", module);   /* 创建 main 函数 */	
	auto bb = BasicBlock::create(module, "entry", mainFun); 
	builder->set_insert_point(bb);  
    
	auto retAlloca = builder->create_alloca(Int32Type);  //分配一个保存返回值的内容
    
	auto *arrayType = ArrayType::get(Int32Type, 10);  //数组类型，参数依次是数组元素的类型Int32Type,数组元素个数10
	auto a=builder->create_alloca(arrayType);	//数组a[10]

	auto a0GEP = builder->create_gep(a, {CONST_INT(0), CONST_INT(0)});	//获取a[0]的地址
	builder->create_store(CONST_INT(10), a0GEP);	//将整数10写入a[0]的地址
	auto a0Load = builder->create_load(a0GEP);	//读取a[0]
	auto m=builder->create_imul(a0Load,CONST_INT(2));	//a[0]*2

	auto a1GEP = builder->create_gep(a, {CONST_INT(0), CONST_INT(1)});	//获取a[1]的地址
	builder->create_store(m, a1GEP);	//将a[0]*2写入a[1]的地址
	auto a1Load = builder->create_load(a1GEP);	//获取a[1]的值

	builder->create_store(a1Load, retAlloca);	//将a[1]的值写入retAlloca地址中
	auto retLoad=builder->create_load(retAlloca);	//读取retAlloca地址的值
	builder->create_ret(retLoad);	//创建ret指令，即返回

	std::cout << module->print();
 	delete module;
	return 0;   
}