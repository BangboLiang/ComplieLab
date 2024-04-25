#include "cminusf_builder.hpp"

// use these macros to get constant value
#define CONST_INT(num) \
 ConstantInt::get(num, module.get())  //以num值来创建常数类    
#define CONST_FP(num) \
 ConstantFP::get((float)num, module.get()) //以num值来创建浮点数类
#define CONST_ZERO(type) \
 ConstantZero::get(type, module.get())    //用于全局变量初始化为0 


#define Int32Type \
 Type::get_int32_type(module.get())      // 获取int类型
#define FloatType \
 Type::get_float_type(module.get())      // 获取float类型

#define checkInt(num) \
 num->get_type()->is_integer_type()      // 整型判断
#define checkFloat(num) \
 num->get_type()->is_float_type()        // 浮点型判断
#define checkPointer(num) \
 num->get_type()->is_pointer_type()      // 指针类型判断

// You can define global variables here
// to store state
Value* ret;                     // 存储返回的结果
Value* arg;                     // 存储参数指针，用于Param的处理
bool return_val=false;   // 标志是返回值还是返回地址

/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */

//program -> declaration-list
void CminusfBuilder::visit(ASTProgram &node) {  
//遍历ASTProgram中的std::vector<std::shared_ptr<ASTDeclaration>>这个类型的向量
 for (auto decl : node.declarations)         //遍历declaration-list子结点，处理每一个declaration
     decl->accept(*this);                    
}

//调用ConstantInt中的API，获取节点中整型或者浮点型的数值 
void CminusfBuilder::visit(ASTNum &node) {     
 if (node.type == TYPE_INT)                  
     ret = ConstantInt::get(node.i_val, module.get());
 else if (node.type == TYPE_FLOAT)           
     ret = ConstantFP::get(node.f_val, module.get());   
}

//var-declaration -> type-specifier ID | type-specifier ID [INTEGER]
//type-specifier -> int | float | void
//ASTVarDeclaration有两个成员变量：CminusType type和std::shared_ptr<ASTNum> num;
void CminusfBuilder::visit(ASTVarDeclaration &node) {              
    Type* tmpType;                  // 类型暂存变量，用于存储变量的类型，用于后续申请空间
    auto initializer = CONST_ZERO(tmpType);                     // 全局变量初始化为0
    if (node.type == TYPE_INT)      // 若为整型，则type为整数类型
        tmpType = Int32Type;
    else if (node.type == TYPE_FLOAT)   // 若为浮点型，则type为浮点类型
        tmpType = FloatType;
    if (node.num != nullptr) {          // 若为数组类型,则需要获取需开辟的对应大小的空间的类型指针
        auto* arrayType = ArrayType::get(tmpType, node.num->i_val); // 获取对应的数组类型

        Value* arrayAlloca;             //存储申请到的数组空间的地址
        if (scope.in_global())          //若为全局数组，则开辟全局数组，否则开辟局部数组
            arrayAlloca = GlobalVariable::create(node.id, module.get(), arrayType, false, initializer);
        else                            
            arrayAlloca = builder->create_alloca(arrayType);
        scope.push(node.id, arrayAlloca);// 将获得的数组变量加入域
    }
    else {                              // 若不是数组类型
        Value* varAlloca;               // 存储申请到的变量空间的地址
        if (scope.in_global())          // 若为全局变量，则申请全局空间，否则申请局部空间 
            varAlloca = GlobalVariable::create(node.id, module.get(), tmpType, false, initializer);
        else
            varAlloca = builder->create_alloca(tmpType);
        scope.push(node.id, varAlloca); // 将获得变量加入域
    }
}

