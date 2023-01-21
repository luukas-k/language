#include <iostream>
#include <stdint.h>
#include <optional>
#include <vector>
#include <assert.h>
#include <fstream>
#include <functional>

using i64 = int64_t;

enum struct ast_node_type {
	unknown = 0,
	number,
	string,
	bin_op,
	sequence,
	call,
	lambda,
	function,
	initialize,
	assign,
	symbol,
	conditional,
	comparison
};

struct ast_node;

enum struct bin_op_type {
	unknown = 0,
	add,
	sub,
	mul,
	div
};

struct bin_op {
	bin_op_type type;
	ast_node* lhs;
	ast_node* rhs;
};

struct argument_decl {
	std::string name;
	std::optional<std::string> type;
};

struct lambda {
	ast_node* scope;
	std::vector<argument_decl> args;
};

struct assign {
	std::string symbol;
	ast_node* value;
};

struct initialize {
	std::string symbol;
	ast_node* value;
};

struct call {
	std::string target;
	std::vector<ast_node*> args;
};

struct if_node {
	ast_node* condition;
	ast_node* scope;
	ast_node* else_scope;
};

enum struct comparison_type {
	unknown = 0,
	eq,
	lt,
	gt,
	lte,
	gte,
};

struct comparison {
	comparison_type type;
	ast_node* lhs;
	ast_node* rhs;
};

struct function {
	std::string symbol;
	ast_node* lambda;
};

struct ast_node {
	ast_node_type type;

	i64 as_number;
	bin_op as_bin_op;
	std::vector<ast_node*> as_sequence;
	call as_call;
	lambda as_lambda;
	function as_function;
	assign as_assign;
	std::string as_symbol;
	initialize as_initialize;
	argument_decl as_argument_decl;
	if_node as_if;
	comparison as_comparison;
	std::string as_string;
};

ast_node* make_number(i64 v) {
	return new ast_node{
		.type = ast_node_type::number,
		.as_number = { v }
	};
}

ast_node* make_string(const std::string& val) {
	return new ast_node{
		.type = ast_node_type::string,
		.as_string = val
	};
}

ast_node* make_bin_op(ast_node* lhs, ast_node* rhs, bin_op_type type) {
	return new ast_node{
		.type = ast_node_type::bin_op,
		.as_bin_op = {
			.type = type,
			.lhs = lhs,
			.rhs = rhs
		}
	};
}

ast_node* make_sequence(std::vector<ast_node*> nodes) {
	return new ast_node{
		.type = ast_node_type::sequence,
		.as_sequence = nodes
	};
}

ast_node* make_call(const std::string& name, std::vector<ast_node*> nodes) {
	return new ast_node{
		.type = ast_node_type::call,
		.as_call = {
			.target = name,
			.args = nodes
		}
	};
}

ast_node* make_lambda(ast_node* scope, const std::vector<argument_decl>& args) {
	return new ast_node{
		.type = ast_node_type::lambda,
		.as_lambda = {
			.scope = scope,
			.args = args
		}
	};
}

ast_node* make_assign(const std::string& sym, ast_node* v) {
	return new ast_node{
		.type = ast_node_type::assign,
		.as_assign = {
			.symbol = sym,
			.value = v
		}
	};
}

ast_node* make_initialize(const std::string& sym, ast_node* v) {
	return new ast_node{
		.type = ast_node_type::initialize,
		.as_initialize = {
			.symbol = sym,
			.value = v
		}
	};
}

ast_node* make_symbol(const std::string& sym) {
	return new ast_node{
		.type = ast_node_type::symbol,
		.as_symbol = sym
	};
}

ast_node* make_if(ast_node* cond, ast_node* scope, ast_node* else_block) {
	return new ast_node{
		.type = ast_node_type::conditional,
		.as_if = {
			.condition = cond,
			.scope = scope,
			.else_scope = else_block
		}
	};
}

ast_node* make_comparison(ast_node* lhs, ast_node* rhs, comparison_type t) {
	return new ast_node{
		.type = ast_node_type::comparison,
		.as_comparison = {
			.type = t,
			.lhs = lhs,
			.rhs = rhs
		}
	};
}

ast_node* make_function(const std::string& symbol, ast_node* lambda) {
	return new ast_node{
		.type = ast_node_type::function,
		.as_function = {
			.symbol = symbol,
			.lambda = lambda
		}
	};
}

struct parse_context {
	std::string src;
	i64 offset;

