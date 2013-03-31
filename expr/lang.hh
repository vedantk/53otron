/*
 * lang.hh
 */

#pragma once

#include <map>
#include <set>
#include <list>
#include <string>
#include <typeinfo>
#include <cstdlib>
#include <ctype.h>
#include <iostream>

#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Constants.h>
#include <llvm/Function.h>
#include <llvm/CallingConv.h>
#include <llvm/BasicBlock.h>
#include <llvm/Instructions.h>
#include <llvm/Pass.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Assembly/PrintModulePass.h>
#include <llvm/IRBuilder.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Support/TargetSelect.h>

#include "jit.hh"

using namespace llvm;

enum token_t {
	TOK_END = 0,
	TOK_OPEN,
	TOK_CLOSE,
	TOK_IDENT,
};

class Lexer {
	size_t _pos;
	string _buf;
	string _lastToken;
	list<token_t> _tok_q;

public:
	Lexer(string buf)
		: _pos(0), _buf(buf)
	{}

	token_t getToken();
	string getTokenString();
	void putToken(token_t tok);
};

struct JIT;

struct ASTNode {
	virtual ~ASTNode() {};
	virtual Value* codeGen(JIT* jit) = 0;
};

struct ASTDef : public ASTNode {
	string name;
	vector<string> params;
	ASTNode* body;

	ASTDef(string _name)
		: name(_name)
	{}

	bool validateArgs();
	virtual Value* codeGen(JIT* jit);
};

struct ASTForeignDef : public ASTDef {
	ASTForeignDef(string _name)
		: ASTDef(_name)
	{}

	ASTForeignDef(ASTDef* def);
	virtual Value* codeGen(JIT* jit);
};

struct ASTCall : public ASTNode {
	string name;
	list<ASTNode*> args;

	ASTCall(string _name)
		: name(_name)
	{}

	~ASTCall();
	virtual Value* codeGen(JIT* jit);
};

struct ASTVar : public ASTNode {
	string ident;

	ASTVar(string _ident)
		: ident(_ident)
	{}

	virtual Value* codeGen(JIT* jit);
};

struct ASTNumber : public ASTNode {
	float num;

	ASTNumber(float _num)
		: num(_num)
	{}

	virtual Value* codeGen(JIT* jit);
};

class Parser {
	Lexer _lex;

public:
	Parser(string expr);
	~Parser() {};
	ASTNode* parse();
};

struct JIT {
	Module* mod;
	IRBuilder<>* builder;
	map<string, Value*> symbols;
	Type* float_pod;
	Type* float_ptr;
	Type* float_vec;
	Type* float_vec_const;
	Type* void_ret;
	PointerType* result_type;
	Function* storeu;
	Function *vsqrt, *vsin, *vcos, *vpow, *vexp, *vlog;
	FunctionPassManager* optimizer;
	ExecutionEngine* jit;

	JIT();
	~JIT();
	void* compile(ASTNode* ast);
};