// fun-declaration -> type-specifier ID ( params ) compound-stmt
//params -> param-list ∣ void
//param-list -> param-list , param | param
//param -> type-specifier ID ∣ type-specifier ID []
void CminusfBuilder::visit(ASTFunDeclaration &node) { //考虑函数返回类型,函数名,参数列表以及复合语句
    Type* retType;               // 存储返回类型 
    if (node.type == TYPE_INT) { retType = Int32Type; }    // 根据不同的返回类型，设置retType
    if (node.type == TYPE_FLOAT) { retType = FloatType; }
    if (node.type == TYPE_VOID) { retType = Type::get_void_type(module.get()); }
    // 根据函数声明，构造形参列表（此处的形参即参数的类型） 
    std::vector<Type*> paramsType;  // 参数类型列表 
    for (auto param : node.params) {
        if (param->isarray) {       // 若参数为数组形式，则存入首地址指针
            if (param->type == TYPE_INT)       
                paramsType.push_back(Type::get_int32_ptr_type(module.get()));
            else if(param->type == TYPE_FLOAT) 
                paramsType.push_back(Type::get_float_ptr_type(module.get()));
        }
        else {                  // 若为单个变量形式，则存入对应类型
            if (param->type == TYPE_INT)
                paramsType.push_back(Int32Type);
            else if (param->type == TYPE_FLOAT) 
                paramsType.push_back(FloatType);
        }
    }
    auto funType = FunctionType::get(retType, paramsType);// retType返回结构，paramsType函数形参结构
    auto function = Function::create(funType, node.id, module.get());   // 创建函数
    scope.push(node.id, function);  // 将函数加入到域并进入此函数作用域
    scope.enter();                  
    auto bb = BasicBlock::create(module.get(), node.id, function);// 创建基本块
    builder->set_insert_point(bb);  //将基本块加入到builder中
    // 函数传参时需要匹配实参和形参 
    std::vector<Value*> args;       // 创建vector存储实参
    for (auto arg = function->arg_begin();arg != function->arg_end();arg++) {// 遍历实参列表
        args.push_back(*arg);       // 将实参加入vector
    }
    for (int i = 0;i < node.params.size();i++) {    // 遍历形参列表
        auto param = node.params[i];        // 取出对应形参 
        arg = args[i];                      // 取出对应实参
        param->accept(*this);           // 调用param的accept进行处理 
    }
    node.compound_stmt->accept(*this);      // 处理函数体内语句compound-stmt
    //判断返回值的类型，根据对应的返回值类型，执行ret操作
    if (builder->get_insert_block()->get_terminator() == nullptr) {
        if (function->get_return_type()->is_void_type())
            builder->create_void_ret();
        else if (function->get_return_type()->is_float_type())
            builder->create_ret(CONST_FP(0.));
        else
            builder->create_ret(CONST_INT(0));
    }
    scope.exit();                       //退出该函数作用域
}

// param -> type-specifier ID / type-specifier ID [] 
void CminusfBuilder::visit(ASTParam &node) {  
//成员变量：CminusType type;std::string id;bool isarray;      
 Value* paramAlloca; /* 该参数的存储空间 */
 if (node.isarray) { // 若为数组
     if (node.type == TYPE_INT)        // 若为整型数组，则开辟整型数组存储空间
         paramAlloca = builder->create_alloca(Type::get_int32_ptr_type(module.get()));
     else if (node.type == TYPE_FLOAT) // 若为浮点数数组，则开辟浮点数数组存储空间
         paramAlloca = builder->create_alloca(Type::get_float_ptr_type(module.get()));
 }
 else {              // 若不是数组
     if (node.type == TYPE_INT)        // 若为整型，则开辟整型存储空间 
         paramAlloca = builder->create_alloca(Int32Type);
     else if (node.type == TYPE_FLOAT) // 若为浮点数，则开辟浮点数存储空间
         paramAlloca = builder->create_alloca(FloatType);
 }
 builder->create_store(arg, paramAlloca);    // 将实参load到开辟的存储空间中
 scope.push(node.id, paramAlloca);           // 将参数push到域中
}

//compound-stmt -> {local-declarations statement-list}
void CminusfBuilder::visit(ASTCompoundStmt &node) {
 scope.enter();      //进入函数体内的作用域
 for (auto local_declaration : node.local_declarations)   // 遍历并处理每一个局部声明
     local_declaration->accept(*this);   
 for (auto statement : node.statement_list)              // 遍历并处理每一个语句
     statement->accept(*this);
 scope.exit();
}