	char peek() const { return src[offset]; }
	char get() { return src[offset++]; }
};

bool is_num(char c) { return c >= '0' && c <= '9'; }
bool is_ws(char c) {
	return
		c == ' ' ||
		c == '\t' ||
		c == '\n' ||
		c == '\r';
}
char to_lower(char c) { return (c >= 'A' && c <= 'Z') ? (c - ('a' - 'A')) : c; }
bool is_in_alphabet(char c) { c = to_lower(c); return c >= 'a' && c <= 'z'; }
void ignore_ws(parse_context& ctx) {
	while (is_ws(ctx.peek())) {
		ctx.get();
	}
}

ast_node* parse_number(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);
	if (is_num(ctx.peek())) {
		i64 v = 0;
		do {
			v *= 10;
			v += (i64)(ctx.get() - '0');
		} while (is_num(ctx.peek()));
		return make_number(v);
	}

	ctx.offset = off;
	return nullptr;
}

ast_node* parse_string(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);
	if (ctx.peek() == '\"') {
		ctx.get(); // "
		std::string tmp;
		while (ctx.peek() != '\"') {
			tmp.push_back(ctx.get());
		}
		ctx.get(); // "
		return make_string(tmp);
	}

	ctx.offset = off;
	return nullptr;
}

bool parse_literal(parse_context& ctx, const std::string& v) {
	i64 off = ctx.offset;
	ignore_ws(ctx);
	for (char c : v) {
		if (c != ctx.peek()) {
			ctx.offset = off;
			return false;
		}
		ctx.get();
	}
	return true;
}

std::optional<std::string> parse_symbol(parse_context& ctx);
ast_node* parse_lambda(parse_context& ctx);
ast_node* parse_expr(parse_context& ctx);
ast_node* parse_scope(parse_context& ctx);
ast_node* parse_call(parse_context& ctx);


ast_node* parse_add(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);
	ast_node* lhs = parse_number(ctx);
	if (!lhs) {
		if (auto call = parse_call(ctx)) {
			lhs = call;
		}
		else if (auto sym = parse_symbol(ctx)) {
			lhs = make_symbol(*sym);
		}
	}
	if (!lhs) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	bool add = parse_literal(ctx, "+");
	if (!add) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	ast_node* rhs = parse_expr(ctx);
	if (!rhs) {
		ctx.offset = off;
		return nullptr;
	}

	return make_bin_op(lhs, rhs, bin_op_type::add);
}

ast_node* parse_sub(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);
	ast_node* lhs = parse_number(ctx);
	if (!lhs) {
		auto sym = parse_symbol(ctx);
		if (sym) {
			lhs = make_symbol(*sym);
		}
	}
	if (!lhs) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	bool add = parse_literal(ctx, "-");
	if (!add) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	ast_node* rhs = parse_expr(ctx);
	if (!rhs) {
		ctx.offset = off;
		return nullptr;
	}

	return make_bin_op(lhs, rhs, bin_op_type::sub);
}

ast_node* parse_mul(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);
	ast_node* lhs = parse_number(ctx);
	if (!lhs) {
		auto sym = parse_symbol(ctx);
		if (sym) {
			lhs = make_symbol(*sym);
		}
	}
	if (!lhs) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	bool add = parse_literal(ctx, "*");
	if (!add) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	ast_node* rhs = parse_expr(ctx);
	if (!rhs) {
		ctx.offset = off;
		return nullptr;
	}

	return make_bin_op(lhs, rhs, bin_op_type::mul);
}

ast_node* parse_div(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);
	ast_node* lhs = parse_number(ctx);
	if (!lhs) {
		auto sym = parse_symbol(ctx);
		if (sym) {
			lhs = make_symbol(*sym);
		}
	}
	if (!lhs) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	bool add = parse_literal(ctx, "/");
	if (!add) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	ast_node* rhs = parse_expr(ctx);
	if (!rhs) {
		ctx.offset = off;
		return nullptr;
	}

	return make_bin_op(lhs, rhs, bin_op_type::div);
}

std::optional<std::string> parse_symbol(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);
	if (is_in_alphabet(ctx.peek())) {
		std::string symbol;
		do {
			symbol.push_back(ctx.get());
		} while (is_in_alphabet(ctx.peek()) || is_num(ctx.peek()));
		return symbol;
	}

	ctx.offset = off;
	return {};
}

