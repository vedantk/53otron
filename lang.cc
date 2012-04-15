/*
 * lang.cc
 */

#include "lang.hh"

static bool isIdentLexeme(char c) {
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
			_lastToken = "";
			_lastToken += cur;
			for (_pos=_pos+1;
				_pos < _buf.size()
				&& isIdentLexeme(_buf[_pos]);
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
		ASTCall* fcall = new ASTCall(_lex.getTokenString());
		while ((probe = _lex.getToken()) && probe != TOK_CLOSE) {
			_lex.putToken(probe);
			fcall->args.push_back(parse());
		}
		return fcall;
	}
	return NULL;
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
	if (args.size() < 2) {
		return NULL;
	}

	vector<Value*> vals;
	list<ASTNode*>::iterator it = args.begin();
	for (; it != args.end(); ++it) {
		vals.push_back((*it)->codeGen(jit));
	}

	Value* ret;

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

	FUNC("+", FAdd);
	FUNC("-", FSub);
	FUNC("*", FMul);
	FUNC("/", FDiv);

	return NULL;
	#undef FUNC
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
		VectorType* VectorTy_5 = VectorType::get(
			Type::getFloatTy(mod->getContext()), 4);
		PointerType* PointerTy_6 = PointerType::get(
			IntegerType::get(mod->getContext(), 8), 0);
		std::vector<Type*>FuncTy_8_args;
		FuncTy_8_args.push_back(PointerTy_6);
		FuncTy_8_args.push_back(VectorTy_5);
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

	optimizer = new FunctionPassManager(mod);
	optimizer->add(new TargetData(*jit->getTargetData()));
	optimizer->add(createBasicAliasAnalysisPass());
	optimizer->add(createInstructionCombiningPass());
	optimizer->add(createReassociatePass());
	optimizer->add(createGVNPass());
	optimizer->add(createCFGSimplificationPass());
	optimizer->doInitialization();
}

JIT::~JIT() {
	if (builder) {
		delete builder;
	}
	delete optimizer;
	delete mod;
}

void* JIT::compile(ASTCall* fcall, vector<string> params) {
	/* Make sure arguments are unique. */
	set<string> sset;
	for (unsigned i=0; i < params.size(); ++i) {
		if (sset.count(params[i]) || params[i] == "result") {
			return NULL;
		}
		sset.insert(params[i]);
	}

	/* Construct function prototype. */
	vector<Type*> proto(params.size(), float_ptr);
	proto.push_back(result_type);
	FunctionType* ftype = FunctionType::get(void_ret,
		ArrayRef<Type*>(proto), false);
	Function* fn = Function::Create(ftype,
		Function::ExternalLinkage, fcall->name, mod);

	/* Start assembling the function. */
	BasicBlock* blk = BasicBlock::Create(mod->getContext(),
		fcall->name, fn);
	builder = new IRBuilder<>(blk);

	/* Vectorize the function arguments. */ 
	Function::arg_iterator param = fn->arg_begin();
	for (unsigned i=0; i < params.size(); ++i, ++param) {
		string name = params[i];
		Value* argptr = param;
		Value* argvec = builder->CreateBitCast(argptr, float_vec, "");
		Value* load = builder->CreateLoad(argvec, false, name);
		symbols[name] = load;
	}

	/* Store into the result vector. */
	Value* result_float = param;
	Value* body = fcall->codeGen(this);
	builder->CreateCall2(storeu, result_float, body, "");
	ReturnInst::Create(mod->getContext(), blk);

	verifyFunction(*fn);
	optimizer->run(*fn);
	return jit->getPointerToFunction(fn);
}