// ExpressionStmt, 表达式语句, expression-stmt -> expression; | ;
void CminusfBuilder::visit(ASTExpressionStmt &node) { 
    if (node.expression != nullptr)     // 若对应表达式存在
        node.expression->accept(*this); // 处理该表达式
}

//if语句, selection-stmt -> if (expression) statement | if (expression) statement else statement                                       
void CminusfBuilder::visit(ASTSelectionStmt &node) {        
//成员变量包含：std::shared_ptr<ASTExpression> expression;
//std::shared_ptr<ASTStatement> if_statement;
//std::shared_ptr<ASTStatement> else_statement;
    auto function = builder->get_insert_block()->get_parent();  // 获得当前所对应的函数
    node.expression->accept(*this);         // 处理条件判断对应的表达式，得到返回值存到expression中
    auto retType = ret->get_type();         // 获取表达式得到的结果类型
    Value* TF;                       // 存储if判断的结果
    if (retType->is_integer_type()) {       // 若结果为整型，则针对整型进行处理(bool类型视为整型)
        auto intType = Int32Type;  
        TF = builder->create_icmp_gt(ret, CONST_ZERO(intType));  // 大于0视为true
    }
    else if (retType->is_float_type()) {    // 若结果为浮点型，则针对浮点数进行处理
        auto floatType = FloatType;
        TF = builder->create_fcmp_gt(ret, CONST_ZERO(floatType));// 大于0视为true
    }
    if (node.else_statement != nullptr) {   // 若存在else语句
        auto trueBB = BasicBlock::create(module.get(), "true", function);   // 创建true块
        auto falseBB = BasicBlock::create(module.get(), "false", function); // 创建else块
        builder->create_cond_br(TF, trueBB, falseBB);    // 设置跳转语句

        builder->set_insert_point(trueBB);  // 符合if条件的块
        node.if_statement->accept(*this);   // 处理符合if条件后要执行的语句
        auto curTrueBB = builder->get_insert_block();   //加入该块 

        builder->set_insert_point(falseBB); //同上，处理语句并加入else块 
        node.else_statement->accept(*this); 
        auto curFalseBB = builder->get_insert_block(); 

        // 处理返回，避免跳转到对应块后无return
        auto trueTerm = builder->get_insert_block()->get_terminator();  // 判断true语句中是否存在ret语句
        auto falseTerm = builder->get_insert_block()->get_terminator();  
        BasicBlock* retBB;
        if (trueTerm == nullptr || falseTerm == nullptr) {  // 若有一方不存在return语句，则需要创建返回块
            retBB = BasicBlock::create(module.get(), "ret", function);  // 创建并进入return块 
            builder->set_insert_point(retBB);              
        }
        if (trueTerm == nullptr) {          // 若ture块中不含return，则设置跳转
            builder->set_insert_point(curTrueBB);   
            builder->create_br(retBB);              // 跳转到刚刚设置的return块
        }
        if (falseTerm == nullptr) {         
            builder->set_insert_point(curFalseBB);  
            builder->create_br(retBB);             
        }
    }
    else {  // 若不存在else语句，则只需要设置true语句块和后续语句块即可 
        auto trueBB = BasicBlock::create(module.get(), "true", function);   // true语句块
        auto retBB = BasicBlock::create(module.get(), "ret", function);     // 后续语句块
        builder->create_cond_br(TF, trueBB, retBB);  // 根据条件设置跳转指令 

        builder->set_insert_point(trueBB);  // true语句块
        node.if_statement->accept(*this);   // 执行条件符合后要执行的语句 
        if (builder->get_insert_block()->get_terminator() == nullptr)   // 补充return（同上）
            builder->create_br(retBB);      // 跳转到刚刚设置的return块并执行 
        builder->set_insert_point(retBB);   
    }
}

