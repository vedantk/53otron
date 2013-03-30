/*
 * lang.cc
 */

#include "lang.hh"

static bool isIdent(char c) {
	return (c > ' ') && (c != '(') && (c != ')');
}

token_t Lexer::getToken() {
	/* Empty the queue before scanning forward. */
	if (_tok_q.size()) {
		token_t tok = _tok_q.front();
		_tok_q.pop_front();
		return tok;
	}

	for (; _pos < _buf.size(); ++_pos) {
		char cur = _buf[_pos];
		if (cur == '(') {
			++_pos;
			return TOK_OPEN;
		} else if (cur == ')') {
			++_pos;
			return TOK_CLOSE;
		} else if (cur > ' ') {
			/* Store the last identifier. */
			_lastToken = "";
			_lastToken += cur;
			for (_pos=_pos+1;
				_pos < _buf.size()
				&& isIdent(_buf[_pos]);
				++_pos)
			{
				_lastToken += _buf[_pos];
			}
			return TOK_IDENT;
		}
	}
	return TOK_END;
}

string Lexer::getTokenString() {
	return _lastToken;
}

void Lexer::putToken(token_t tok) {
	_tok_q.push_back(tok);
}

Parser::Parser(string expr)
	: _lex(Lexer(expr))
{}

ASTNode* Parser::parse() {
	token_t cur = _lex.getToken();
	if (cur == TOK_END) {
		return NULL;
	} else if (cur == TOK_IDENT) {
		/* This parser doesn't handle 0's. */
		float num = atof(_lex.getTokenString().c_str());
		if (num == 0.0) {
			return new ASTVar(_lex.getTokenString());
		} else {
			return new ASTNumber(num);
		}
	} else if (cur == TOK_OPEN && _lex.getToken() == TOK_IDENT) {
		token_t probe;
		string cur_ident = _lex.getTokenString();
		if (cur_ident == "def") {
			/* (def <name> (...) <body>) */
			probe = _lex.getToken();
			if (probe != TOK_IDENT) return NULL;
			string def_name = _lex.getTokenString();
			probe = _lex.getToken();
			if (probe != TOK_OPEN) return NULL;
			ASTDef* defn = new ASTDef(def_name);

			while ((probe = _lex.getToken()) && probe == TOK_IDENT)
			{
				string var = _lex.getTokenString();
				if (!isalpha(var[0])) {
					delete defn;
					return NULL;
				}
				defn->params.push_back(var);
			}

			if (probe != TOK_CLOSE) {
				delete defn;
				return NULL;
			}
			defn->body = parse();
			if (_lex.getToken() != TOK_CLOSE) {
				delete defn;
				return NULL;
			}

			return defn;
		} else {
			/* (<ident> ...) */
			ASTCall* fcall = new ASTCall(cur_ident);
			while ((probe = _lex.getToken()) && probe != TOK_CLOSE)
			{
				_lex.putToken(probe);
				fcall->args.push_back(parse());
			}
			return fcall;
		}
	}
	return NULL;
}

bool ASTDef::validateArgs() {
	/* Make sure the arguments are unique. */
	set<string> sset;
	for (unsigned i=0; i < params.size(); ++i) {
		if (sset.count(params[i]) || params[i] == "result") {
			return false;
		}
		sset.insert(params[i]);
	}
	return true;
}

Value* ASTDef::codeGen(JIT* jit) {
	if (!validateArgs()) {
		return NULL;
	}

	/* (Vec4<float>, ...) => Vec4<float> */
	vector<Type*> proto(params.size(), jit->float_vec_const);
	FunctionType* ftype = FunctionType::get(jit->float_vec_const,
		ArrayRef<Type*>(proto), false);
	Function* fn = Function::Create(ftype,
		Function::ExternalLinkage, name, jit->mod);

	/* Fill up the global symbol table. */
	BasicBlock* blk = BasicBlock::Create(jit->mod->getContext(),
		name, fn);
	jit->builder = new IRBuilder<>(blk);
	Function::arg_iterator param = fn->arg_begin();
	for (unsigned i=0; i < params.size(); ++i, ++param) {
		string argname = params[i];
		jit->symbols[argname] = param;
	}

	Value* child_node = body->codeGen(jit);
	if (!child_node) return NULL;
	jit->builder->CreateRet(child_node);
	verifyFunction(*fn);
	jit->optimizer->run(*fn);
	return fn;
}

ASTForeignDef::ASTForeignDef(ASTDef* def)
	: ASTDef(def->name)
{
	params = def->params;
	body = def->body;
}

