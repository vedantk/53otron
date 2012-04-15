/*
 * repl.cc
 */

#include "lang.hh"

typedef void (*apply_jit_func)(float*);

int main(int argc, const char* argv[]) {
	JIT jit = JIT();

	float result[4];
	bool do_emit = false;
	if (argc > 1 && string(argv[1]) == "-emit") {
		do_emit = true;
	}

	cout << "Herro! Dis simd-lisp repl." << endl;

	while (true) {
		cout << "> ";
		string expr;
		getline(cin, expr);

		if (expr.size() == 0) {
			cout << "Exzhit." << endl;
			return 0;
		}

		void* fn = jit.compile(expr);
		if (do_emit) {
			jit.mod->dump();
		}

		if (fn) {
			apply_jit_func func = apply_jit_func(fn);
			func(result);
			cout << "<";
			for (int i=0; i < 3; ++i) {
				cout << result[i] << ", ";
			}
			cout << result[3] << ">\n";
		}
	}
}
