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

	//callee函数
	std::vector<Type *> Ints(1, Int32Type);	// 函数参数类型为int
	auto calleeFunTy = FunctionType::get(Int32Type, Ints); 	//通过返回值类型与参数类型列表得到函数类型
	auto calleeFun = Function::create(calleeFunTy,"callee", module);	// 由函数类型得到函数
	
	// BB的名字在生成中无所谓,但是可以方便阅读  
	auto bb = BasicBlock::create(module, "entry", calleeFun);
	builder->set_insert_point(bb);	// 一个BB的开始,将当前插入指令点的位置设在bb

	auto aAlloca = builder->create_alloca(Int32Type);  // 在内存中分配参数a的位置
 
	std::vector<Value *> args;	//获取函数的形参,通过Function中的iterator
	for (auto arg = calleeFun->arg_begin(); arg != calleeFun->arg_end(); arg++) {
	args.push_back(*arg);	// * 号运算符是从迭代器中取出迭代器当前指向的元素
	}
	builder->create_store(args[0], aAlloca);	// 将参数a的值存入aAlloca
	auto aLoad=builder->create_load(aAlloca);	//获取参数a的值
	auto m=builder->create_imul(aLoad,CONST_INT(2));	//a*2
  
	auto retAlloca = builder->create_alloca(Int32Type);	//分配一个保存返回值的内容
	builder->create_store(m, retAlloca);	//将m的值写入retAlloca地址中
	auto retLoad=builder->create_load(retAlloca);	//读取retAlloca地址的值
	builder->create_ret(retLoad);	//创建ret指令，即返回

	// main函数
	auto mainFun = Function::create(FunctionType::get(Int32Type, {}),"main", module);
	bb = BasicBlock::create(module, "entry", mainFun);	//创建BasicBlock
	builder->set_insert_point(bb);	//将当前插入指令点的位置设在bb
  
	auto b= builder->create_alloca(Int32Type);	//分配参数b的位置
	builder->create_store(CONST_INT(110), b);	//b=110
	auto bLoad = builder->create_load(b);	//读取b
	auto call = builder->create_call(calleeFun, {bLoad});	//创建call指令
	builder->create_ret(call);	//创建ret指令,即返回

	std::cout << module->print();
	delete module;
	return 0;
}