Value* ASTForeignDef::codeGen(JIT* jit) {
	if (!validateArgs()) {
		return NULL;
	}

	/* (float*, ..., i8*) => void */
	vector<Type*> proto(params.size(), jit->float_ptr);
	proto.push_back(jit->result_type);
	FunctionType* ftype = FunctionType::get(jit->void_ret,
		ArrayRef<Type*>(proto), false);
	Function* fn = Function::Create(ftype,
		Function::ExternalLinkage, name, jit->mod);

	BasicBlock* blk = BasicBlock::Create(jit->mod->getContext(),
		name, fn);
	jit->builder = new IRBuilder<>(blk);

	/* Create vectors out of the function arguments. */ 
	Function::arg_iterator param = fn->arg_begin();
	for (unsigned i=0; i < params.size(); ++i, ++param) {
		string argname = params[i];
		Value* argptr = param;
		Value* argvec = jit->builder->CreateBitCast(argptr,
			jit->float_vec, "");
		argptr = jit->builder->CreateLoad(argvec, 
			false, argname);
		jit->symbols[argname] = argptr;
	}

	/* Inject the result vector into the i8 address provided. */
	Value* child_node = body->codeGen(jit);
	if (!child_node) return NULL;
	Value* result_float = param;
	jit->builder->CreateCall2(jit->storeu,
		result_float, child_node, "");
	jit->builder->CreateRetVoid();
	verifyFunction(*fn);
	jit->optimizer->run(*fn);
	return fn;

}

ASTCall::~ASTCall() {
	list<ASTNode*>::iterator it = args.begin();
	for (; it != args.end(); ++it) {
		if (*it) {
			delete *it;
		}
	}
}

Value* ASTCall::codeGen(JIT* jit) {
	/* Generate code for all child nodes. */
	vector<Value*> vals;
	list<ASTNode*>::iterator it = args.begin();
	for (; it != args.end(); ++it) {
		Value* code = (*it)->codeGen(jit);
		if (!code) return NULL;
		vals.push_back(code);
	}

	Value* ret = NULL;

	/* Handle arithmetic. */
	#define FUNC(_name, _llvmop) \
		if (name == _name) { \
			ret = jit->builder->Create ## _llvmop ( \
				vals[0], vals[1], ""); \
			for (unsigned i=2; i < args.size(); ++i) { \
				ret = jit->builder->Create ## _llvmop ( \
					ret, vals[i]); \
			} \
			return ret; \
		} \

	if (args.size() >= 2) {
		FUNC("+", FAdd);
		FUNC("-", FSub);
		FUNC("*", FMul);
		FUNC("/", FDiv);
	}

	#undef FUNC

	/* Handle more advanced vector intrinsics. */
	#define SPECIAL_FUNC(_name, _jvar) \
	if (name == _name) { \
		return jit->builder->CreateCall(_jvar, \
			vals[0], ""); \
	} \

	if (args.size() == 1) {
		SPECIAL_FUNC("sqrt", jit->vsqrt);
		SPECIAL_FUNC("sin", jit->vsin);
		SPECIAL_FUNC("cos", jit->vcos);
		SPECIAL_FUNC("exp", jit->vexp);
		SPECIAL_FUNC("log", jit->vlog);
	}

	#undef SPECIAL_FUNC

	if (name == "pow" && args.size() == 2) {
		return jit->builder->CreateCall2(jit->vpow,
			vals[0], vals[1], "");
	}

	Function* fn = jit->mod->getFunction(name);
	if (!fn || fn->arg_size() != vals.size()) {
		return NULL;
	} else {
		return jit->builder->CreateCall(fn,
			ArrayRef<Value*>(vals), "");
	}
}

Value* ASTVar::codeGen(JIT* jit) {
	Value* val = jit->symbols[ident];
	if (!val) {
		ASTNumber zero(0.0);
		return zero.codeGen(jit);
	}
	return val;
}

Value* ASTNumber::codeGen(JIT* jit) {
	ConstantFP* val = ConstantFP::get(jit->mod->getContext(),
		APFloat(num));
	vector<Constant*> vec(4, val);
	return ConstantVector::get(ArrayRef<Constant*>(vec));
}