ast_node* parse_call(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);
	auto sym = parse_symbol(ctx);
	if (!sym) {
		ctx.offset = off;
		return nullptr;
	}
	
	ignore_ws(ctx);
	bool o_paren = parse_literal(ctx, "(");
	if (!o_paren) {
		ctx.offset = off;
		return nullptr;
	}

	std::vector<ast_node*> args;
	ignore_ws(ctx);
	auto arg0 = parse_expr(ctx);
	bool fail = false;
	if (arg0) {
		args.push_back(arg0);

		bool did = false;
		do {
			ignore_ws(ctx);
			bool comma = parse_literal(ctx, ",");
			ignore_ws(ctx);
			auto arg = parse_expr(ctx);
			if (comma && arg) {
				did = true;
				args.push_back(arg);
			}
			else if (comma || arg) {
				fail = true;
			}
			else {
				did = false;
			}
		} while(did);
	}
	if (fail) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	bool c_paren = parse_literal(ctx, ")");

	if (!c_paren) {
		ctx.offset = off;
		return nullptr;
	}

	return make_call(*sym, args);
}


ast_node* parse_assign(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);
	auto lhs = parse_symbol(ctx);
	if (!lhs) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	bool assign = parse_literal(ctx, "=");
	if (!assign) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	auto rhs = parse_expr(ctx);
	if (!rhs) {
		ctx.offset = off;
		return nullptr;
	}

	return make_assign(*lhs, rhs);
}

ast_node* parse_initialize(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);
	auto let = parse_literal(ctx, "let ");
	if (!let) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	auto lhs = parse_symbol(ctx);
	if (!lhs) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	bool assign = parse_literal(ctx, "=");
	if (!assign) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	auto rhs = parse_expr(ctx);
	if (!rhs) {
		ctx.offset = off;
		return nullptr;
	}

	return make_assign(*lhs, rhs);
}

ast_node* parse_comparison(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);
	ast_node* lhs = parse_number(ctx);
	if (!lhs) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	if (!parse_literal(ctx, "==")) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	ast_node* rhs = parse_expr(ctx);
	if (!rhs) {
		ctx.offset = off;
		return nullptr;
	}

	return make_comparison(lhs, rhs, comparison_type::eq);
}

ast_node* parse_expr(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);

	ast_node* init = parse_initialize(ctx);
	if (init) return init;

	ast_node* assign = parse_assign(ctx);
	if (assign) return assign;

	ast_node* func = parse_lambda(ctx);
	if (func) return func;

	ast_node* mul = parse_mul(ctx);
	if (mul) return mul;

	ast_node* div = parse_div(ctx);
	if (div) return div;

	ast_node* add = parse_add(ctx);
	if (add) return add;

	ast_node* sub = parse_sub(ctx);
	if (sub) return sub;

	ast_node* cmp = parse_comparison(ctx);
	if(cmp) return cmp;
	
	ast_node* call = parse_call(ctx);
	if (call) return call;

	ast_node* num = parse_number(ctx);
	if (num) return num;

	ast_node* str = parse_string(ctx);
	if(str) return str;

	auto sym = parse_symbol(ctx);
	if (sym) return make_symbol(*sym);

	ctx.offset = off;
	return nullptr;
}

ast_node* parse_if(parse_context& ctx) {
	i64 off = ctx.offset;

	if(!parse_literal(ctx, "if")){
		ctx.offset = off;
		return nullptr;
	}

	if(!parse_literal(ctx, "(")){
		ctx.offset = off;
		return nullptr;
	}

	ast_node* expr = parse_expr(ctx);
	if (!expr) {
		ctx.offset = off;
		return nullptr;
	}

	if (!parse_literal(ctx, ")")) {
		ctx.offset = off;
		return nullptr;
	}

	ast_node* scope = parse_scope(ctx);
	if (!scope) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	if (!parse_literal(ctx, "else")) {
		return make_if(expr, scope, nullptr);
	}

	ignore_ws(ctx);
	ast_node* else_block = parse_scope(ctx);
	
	if (!else_block) {
		ctx.offset = off;
		return nullptr;
	}
	
	return make_if(expr, scope, else_block);
}

ast_node* parse_statement(parse_context& ctx) {
	i64 off = ctx.offset;

	ast_node* if_node = parse_if(ctx);
	if (if_node) {
		return if_node;
	}

	ignore_ws(ctx);
	ast_node* expr = parse_expr(ctx);
	ignore_ws(ctx);
	bool lit = parse_literal(ctx, ";");

	if (expr && lit) {
		return expr;
	}

	ctx.offset = off;
	return nullptr;
}

