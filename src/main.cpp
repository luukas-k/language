#include <iostream>
#include <stdint.h>
#include <optional>
#include <vector>
#include <assert.h>
#include <fstream>
#include <functional>
#include <chrono>

#include "parser.h"
#include "type_checker.h"
#include "vm.h"

std::optional<std::string> read_file(const std::string& fname) {
	std::fstream fs(fname, std::fstream::in);
	if (!fs) return {};
	return std::string(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());
}

std::vector<std::string> parse_args(int argc, const char* argv[]) {
	std::vector<std::string> args;
	for (i64 i = 0; i < argc; i++) {
		args.push_back(argv[i]);
	}
	return args;
}

class timer {
public:
	timer(){ mStart = std::chrono::high_resolution_clock::now(); }
	void reset() {
		mStart = std::chrono::high_resolution_clock::now();
	}
	double elapsed(){
		auto now = std::chrono::high_resolution_clock::now();
		return (double)std::chrono::duration_cast<std::chrono::milliseconds>(now - mStart).count() * 0.001;
	}
private:
	std::chrono::steady_clock::time_point mStart;
};

int main(int argc, const char* argv[]) {
	timer t;

	auto args = parse_args(argc, argv);

	if (args.size() <= 1) {
		std::cout << "Input source file.\n";
		return -1;
	}

	std::string src_file = args[1];

	// std::string fname = "example/0_fibonacci_recursive.fl";
	// std::string fname = "example/0_fibonacci_loop.fl";
	// std::string fname = "example/1_objects.fl";
	std::string fname = "example/compiler.fl";
	
	auto file = read_file(fname);

	if (!file) {
		std::cout << "Unable to read file.\n";
		return -1;
	}

	t.reset();
	auto[ast,errors] = parse_ast(*file);
	auto compile_end = t.elapsed();

	if (errors.size() == 0) {
		std::cout << "[Built successfully]\n";
	}
	else {
		std::cout << "[Encountered errors in build]\n";
		for (auto& err : errors) {
			std::cout << err << "\n";
		}
		return -1;
	}

	if (ast.functions.empty()) {
		std::cout << "Unable to parse AST.\n";
		return -1;
	}
	
	std::cout << "[Built program in]: " << compile_end << "s\n";

	t.reset();
	auto type_errors = type_check(ast);
	auto tc_end = t.elapsed();

	if (type_errors.size() == 0) {
		std::cout << "[No type errors]\n";
	}
	else {
		std::cout << "[Encountered type errors in build]\n";
		for (auto& err : type_errors) {
			std::cout << err << "\n";
		}
		//return -1;
	}
	std::cout << "[Checked types in]: " << tc_end << "s\n";

	std::cout << "[Running]\n";
	
	t.reset();
	i64 res = evaluate(ast);
	auto run_end = t.elapsed();

	std::cout << "[Ran program in]: " << run_end << "s\n";

	return (int)res;
}