JIT::JIT() {
	InitializeNativeTarget();
	LLVMContext& ctx = getGlobalContext();
	mod = new Module("jit", ctx);

	string errs;
	jit = EngineBuilder(mod).setErrorStr(&errs).create();
	if (!jit) {
		cerr << errs.c_str() << endl;
	}

	float_pod = Type::getFloatTy(mod->getContext());
	float_ptr = PointerType::get(float_pod, 0);
	float_vec_const  = VectorType::get(float_pod, 4);
	float_vec = PointerType::get(float_vec_const, 0);
	void_ret = Type::getVoidTy(mod->getContext());
	result_type = PointerType::get(
		IntegerType::get(mod->getContext(), 8), 0);

	storeu = mod->getFunction("llvm.x86.sse.storeu.ps");
	if (!storeu) {
		PointerType* PointerTy_6 = PointerType::get(
			IntegerType::get(mod->getContext(), 8), 0);
		std::vector<Type*>FuncTy_8_args;
		FuncTy_8_args.push_back(PointerTy_6);
		FuncTy_8_args.push_back(float_vec_const);
		FunctionType* FuncTy_8 = FunctionType::get(
			/*Result=*/Type::getVoidTy(mod->getContext()),
			/*Params=*/FuncTy_8_args,
			/*isVarArg=*/false);
		storeu = Function::Create(
			/*Type=*/FuncTy_8,
			/*Linkage=*/GlobalValue::ExternalLinkage,
			/*Name=*/"llvm.x86.sse.storeu.ps", mod); 
		storeu->setCallingConv(CallingConv::C);
	}

	vector<Type*> func_proto(1, float_vec_const);
	FunctionType* ftype_vec_vec = FunctionType::get(
		float_vec_const, func_proto, false);

	#define INTRIN_VEC_VEC(_var, _llvmop) { \
		_var = mod->getFunction("llvm." _llvmop ".v4f32"); \
		if (!_var) { \
			_var = Function::Create( \
				ftype_vec_vec, GlobalValue::ExternalLinkage, \
				"llvm." _llvmop ".v4f32", mod); \
			_var->setCallingConv(CallingConv::C); \
		} \
	} \

	INTRIN_VEC_VEC(vsqrt, "sqrt");
	INTRIN_VEC_VEC(vsin, "sin");
	INTRIN_VEC_VEC(vcos, "cos");
	INTRIN_VEC_VEC(vexp, "exp");
	INTRIN_VEC_VEC(vlog, "log");

	func_proto.push_back(float_vec_const);
	ftype_vec_vec = FunctionType::get(
		float_vec_const, func_proto, false);
	INTRIN_VEC_VEC(vpow, "pow");

	#undef INTRIN_VEC_VEC

	optimizer = new FunctionPassManager(mod);
	// optimizer->add(new TargetData(*jit->getTargetData()));
	optimizer->add(createBasicAliasAnalysisPass());
	optimizer->add(createInstructionCombiningPass());
	optimizer->add(createReassociatePass());
	optimizer->add(createGVNPass());
	optimizer->add(createCFGSimplificationPass());
	optimizer->doInitialization();
}

JIT::~JIT() {
	delete optimizer;
	delete mod;
}

JITMachine::JITMachine() {
	jit = new JIT();
}

JITMachine::~JITMachine() {
	delete jit;
}

void JITMachine::jit_internal(string expr) {
	ASTNode* ast = Parser(expr).parse();
	if (!ast) return;
	jit_internal_ast(ast);
}

void JITMachine::jit_internal_ast(ASTNode* ast) {
	ASTDef* toplevel = dynamic_cast<ASTDef*>(ast);
	if (toplevel && typeid(toplevel) == typeid(ASTDef*)) {
		toplevel->codeGen(jit);
	}
	delete ast;
}

void* JITMachine::jit_external(string defn) {
	ASTNode* ast = Parser(defn).parse();
	if (!ast) return NULL;
	return jit_external_ast(ast);
}

void* JITMachine::jit_external_ast(ASTNode* ast) {
	void* func = NULL;
	ASTDef* toplevel = dynamic_cast<ASTDef*>(ast);
	if (toplevel && typeid(toplevel) == typeid(ASTDef*)) {
		ASTForeignDef fdef(toplevel);
		Value* val = fdef.codeGen(jit);
		if (!val) {
			goto done;
		}
		Function* fn = static_cast<Function*>(val);
		func = jit->jit->getPointerToFunction(fn);
	}

done:
	delete ast;
	return func;
}

void* JITMachine::jit_external_expr(string expr, vector<string> params) {
	ASTNode* ast = Parser(expr).parse();
	if (!ast) return NULL;
	return jit_external_expr_ast(ast, params);
}

void* JITMachine::jit_external_expr_ast(ASTNode* ast, vector<string> params) {
	void* func = NULL;
	ASTForeignDef wrapper("externalexpr");
	wrapper.body = ast;
	wrapper.params = params;
	Value* val = wrapper.codeGen(jit);
	if (!val) {
		goto done;
	} else {
		Function* fn = static_cast<Function*>(val);
		func = jit->jit->getPointerToFunction(fn);
	}	

done:
	delete ast;
	return func;
}

void* JITMachine::jit_repl_expr(string expr) {
	ASTNode* ast = Parser(expr).parse();
	ASTDef* toplevel = dynamic_cast<ASTDef*>(ast);
	if (toplevel && typeid(toplevel) == typeid(ASTDef*)) {
		jit_internal_ast(toplevel);
		return NULL;
	} else {
		vector<string> params;
		return jit_external_expr_ast(ast, params);
	}
}