ast_node* parse_statement_sequence(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);
	ast_node* stmnt0 = parse_statement(ctx);
	ignore_ws(ctx);
	if (stmnt0) {
		std::vector<ast_node*> stmnts{ stmnt0 };
		i64 coff = ctx.offset;
		while (ast_node* stmnt = parse_statement(ctx)) {
			stmnts.push_back(stmnt);
			coff = ctx.offset;
			ignore_ws(ctx);
		}
		ctx.offset = coff;
		return make_sequence(stmnts);
	}

	ctx.offset = 0;
	return nullptr;
}

ast_node* parse_scope(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);
	bool o_curly = parse_literal(ctx, "{");
	if (!o_curly) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	auto seq = parse_statement_sequence(ctx);
	if (!seq) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	bool c_curly = parse_literal(ctx, "}");
	if (!c_curly) {
		ctx.offset = off;
		return nullptr;
	}

	return seq;
}

std::optional<argument_decl> parse_argument_decl(parse_context& ctx) {
	i64 off = ctx.offset;
	ignore_ws(ctx);
	auto name = parse_symbol(ctx);
	if(!name) {
		ctx.offset = off;
		return {};
	}
	ignore_ws(ctx);

	if(!parse_literal(ctx, ":")) {
		return argument_decl { 
			.name = *name,
			.type = {}
		};
	}

	ignore_ws(ctx);
	auto type = parse_symbol(ctx);

	if(!type) {
		ctx.offset = off;
		return {};
	}

	return argument_decl{
		.name = *name,
		.type = *type
	};
}

ast_node* parse_lambda(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);
	bool o_paren = parse_literal(ctx, "(");

	ignore_ws(ctx);
	std::vector<argument_decl> arg_names;
	auto arg0 = parse_argument_decl(ctx);

	if (arg0) {
		arg_names.push_back({*arg0});
		bool did = false;
		do {
			ignore_ws(ctx);
			bool comma = parse_literal(ctx, ",");
			if (!comma) {
				did = false;
				break;
			}
			ignore_ws(ctx);
			auto arg = parse_argument_decl(ctx);
			if (arg) {
				arg_names.push_back({*arg});
				did = true;
			}
			else {
				did = false;
			}
		} while(did);
	}

	ignore_ws(ctx);
	bool c_paren = parse_literal(ctx, ")");

	ignore_ws(ctx);
	bool arrow = parse_literal(ctx, "->");

	ignore_ws(ctx);
	auto rtype = parse_symbol(ctx);

	ignore_ws(ctx);
	auto scope = parse_scope(ctx);

	if (o_paren && c_paren && arrow && scope) {
		return make_lambda(scope, arg_names);
	}

	ctx.offset = off;
	return nullptr;
}

ast_node* parse_function(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);
	if (!parse_literal(ctx, "fn")) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	auto symbol = parse_symbol(ctx);
	if(!symbol) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	auto body = parse_lambda(ctx);
	if (!body) {
		ctx.offset = off;
		return nullptr;
	}

	return make_function(*symbol, body);
}

struct library {
	std::vector<ast_node*> functions;
};

library parse_library(parse_context& ctx) {
	std::vector<ast_node*> functions;
	do {
		ignore_ws(ctx);
		ast_node* n = nullptr;
		if (n = parse_function(ctx)) {
			functions.push_back(n);
		}
		else {
			break;
		}
	} while(true);
	return library {
		.functions = functions
	};
}

library parse_ast(const std::string& src) {
	parse_context ctx{ src, 0 };
	return parse_library(ctx);
}

enum struct value_type {
	unknown = 0,
	i64,
	string,
	function
};

struct value {
	value_type type;

	i64 as_i64;
	std::string as_string;
	lambda* as_function;
};

struct eval_scope {
	std::vector<std::pair<std::string, value>> values;
};

struct eval_context {
	value ret_value;
	std::vector<eval_scope> scopes;
	std::vector<std::pair<std::string, std::function<void(eval_context&, std::vector<value> args)>>> internal_functions;
};

value get_value(eval_context& ctx, const std::string& name) {
	auto& s = ctx.scopes[ctx.scopes.size() - 1];
	for (auto& [n, v] : s.values) {
		if (n == name) return v;
	}	
	assert(false);
	return {};
}