//while语句, iteration-stmt -> while (expression) statement
void CminusfBuilder::visit(ASTIterationStmt &node) {    
    auto function = builder->get_insert_block()->get_parent();
    auto conditionBB = BasicBlock::create(module.get(), "condition", function); // 条件判断块
    auto loopBB = BasicBlock::create(module.get(), "loop", function);           // 循环语句块
    auto retBB = BasicBlock::create(module.get(), "ret", function);             // 后续语句块
    builder->create_br(conditionBB);        // 先进条件判断 

    builder->set_insert_point(conditionBB);
    node.expression->accept(*this);         // 处理条件判断表达式，返回值存到expression中
    auto retType = ret->get_type();         // 获取表达式得到的结果类型
    Value* TF;                       // 存储if判断的结果
    if (retType->is_integer_type()) {       // 若结果为整型，则针对整型进行处理，注意bool类型视为整型
        auto intType = Int32Type;
        TF = builder->create_icmp_gt(ret, CONST_ZERO(intType));  // 大于0视为true
    }
    else if (retType->is_float_type()) {    // 若结果为浮点型，则针对浮点数进行处理
        auto floatType = FloatType;
        TF = builder->create_fcmp_gt(ret, CONST_ZERO(floatType));
    }
    builder->create_cond_br(TF, loopBB, retBB);  // 设置条件跳转语句

    builder->set_insert_point(loopBB);      // 循环语句执行块
    node.statement->accept(*this);          // 执行对应语句
    if (builder->get_insert_block()->get_terminator() == nullptr)   // 若无返回，则补充跳转
        builder->create_br(conditionBB);    // 跳回条件判断语句 

    builder->set_insert_point(retBB);       // 执行return块（即后续语句）
}

// ReturnStmt, 返回语句, return-stmt -> return;| return expression;                                 
void CminusfBuilder::visit(ASTReturnStmt &node) {   
    auto function = builder->get_insert_block()->get_parent();
    auto retType = function->get_return_type();     // 获取返回类型
    if (retType->is_void_type()) {      // 如果是void，则创建void返回，随后return，无需后续操作
        builder->create_void_ret();
        return;
    }

    node.expression->accept(*this);     // 如果不是void，处理条件判断对应的表达式，将返回值存到expression中
    auto EretType = ret->get_type();     // 获取表达式得到的结果类型
    
    // 最后处理expression返回的结果和需要return的结果类型不匹配的问题，以返回类型优先
    if (retType->is_integer_type() && EretType->is_float_type()) 
        ret = builder->create_fptosi(ret, Int32Type);
    if (retType->is_float_type() && EretType->is_integer_type())
        ret = builder->create_sitofp(ret, Int32Type);
    builder->create_ret(ret);
}

// Var, 变量引用, var -> ID | ID [expression]                 
void CminusfBuilder::visit(ASTVar &node) { 
//成员变量std::string id;
//std::shared_ptr<ASTExpression> expression;// nullptr if var is of int type    
    auto var = scope.find(node.id);             // 从域中取出对应变量
    bool should_return_lvalue = return_val;    // 判断是否需要返回地址
    return_val = false;            // 重置全局变量return_val
    Value* index = CONST_INT(0);        // 初始化index 
    if (node.expression != nullptr) {   // 若有expression，代表不是int类型的引用
        node.expression->accept(*this); // 处理expression，
        auto res = ret;                 // 存储得到结果ret
        if (checkFloat(res))            
            res = builder->create_fptosi(res, Int32Type);   // 若结果为浮点数，则矫正为整数
        index = res;                    // 赋值给index，考虑数组下标的情况 
        // 判断下标是否为负数。若是，则调用neg_idx_except函数进行处理
        auto function = builder->get_insert_block()->get_parent();  // 获取当前函数
        auto indexTest = builder->create_icmp_lt(index, CONST_ZERO(Int32Type)); // 下标是否为负数
        auto failBB = BasicBlock::create(module.get(), node.id + "Fail", function);// 为负数 
        auto passBB = BasicBlock::create(module.get(), node.id + "Pass", function); 
        builder->create_cond_br(indexTest, failBB, passBB); // 设置跳转

        builder->set_insert_point(failBB);
        auto fail = scope.find("neg_idx_except");               // 取出并调用neg_idx_except函数进行处理
        builder->create_call(static_cast<Function*>(fail), {}); 
        builder->create_br(passBB);         // 跳转到pass块

        builder->set_insert_point(passBB); 
        if (var->get_type()->get_pointer_element_type()->is_array_type())   // 若为指向数组的指针 
            var = builder->create_gep(var, { CONST_INT(0), index });    // 则进行两层寻址
        else {
            if (var->get_type()->get_pointer_element_type()->is_pointer_type()) // 若为指针
                var = builder->create_load(var);        // 则取出指针指向的元素/
            var = builder->create_gep(var, { index });  // 进行一层寻址（因为此时并非指向数组）/
        }
        if (var->get_type()->get_pointer_element_type()->is_array_type())   // 若为指向数组的指针
            var = builder->create_gep(var, { CONST_INT(0), index });    // 则进行两层寻址
        else {
            if (var->get_type()->get_pointer_element_type()->is_pointer_type()) // 若为指针
                var = builder->create_load(var);        // 则取出指针指向的元素/
            var = builder->create_gep(var, { index });  // 进行一层寻址即可 
        }
        if (should_return_lvalue) {         // 若要返回值
            ret = var;                      // 则返回var对应的地址
            return_val = false;        // 并重置全局变量return_val
        }
        else 
            ret = builder->create_load(var);// 否则则进行load
        return;
    }
    // 处理无expression的情况
    if (should_return_lvalue) { // 若要返回值
        ret = var;              // 则返回var对应的地址
        return_val = false;// 并重置全局变量return_val
    }
    else {                      
        if (var->get_type()->get_pointer_element_type()->is_array_type())   // 若指向数组
            ret = builder->create_gep(var, { CONST_INT(0), CONST_INT(0) }); // 则寻址
        else
            ret = builder->create_load(var);// 否则则进行load
    }
}

