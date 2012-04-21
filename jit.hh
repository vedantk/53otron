/*
 * jit.hh
 */

#pragma once

#include <string>
#include <vector>

using namespace std;

struct JIT;
struct ASTNode;

struct JITMachine {
	JIT* jit;

	JITMachine();
	~JITMachine();

	/* For definitions of the type;
	 * (<4 x float>, ...) => <4 x float> */
	void jit_internal(string expr);

	/* For definitions of the type;
	 * (<4 x float>*, ..., [<4 x float>*]) => void */
	void* jit_external(string defn);

	/* For any expression that can be applied over 'params'. */
	void* jit_external_expr(string expr, vector<string> params);

	/* Definitions are internal, all other expressions are external. */
	void* jit_repl_expr(string expr);

private:
	void jit_internal_ast(ASTNode* ast);
	void* jit_external_ast(ASTNode* ast);
	void* jit_external_expr_ast(ASTNode* ast, vector<string> params);
};