std::string get_value_type(const value& v) {
	switch (v.type) {
		case value_type::i64: return "i64";
		case value_type::string: return "string";
		case value_type::function: return "fn";
	}
	return "???";
}

void set_value(eval_context& ctx, const std::string& name, value new_v) {
	auto& s = ctx.scopes[ctx.scopes.size() - 1];
	for (auto& [n, v] : s.values) {
		if (n == name) {
			v = new_v;
			return;
		}
	}
	s.values.push_back({ name, new_v });
}
void init_value(eval_context& ctx, const std::string& name, value new_v) {
	auto& s = ctx.scopes[ctx.scopes.size() - 1];
	s.values.push_back({ name, new_v });
}

void set_rval_i64(eval_context& ctx, i64 v) {
	ctx.ret_value = value{
		.type = value_type::i64,
		.as_i64 = v
	};
}

void set_rval_str(eval_context& ctx, const std::string& v) {
	ctx.ret_value = value{
		.type = value_type::string,
		.as_string = v
	};
}

void set_value_fn(eval_context& ctx, const std::string& name, lambda* fn) {
	set_value(ctx, name, value{
		.type = value_type::function,
		.as_function = fn
	});
}

void set_value_str(eval_context& ctx, const std::string& name, const std::string& v) {
	set_value(ctx, name, value{
		.type = value_type::string,
		.as_string = v
	});
}

void set_rval_fn(eval_context& ctx, lambda* fn) {
	ctx.ret_value = value{
		.type = value_type::function,
		.as_function = fn
	};
}

value add(value lhs, value rhs) {
	if (lhs.type == rhs.type && lhs.type == value_type::i64) {
		return value{
			.type = value_type::i64,
			.as_i64 = lhs.as_i64 + rhs.as_i64
		};
	}
	assert(false);
	return {};
}

value sub(value lhs, value rhs) {
	if (lhs.type == rhs.type && lhs.type == value_type::i64) {
		return value{
			.type = value_type::i64,
			.as_i64 = lhs.as_i64 - rhs.as_i64
		};
	}
	assert(false);
	return {};
}

value mul(value lhs, value rhs) {
	if (lhs.type == rhs.type && lhs.type == value_type::i64) {
		return value{
			.type = value_type::i64,
			.as_i64 = lhs.as_i64 * rhs.as_i64
		};
	}
	assert(false);
	return {};
}

value div(value lhs, value rhs) {
	if (lhs.type == rhs.type && lhs.type == value_type::i64) {
		assert(rhs.as_i64 != 0);
		return value{
			.type = value_type::i64,
			.as_i64 = lhs.as_i64 / rhs.as_i64
		};
	}
	assert(false);
	return {};
}