// AssignExpression, 赋值语句, var = expression | simple-expression
void CminusfBuilder::visit(ASTAssignExpression &node) { 
//成员变量std::shared_ptr<ASTVar> var;std::shared_ptr<ASTExpression> expression;    
    return_val = true;         // 设置return_val
    node.var->accept(*this);        // 处理var
    auto var = ret;                 
    node.expression->accept(*this); // 处理expression
    auto Eret = ret;                 
    auto varType = var->get_type()->get_pointer_element_type(); // 获取var的类型
    // 若赋值语句左右类型不匹配，则进行匹配
    if (varType == FloatType && checkInt(Eret))
        Eret = builder->create_sitofp(ret, FloatType);
    if (varType == Int32Type && checkFloat(Eret))
        Eret = builder->create_fptosi(ret, Int32Type);
    builder->create_store(Eret, var);// 最后赋值
}

// SimpleExpression, 比较表达式, simple-expression -> additive-expression relop additive-expression | additive-expression                                                   
void CminusfBuilder::visit(ASTSimpleExpression &node) {  
//std::shared_ptr<ASTAdditiveExpression> additive_expression_l;
//std::shared_ptr<ASTAdditiveExpression> additive_expression_r;
    node.additive_expression_l->accept(*this);  // 处理左侧的expression并用lres存储结果 
    auto lres = ret;                            
    if (node.additive_expression_r == nullptr) { return; }  // 若不存在右expression，则直接返回 
    node.additive_expression_r->accept(*this);  // 以同样的方式处理右边的expression
    auto rres = ret;
    if (checkInt(lres) && checkInt(rres)) {     // 确保两边都是整数
        switch (node.op) {  // 根据不同的比较操作，调用icmp进行处理
        case OP_LE:
            ret = builder->create_icmp_le(lres, rres);break;
        case OP_LT:
            ret = builder->create_icmp_lt(lres, rres);break;
        case OP_GT:
            ret = builder->create_icmp_gt(lres, rres);break;
        case OP_GE:
            ret = builder->create_icmp_ge(lres, rres);break;
        case OP_EQ:
            ret = builder->create_icmp_eq(lres, rres);break;
        case OP_NEQ:
            ret = builder->create_icmp_ne(lres, rres);break;
        }
    }
    else {  // 若有一边是浮点类型，则需要先将另一边也转为浮点数，再进行比较
        if (checkInt(lres))
            lres = builder->create_sitofp(lres, FloatType);
        if (checkInt(rres)) 
            rres = builder->create_sitofp(rres, FloatType);
        switch (node.op) {
        case OP_LE:
            ret = builder->create_fcmp_le(lres, rres);break;
        case OP_LT:
            ret = builder->create_fcmp_lt(lres, rres);break;
        case OP_GT:
            ret = builder->create_fcmp_gt(lres, rres);break;
        case OP_GE:
            ret = builder->create_fcmp_ge(lres, rres);break;
        case OP_EQ:
            ret = builder->create_fcmp_eq(lres, rres);break;
        case OP_NEQ:
            ret = builder->create_fcmp_ne(lres, rres);break;
        }
    }
    ret = builder->create_zext(ret, Int32Type); // 将结果作为整数保存
}

