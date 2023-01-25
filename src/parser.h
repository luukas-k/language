#pragma once

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
	comparison,
	object_type,
	object_init,
	loop,
	enum_def,
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
	argument_decl symbol;
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

enum struct loop_type {
	unknown = 0,
	loop_for,
	loop_while,
	loop_infinite
};

struct loop_node {
	loop_type type;
	ast_node* condition;
	ast_node* scope;
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

struct object_type {
	std::string name;
	std::vector<argument_decl> members;
};

struct object_init {
	std::string type;
	std::vector<std::pair<std::string, ast_node*>> initial_values;
};

struct enum_def {
	std::string name;
	std::vector<std::string> values;
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
	object_type as_object_type;
	object_init as_object_init;
	loop_node as_loop;
	enum_def as_enum_def;
};

ast_node* make_enum(const std::string& name, const std::vector<std::string>& vals) {
	return new ast_node{
		.type = ast_node_type::enum_def,
		.as_enum_def = {
			.name = name,
			.values = vals
		}
	};
}

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

ast_node* make_initialize(argument_decl sym, ast_node* v) {
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

ast_node* make_object_type(const std::string& name, const std::vector<argument_decl>& members) {
	return new ast_node{
		.type = ast_node_type::object_type,
		.as_object_type = {
			.name = name,
			.members = members
		}
	};
}

ast_node* make_object_init(const std::string& name, const std::vector<std::pair<std::string, ast_node*>> vals) {
	return new ast_node{
		.type = ast_node_type::object_init,
		.as_object_init = {
			.type = name,
			.initial_values = vals
		}
	};
}

ast_node* make_loop(ast_node* condition, ast_node* scope, loop_type t) {
	return new ast_node{
		.type = ast_node_type::loop,
		.as_loop = {
			.type = t,
			.condition = condition,
			.scope = scope
		}
	};
}

struct parse_context {
	std::string src;
	i64 offset;
	std::vector<std::string> errors;

	char peek() const { return src[offset]; }
	char get() { return src[offset++]; }

	void error(const std::string& msg){ errors.push_back(msg); }
};


bool is_num(char c) { return c >= '0' && c <= '9'; }
bool is_ws(char c) {
	return
		c == ' ' ||
		c == '\t' ||
		c == '\n' ||
		c == '\r';
}
char to_lower(char c) { return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c; }
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

std::optional<std::string> parse_symbol(parse_context& ctx, bool scoped = true);
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

std::optional<std::string> parse_symbol(parse_context& ctx, bool scoped) {
	i64 off = ctx.offset;

	ignore_ws(ctx);
	if (is_in_alphabet(ctx.peek())) {
		std::string symbol;
		do {
			symbol.push_back(ctx.get());
		} while (is_in_alphabet(ctx.peek()) || is_num(ctx.peek()) || (ctx.peek() == '_') || (scoped && (ctx.peek() == '.')));
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

ast_node* parse_while(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);
	if(!parse_literal(ctx, "while")){
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	if(!parse_literal(ctx, "(")){
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	ast_node* cond = parse_expr(ctx);

	ignore_ws(ctx);
	if(!parse_literal(ctx, ")")){
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	ast_node* scope = parse_scope(ctx);
	if (!scope) {
		ctx.offset = off;
		return nullptr;
	}

	return make_loop(cond, scope, loop_type::loop_while);
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

std::optional<argument_decl> parse_argument_decl(parse_context& ctx);

ast_node* parse_initialize(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);
	auto let = parse_literal(ctx, "let");
	if (!let) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	auto lhs = parse_argument_decl(ctx);
	if (!lhs) {
		ctx.error("No value decleration after 'let'.");
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	bool assign = parse_literal(ctx, "=");
	if (!assign) {
		ctx.error("No assignment after 'let'.");
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	auto rhs = parse_expr(ctx);
	if (!rhs) {
		ctx.error("Missing expression after assignment in value initialization.");
		ctx.offset = off;
		return nullptr;
	}

	return make_initialize(*lhs, rhs);
}

ast_node* parse_object_initialize(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);
	auto tname = parse_symbol(ctx);
	if(!tname) {
		ctx.offset = off;
		return nullptr;
	}
	
	ignore_ws(ctx);
	if (!parse_literal(ctx, "{")) {
		ctx.offset = off;
		return nullptr;
	}

	bool is_first = true;
	std::vector<std::pair<std::string, ast_node*>> initial_vals;
	do {
		if (!is_first) {
			ignore_ws(ctx);
			if (!parse_literal(ctx, ",")) {
				break;
			}
		}
		is_first = false;
		
		ignore_ws(ctx);
		if (!parse_literal(ctx, ".")) {
			break;
		}
		
		ignore_ws(ctx);
		auto sym = parse_symbol(ctx);
		if (!sym) {
			ctx.error("No symbol after '.' in object initializer.");
			assert(false);
			ctx.offset = off;
			return nullptr;
		}

		ignore_ws(ctx);
		if (!parse_literal(ctx, "=")) {
			ctx.error("No '=' after object field specifier in object initializer.");
			// error();
			assert(false);
			ctx.offset = off;
			return nullptr;
		}

		ignore_ws(ctx);
		ast_node* val = parse_expr(ctx);
		
		if (!val) {
			ctx.error("No expression after object field specifier and '='.");
			ctx.offset = off;
			return nullptr;
		}

		initial_vals.push_back({*sym, val});
	} while(true);
	
	ignore_ws(ctx);
	if (!parse_literal(ctx, "}")) {
		ctx.error("No closing '}' in object initializer.");
		assert(false);
		ctx.offset = off;
		return nullptr;
	}

	return make_object_init(*tname, initial_vals);
}

comparison_type parse_comparison_type(parse_context& ctx) {
	i64 off = ctx.offset;

	if (ctx.peek() == '=') {
		ctx.get();
		if (ctx.peek() == '=') {
			ctx.get();
			return comparison_type::eq;
		}
	}
	else if (ctx.peek() == '>') {
		ctx.get();
		if (ctx.peek() == '=') {
			ctx.get();
			return comparison_type::gte;
		}
		else {
			return comparison_type::gt;
		}
	}
	else if (ctx.peek() == '<') {
		ctx.get();
		if (ctx.peek() == '=') {
			ctx.get();
			return comparison_type::lte;
		}
		else {
			return comparison_type::lt;
		}
	}

	ctx.offset = off;
	return comparison_type::unknown;
}

ast_node* parse_comparison(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);
	ast_node* lhs = parse_number(ctx);
	if (!lhs) {
		auto sym = parse_symbol(ctx);
		if (sym) {
			lhs = make_symbol(*sym);
		}
		else {
			ctx.offset = off;
			return nullptr;
		}
	}
	
	ignore_ws(ctx);
	comparison_type cmp_t = parse_comparison_type(ctx);
	if (cmp_t == comparison_type::unknown) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	ast_node* rhs = parse_expr(ctx);
	if (!rhs) {
		ctx.offset = off;
		return nullptr;
	}

	return make_comparison(lhs, rhs, cmp_t);
}

ast_node* parse_expr(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);

	ast_node* obj_init = parse_object_initialize(ctx);
	if(obj_init) return obj_init;

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

ast_node* parse_enum(parse_context& ctx) {
	i64 off = ctx.offset;

	ignore_ws(ctx);
	if (!parse_literal(ctx, "enum")) {
		ctx.offset = off;
		return nullptr;
	}
	
	ignore_ws(ctx);
	auto sym = parse_symbol(ctx, false);
	if (!sym) {
		ctx.offset = off;
		return nullptr;
	}
	
	ignore_ws(ctx);
	if (!parse_literal(ctx, "{")) {
		ctx.offset = off;
		return nullptr;
	}

	bool is_first = true;
	std::vector<std::string> symbols;
	do {
		if (!is_first) {
			ignore_ws(ctx);
			if (!parse_literal(ctx, ",")) {
				break;
			}
		}
		is_first = false;
		
		ignore_ws(ctx);
		auto s = parse_symbol(ctx, false);
		if (!s) {
			break;
		}
		symbols.push_back(*s);
	} while (true);
	
	ignore_ws(ctx);
	if (!parse_literal(ctx, "}")) {
		ctx.offset = off;
		return nullptr;
	}

	return make_enum(*sym, symbols);
}

ast_node* parse_statement(parse_context& ctx) {
	i64 off = ctx.offset;

	ast_node* if_node = parse_if(ctx);
	if (if_node) {
		return if_node;
	}
	ast_node* while_loop = parse_while(ctx);
	if (while_loop) {
		return while_loop;
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
	std::vector<ast_node*> object_types;
};

ast_node* parse_object_type(parse_context& ctx) {
	i64 off = ctx.offset;

	if (!parse_literal(ctx, "object")) {
		ctx.offset = off;
		return nullptr;
	}

	ignore_ws(ctx);
	auto sym = parse_symbol(ctx);
	if (!sym) {
		ctx.offset = off;
		return nullptr;
	}
	
	ignore_ws(ctx);
	if (!parse_literal(ctx, "{")) {
		ctx.offset = off;
		return nullptr;
	}

	std::vector<argument_decl> members;
	do {
		ignore_ws(ctx);
		auto member = parse_argument_decl(ctx);
		if (member) {
			members.push_back(*member);
		}
		else {
			break;
		}
	} while(true);

	
	ignore_ws(ctx);
	if(!parse_literal(ctx, "}")){
		ctx.offset = off;
		return nullptr;
	}

	return make_object_type(*sym, members);
}

library parse_library(parse_context& ctx) {
	std::vector<ast_node*> functions;
	std::vector<ast_node*> object_types;
	do {
		ignore_ws(ctx);
		ast_node* n = nullptr;
		if (n = parse_function(ctx)) {
			functions.push_back(n);
		}
		else if (n = parse_object_type(ctx)) {
			object_types.push_back(n);
		}
		else if (n = parse_enum(ctx)) {
			object_types.push_back(n);
		}
		else {
			break;
		}
	} while(true);
	return library {
		.functions = functions,
		.object_types = object_types
	};
}

std::pair<library, std::vector<std::string>> parse_ast(const std::string& src) {
	parse_context ctx{ src, 0 };
	library lib = parse_library(ctx);
	ignore_ws(ctx);
	//assert(ctx.offset == ctx.src.size()); // need to consume everything
	return { lib, ctx.errors };
}