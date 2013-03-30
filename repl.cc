/*
 * repl.cc
 */

#include "lang.hh"

typedef void (*apply_jit_func)(float*);
typedef void (*custom_func)(float*, float*, float*);

static string get_expr() {
	cout << "> ";
	string expr;
	int opened = 0;
	while (true) {
		string chunk;
		getline(cin, chunk);
		for (size_t i=0; i < chunk.size(); ++i) {
			if (chunk[i] == '(') {
				++opened;
			} else if (chunk[i] == ')') {
				--opened;
			}
		}
		expr += chunk;
		if (opened == 0) {
			return expr;
		}
	}
}

static void print_vector(float* result) {
	cout << "<";
	for (int i=0; i < 3; ++i) {
		cout << result[i] << ", ";
	}
	cout << result[3] << ">\n";
}

int main(int argc, const char* argv[]) {
	float result[4];
	bool do_emit = false;
	if (argc > 1 && string(argv[1]) == "-emit") {
		do_emit = true;
	}

	JITMachine machine;

	cout << "Herro! Dis simd-lisp repl." << endl;

	while (true) {
		string expr = get_expr();

		if (expr.size() == 0) {
			cout << "Exiting." << endl;
			break;
		}

		void* fn = machine.jit_repl_expr(expr);
		if (do_emit) {
			machine.jit->mod->dump();
		}

		if (fn) {
			apply_jit_func func = apply_jit_func(fn);
			func(result);
			print_vector(result);
		}
	}

	custom_func magic = custom_func(machine.jit_external(
		"(def magic (x y)"
		"  (* (/ (sin x)"
		"        (cos x))"
		"     (pow x y)))"
	));
	if (do_emit) {
		machine.jit->mod->dump();
	}
	cout << "Calling magic...\n";
	float x[] = {.25, .75, 1.25, 1.75};
	float y[] = {1.0, 2.0, 4.50, 11.5};
	magic(x, y, result);
	cout << "x = ";
	print_vector(x);
	cout << "y = ";
	print_vector(y);
	cout << "Result = ";
	print_vector(result);
	return 0;
}