// AdditiveExpression, 加法表达式, additive-expression -> additive-expression addop term | term                                                      
void CminusfBuilder::visit(ASTAdditiveExpression &node) {   
    if (node.additive_expression == nullptr) {  // 若无加减法运算，处理term语句 
        node.term->accept(*this);return;        
    }
    node.additive_expression->accept(*this);   
    auto lres = ret;                           
    node.term->accept(*this);                   
    auto rres = ret;                            
    if (checkInt(lres) && checkInt(rres)) {     // 若两边都是整数
        switch (node.op) {  // 根据对应加法或是减法，调用iadd或isub进行处理
        case OP_PLUS:
            ret = builder->create_iadd(lres, rres);break;
        case OP_MINUS:
            ret = builder->create_isub(lres, rres);break;
        }
    }
    else { // 若有一边不是整数，转化为浮点数之后，调用fadd或fsub进行处理
        if (checkInt(lres)) 
            lres = builder->create_sitofp(lres, FloatType);
        if (checkInt(rres)) 
            rres = builder->create_sitofp(rres, FloatType);
        switch (node.op) {  
        case OP_PLUS:
            ret = builder->create_fadd(lres, rres);break;
        case OP_MINUS:
            ret = builder->create_fsub(lres, rres);break;
        }
    }
}

// Term, 乘除法语句, Term -> term mulop factor | factor       
void CminusfBuilder::visit(ASTTerm &node) {     
    if (node.term == nullptr) {             // 若无乘法运算，处理factor元素 
        node.factor->accept(*this);return;
    }
    node.term->accept(*this);   
    auto lres = ret;            
    node.factor->accept(*this);
    auto rres = ret;
    if (checkInt(lres) && checkInt(rres)) {
        switch (node.op) {  //调用imul或是idiv进行处理 
        case OP_MUL:
            ret = builder->create_imul(lres, rres);break;
        case OP_DIV:
            ret = builder->create_isdiv(lres, rres);break;
        }
    }
    else {
        if (checkInt(lres))
            lres = builder->create_sitofp(lres, FloatType);
        if (checkInt(rres)) 
            rres = builder->create_sitofp(rres, FloatType);
        switch (node.op) {  // 调用fmul或是fdiv进行处理
        case OP_MUL:
            ret = builder->create_fmul(lres, rres);break;
        case OP_DIV:
            ret = builder->create_fdiv(lres, rres);break;
        }
    }
}

// Call, 函数调用, call -> ID (args)
void CminusfBuilder::visit(ASTCall &node) {    
    auto function = static_cast<Function*>(scope.find(node.id));    // 获取需要调用的函数
    auto paramType = function->get_function_type()->param_begin();  // 获取其参数类型
    std::vector<Value*> args;       // 创建args用于存储函数参数的值，构建调用函数的参数列表
    for (auto arg : node.args) {    // 遍历形参列表 
        arg->accept(*this);         // 对每一个参数进行处理，获取参数对应的值
        if (ret->get_type()->is_pointer_type()) {   // 若参数是指针 
            args.push_back(ret);    // 则直接将值加入到参数列表
        }
        else {  // 若不是指针，则需要判断形参和实参的类型是否符合。若不符合则需要类型转换
            if (*paramType==FloatType && checkInt(ret))
                ret = builder->create_sitofp(ret, FloatType);
            if (*paramType==Int32Type && checkFloat(ret))
                ret = builder->create_fptosi(ret, Int32Type);
            args.push_back(ret);
        }
        paramType++;                // 查看下一个形参
    }
    ret = builder->create_call(static_cast<Function*>(function), args); // 最后创建函数调用
}