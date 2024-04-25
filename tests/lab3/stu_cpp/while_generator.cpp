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
	auto module = new Module("while code");
	auto builder = new IRBuilder(nullptr, module); //该类提供了独立的接口创建各种 IR 指令
	Type *Int32Type = Type::get_int32_type(module);

	// main函数
	auto mainFun = Function::create(FunctionType::get(Int32Type, {}),"main", module);
	auto bb = BasicBlock::create(module, "entry", mainFun); //创建BasicBlock
	builder->set_insert_point(bb);  //将当前插入指令点的位置设在bb
  
	auto retAlloca = builder->create_alloca(Int32Type);   // 在内存中分配返回值的位置
  
	auto aAlloca=builder->create_alloca(Int32Type); //在内存中分配参数a的位置
	auto iAlloca=builder->create_alloca(Int32Type); //在内存中分配参数i的位置
  
	builder->create_store(CONST_INT(10), aAlloca);  //将10写入a的地址
	builder->create_store(CONST_INT(0), iAlloca);  //将0写入i的地址
  
  
	auto condBB = BasicBlock::create(module, "condBB", mainFun);	// cond分支,用于while循环的判断
	auto trueBB = BasicBlock::create(module, "trueBB", mainFun);	// 条件为真时进入true分支
	auto retBB = BasicBlock::create(module, "retBB", mainFun);	// 条件不为真时进入ret分支
	builder->create_br(condBB); 
  
	builder->set_insert_point(condBB);	//cond分支判断条件，分支的开始需要SetInsertPoint设置
	auto iLoad = builder->create_load(iAlloca);	//获取i地址中的值
	auto cmp = builder->create_icmp_lt(iLoad, CONST_INT(10));	//判断i是否小于10
	auto br = builder->create_cond_br(cmp, trueBB, retBB);  // 条件BR
   
	builder->set_insert_point(trueBB);  // if true; 
	auto aLoad = builder->create_load(aAlloca);	//读取a
	iLoad = builder->create_load(iAlloca);	//读取i
	auto add1=builder->create_iadd(iLoad,CONST_INT(1));	//i+=1
	builder->create_store(add1, iAlloca);	//i=i+1,把i+1的值写入i的地址
	iLoad = builder->create_load(iAlloca);	//从i地址读取i
	auto add2=builder->create_iadd(iLoad,aLoad);	//进行i+a
	builder->create_store(add2, aAlloca);	//把i+a的值写入a的地址
	builder->create_br(condBB);  // br condBB
  
	builder->set_insert_point(retBB);	// ret分支
	aLoad = builder->create_load(aAlloca);	//从a地址读取a
	builder->create_store(aLoad,retAlloca );	//把a的值写入返回值的地址
	auto retLoad = builder->create_load(retAlloca);	//获取返回值地址中值
	builder->create_ret(retLoad);	//创建ret指令,返回

	std::cout << module->print();
	delete module;
	return 0;
}