i64 evaluate(eval_context& ctx, ast_node* v) {
	auto get_rv = [&](i64 v) {
		return ctx.ret_value;
	};
	switch (v->type) {
		case ast_node_type::number:
		{
			set_rval_i64(ctx, v->as_number);
			return v->as_number;
		}
		case ast_node_type::bin_op:
		{
			evaluate(ctx, v->as_bin_op.lhs);
			value lhs = ctx.ret_value;

			evaluate(ctx, v->as_bin_op.rhs);
			value rhs = ctx.ret_value;

			value res{};
			switch (v->as_bin_op.type) {
				case bin_op_type::add: res = add(lhs, rhs); break;
				case bin_op_type::sub: res = sub(lhs, rhs); break;
				case bin_op_type::mul: res = mul(lhs, rhs); break;
				case bin_op_type::div: res = div(lhs, rhs); break;
			}
			ctx.ret_value = res;
			return 0;
		}
		case ast_node_type::sequence:
		{
			value rv{};
			for (auto& e : v->as_sequence) {
				evaluate(ctx, e);
				rv = ctx.ret_value;
			}
			ctx.ret_value = rv;
			return 0;
		}
		case ast_node_type::call:
		{
			std::vector<value> args;
			for (auto& arg : v->as_call.args) {
				evaluate(ctx, arg);
				args.push_back(ctx.ret_value);
			}

			for (auto& [name, cb] : ctx.internal_functions) {
				if (name == v->as_call.target) {
					cb(ctx, args);
					return 0;
				}
			}

			value fn = get_value(ctx, v->as_call.target);

			ctx.scopes.push_back({});
			init_value(ctx, "this", value{
				.type = value_type::function,
				.as_function = fn.as_function
			});
			for (i64 i = 0; i < fn.as_function->args.size(); i++) {
				if (fn.as_function->args[i].type) {
					assert(*fn.as_function->args[i].type == get_value_type(args[i]));
				}
				init_value(ctx, fn.as_function->args[i].name, args[i]);
			}
			assert(fn.as_function->args.size() == args.size()); // Passed arg count must match function signature
			evaluate(ctx, fn.as_function->scope);
			ctx.scopes.pop_back();
			return 0;
		}
		case ast_node_type::lambda:
		{
			set_rval_fn(ctx, &v->as_lambda);
			return 0;
		}
		case ast_node_type::assign:
		{
			evaluate(ctx, v->as_assign.value);
			set_value(ctx, v->as_assign.symbol, ctx.ret_value);
			return 0;
		}
		case ast_node_type::initialize:
		{
			evaluate(ctx, v->as_assign.value);
			init_value(ctx, v->as_assign.symbol, ctx.ret_value);
			return 0;
		}
		case ast_node_type::symbol:
		{
			ctx.ret_value = get_value(ctx, v->as_symbol);
			return 0;
		}
		case ast_node_type::string: 
		{
			set_rval_str(ctx, v->as_string);
			return 0;
		}
		case ast_node_type::conditional: 
		{
			evaluate(ctx, v->as_if.condition);
			value res = ctx.ret_value;
			assert(res.type == value_type::i64);
			if (res.as_i64 > 0) {
				evaluate(ctx, v->as_if.scope);
			}
			else if (v->as_if.else_scope) {
				evaluate(ctx, v->as_if.else_scope);
			}
			return 0;
		}
		case ast_node_type::comparison: 
		{
			evaluate(ctx, v->as_comparison.lhs);
			value lhs = ctx.ret_value;
			evaluate(ctx, v->as_comparison.rhs);
			value rhs = ctx.ret_value;

			switch (v->as_comparison.type) {
				case comparison_type::eq: 
				{
					set_rval_i64(ctx, lhs.as_i64 == rhs.as_i64);
					break;
				}
				default: 
				{
					assert(false);
					break;
				}
			}

			return 0;
		}
		case ast_node_type::function: 
		{
			set_value_fn(ctx, v->as_function.symbol, &v->as_function.lambda->as_lambda);
			return 0;
		}
		default: 
		{
			assert(false);
			return 0;
		}
	}
	return 0;
}

lambda* find_function(eval_context& ctx, const std::string& name) {
	for (auto& [n, v] : ctx.scopes[0].values) {
		if (n == name) {
			assert(v.type == value_type::function);
			return v.as_function;
		}
	}
	return nullptr;
}

void register_internal_function(eval_context& ctx, const std::string& name, std::function<void(eval_context&, std::vector<value>)> fn) {
	ctx.internal_functions.push_back({name, fn});
}

i64 evaluate(const library& lib) {
	eval_context ctx{};

	register_internal_function(ctx, "print", [](eval_context& ctx, std::vector<value> vals) {
		bool first = true;
		for (auto& v : vals) {
			if (!first) {
				std::cout << " ";
			}
			first = true;
			switch (v.type) {
				case value_type::string: 
				{
					std::cout << v.as_string;
					break;
				}
				case value_type::i64: 
				{
					std::cout << v.as_i64;
					break;
				}
				default:
				{
					std::cout << "[unknown]";
					break;
				}
			}
		}
		std::cout << "\n";
		set_rval_i64(ctx, 0);
	});

	ctx.scopes.push_back({});
	for (auto& fn : lib.functions) {
		evaluate(ctx, fn);
	}
	evaluate(ctx, find_function(ctx, "main")->scope);
	assert(ctx.ret_value.type == value_type::i64);
	return ctx.ret_value.as_i64;
}

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

int main(int argc, const char* argv[]) {
	auto args = parse_args(argc, argv);

	if (args.size() <= 1) {
		std::cout << "Input source file.\n";
		return -1;
	}

	std::string src_file = args[1];

	auto file = read_file("example/0_fibonacci.fl");
	if (!file) {
		std::cout << "Unable to read file.\n";
		return -1;
	}

	library ast = parse_ast(*file);

	if (ast.functions.empty()) {
		std::cout << "Unable to parse AST.\n";
		return -1;
	}

	i64 res = evaluate(ast);

	return (int)res;
}