/*
 * repl.cc
 */

#include "lang.hh"

typedef void (*func_arity3)(float*, float*, float*, float*);

int main(int argc, const char* argv[]) {
	JIT jit = JIT();

	bool do_emit = false;
	if (argc > 1 && string(argv[1]) == "-emit") {
		do_emit = true;
	}

	float out[4];
	float x[] = {1.0, 2.0, 4.0, 8.0};
	float y[] = {1.0, 1.0, 2.0, 3.0};
	float z[] = {1.0, 2.0, 6.0, 24.0};
	string args[] = {"x", "y", "z"};
	vector<string> params(args, args + 3);

	cout << "Herro! Dis simd-lisp repl." << endl;

	while (true) {
		cout << "> ";
		string expr;
		getline(cin, expr);

		if (expr.size() == 0) {
			cout << "Exzhit." << endl;
			return 0;
		}

		Lexer lex(expr);
		ASTNode* ast = Parser(lex).parse();
		void* fn = jit.compile(static_cast<ASTCall*>(ast), params);
		func_arity3 jit_func = func_arity3(fn);

		if (do_emit) {
			jit.mod->dump();
		}

		jit_func(x, y, z, out);

		for (int i=0; i < 4; ++i) {
			cout << "out[" << i << "] = " << out[i] << endl;
		}

		delete ast;
	}
}
