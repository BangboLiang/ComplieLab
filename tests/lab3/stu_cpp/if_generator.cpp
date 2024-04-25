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
    
int main() {
	auto module = new Module("Cminus code");
	auto builder = new IRBuilder(nullptr, module); //该类提供了独立的接口创建各种 IR 指令
	Type *Int32Type = Type::get_int32_type(module);
	Type *FloatType = Type::get_float_type(module);
  
	// main函数
	auto mainFun = Function::create(FunctionType::get(Int32Type, {}),"main", module);
	auto bb = BasicBlock::create(module, "entry", mainFun);	//创建BasicBlock
	builder->set_insert_point(bb);	//将当前插入指令点的位置设在bb
  
	auto retAlloca = builder->create_alloca(Int32Type);	//在内存中分配返回值的位置
  
	auto aAlloca=builder->create_alloca(FloatType); //在内存中分配参数a的位置
  	builder->create_store(CONST_FP(5.555), aAlloca);	//将浮点数5.555写入a的地址
  	auto aLoad = builder->create_load(aAlloca);	//读取a
  
  	auto cmp = builder->create_fcmp_gt(aLoad, CONST_FP(1.000));	//判断a是否大于1
  	auto trueBB = BasicBlock::create(module, "trueBB", mainFun);	//创建true分支
  	auto falseBB = BasicBlock::create(module, "falseBB", mainFun);	//创建false分支
  	auto retBB = BasicBlock::create(module, "", mainFun);	//创建return分支,提前create,以便true分支可以br
  	auto br = builder->create_cond_br(cmp, trueBB, falseBB);	//条件BR
  
  	builder->set_insert_point(trueBB);  // if true; 分支的开始需要SetInsertPoint设置
  	builder->create_store(CONST_INT(233), retAlloca);	//将233存储到返回值retAlloca的地址中
  	builder->create_br(retBB);  // br retBB
  
  	builder->set_insert_point(falseBB);  // if false
  	builder->create_store(CONST_INT(0), retAlloca);	//将0存储到返回值的地址中
  	builder->create_br(retBB);  // br retBB

  	builder->set_insert_point(retBB);  // ret分支
  	auto retLoad = builder->create_load(retAlloca);	//获取返回值地址中的值
  	builder->create_ret(retLoad);  //创建ret指令,返回

  	std::cout << module->print();
  	delete module;
  	return 0;
}

