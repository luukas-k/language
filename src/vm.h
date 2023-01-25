#pragma once

enum struct value_type {
	unknown = 0,
	i64,
	string,
	function,
	object,
};

struct object_data;

struct value {
	value_type type;

	i64 as_i64;
	std::string as_string;
	lambda* as_function;
	object_data* as_object;
};

struct object_data {
	std::string type_name;
	std::vector<std::pair<std::string, value>> members;
};

struct eval_scope {
	std::vector<std::pair<std::string, value>> values;
};

struct eval_context {
	const library* ast;
	value ret_value;
	std::vector<eval_scope> scopes;
	std::vector<std::pair<std::string, std::function<void(eval_context&, std::vector<value> args)>>> internal_functions;
};

value construct_object(eval_context& ctx, const std::string& name, std::vector<std::pair<std::string, value>> values) {
	if (name == "i64") {
		return values[0].second;
	}
	else if (name == "string") {
		return values[0].second;
	}
	else{
		for (auto& t : ctx.ast->object_types) {
			if (t->as_object_type.name == name) {
				value rv{};
				rv.type = value_type::object;

				rv.as_object = new object_data{ .type_name = name };
				auto get_value = [&](const std::string& name) {
					for (auto& [n, v] : values) {
						if(n == name)
							return v;
					}
					// assert(false); // No value given in initializer for 'name'
					return value{.type = value_type::unknown};
				};
				for (auto& m : t->as_object_type.members) {
					rv.as_object->members.push_back({m.name, get_value(m.name)});
				}

				return rv;
			}
		}
	}
	assert(false);
	return {};
}

value get_value(value& v, const std::string& name) {
	std::string sub = name.substr(0, name.find_first_of('.'));
	switch (v.type) {
		case value_type::object:
		{
			for (auto& [member, val] : v.as_object->members) {
				if (member == sub) {
					if (name.find_first_of('.') != name.npos) {
						return get_value(val, name.substr(name.find_first_of('.') + 1));
					}
					return val;
				}
			}
		}
	}
	return v;
}

value get_value(eval_context& ctx, const std::string& name) {
	std::string sub = name.substr(0, name.find_first_of('.'));
	for (i64 i = 0; i < ctx.scopes.size(); i++) {
		i64 rind = ctx.scopes.size() - 1 - i;
		auto& s = ctx.scopes[rind];

		for (auto& [n, v] : s.values) {
			if (n == sub) {
				if (name.find_first_of('.') != name.npos) {
					return get_value(v, name.substr(name.find_first_of('.') + 1));
				}
				return v;
			}
		}
	}
	assert(false); // Couldn't find value (enums fail)
	return {};
}

std::string get_value_type(const value& v) {
	switch (v.type) {
		case value_type::i64:		return "i64";
		case value_type::string:	return "string";
		case value_type::function:	return "fn";
		case value_type::object:	return v.as_object->type_name;
	}
	return "???";
}

void set_value(value& target, const std::string& mem, value new_v) {
	std::string sub = mem.substr(0, mem.find_first_of('.'));

	if (target.type == value_type::object) {
		for (auto& [n, v] : target.as_object->members) {
			if (n == sub) {
				if (mem.find_first_of('.') != mem.npos) {
					set_value(v, mem.substr(mem.find_first_of('.') + 1), new_v);
				}
				else {
					v = new_v;
				}
			}
		}
	}
	else {
		assert(false);
	}
}

void set_value(eval_context& ctx, const std::string& name, value new_v) {
	std::string sub = name.substr(0, name.find_first_of('.'));
	for (i64 i = 0; i < ctx.scopes.size(); i++) {
		i64 rind = ctx.scopes.size() - 1 - i;
		auto& s = ctx.scopes[rind];

		for (auto& [n, v] : s.values) {
			if (n == sub) {
				if (name.find_first_of('.') != name.npos) {
					set_value(v, name.substr(name.find_first_of('.') + 1), new_v);
				}
				else {
					v = new_v;
				}
				return;
			}
		}
	}
	auto& last_scope = ctx.scopes[ctx.scopes.size() - 1];
	last_scope.values.push_back({ name, new_v });
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
			// std::cout << v->as_assign.symbol << " = " << ctx.ret_value.as_i64 << "\n";
			return 0;
		}
		case ast_node_type::initialize:
		{
			evaluate(ctx, v->as_initialize.value);
			assert(v->as_initialize.symbol.type.value_or(get_value_type(ctx.ret_value)) == get_value_type(ctx.ret_value));
			init_value(ctx, v->as_initialize.symbol.name, ctx.ret_value);
			// std::cout << v->as_initialize.symbol.name << " := " << ctx.ret_value.as_i64 << "\n";
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
				case comparison_type::lt: 
				{
					set_rval_i64(ctx, lhs.as_i64 < rhs.as_i64);
					break;
				}
				case comparison_type::gt: 
				{
					set_rval_i64(ctx, lhs.as_i64 > rhs.as_i64);
					break;
				}
				case comparison_type::lte: 
				{
					set_rval_i64(ctx, lhs.as_i64 <= rhs.as_i64);
					break;
				}
				case comparison_type::gte: 
				{
					set_rval_i64(ctx, lhs.as_i64 >= rhs.as_i64);
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
		case ast_node_type::object_init: 
		{
			std::vector<std::pair<std::string, value>> values;
			for (auto& [n, v] : v->as_object_init.initial_values) {
				evaluate(ctx, v);
				values.emplace_back(n, ctx.ret_value);
			}
			
			value rv = construct_object(ctx, v->as_object_init.type, values);
			ctx.ret_value = rv;
			return 0;
		}
		case ast_node_type::loop:
		{
			switch (v->as_loop.type) {
				case loop_type::loop_while: 
				{
					while (true) {
						evaluate(ctx, v->as_loop.condition);
						value rhs = ctx.ret_value;
						if (rhs.as_i64 == 0) {
							break;
						}
						ctx.scopes.push_back({});
						evaluate(ctx, v->as_loop.scope);
						ctx.scopes.pop_back();
					}
					return 0;
				}
				default:
				{
					assert(false); // Unknown loop type
					return 0;
				}
			}

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
	ctx.ast = &lib;

	std::function<void(eval_context&, std::vector<value>)> print = [&print](eval_context& ctx, std::vector<value> vals) {
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
				case value_type::object:
				{
					print(ctx, {value{.type = value_type::string, .as_string = v.as_object->type_name + " { "}});
					bool is_first = true;
					for (auto& [name, val] : v.as_object->members) {
						if (!is_first) {
							print(ctx, {value{.type = value_type::string, .as_string = " , "}});
						}
						is_first = false;
						print(ctx, {value{.type = value_type::string, .as_string = "." + name + " = "}, val});
					}
					print(ctx, {value{.type = value_type::string, .as_string = " }"}});
					break;
				}
				default:
				{
					std::cout << "[unknown]";
					break;
				}
			}
		}
		set_rval_i64(ctx, 0);
	};
	register_internal_function(ctx, "print", print);
	register_internal_function(ctx, "println", [&print](eval_context& ctx, std::vector<value> vals) {
		print(ctx, vals);
		print(ctx, {value{.type = value_type::string, .as_string = "\n"}});
	});

	auto add_enum = [&](const enum_def& ed) {
		value v;
		v.type = value_type::object;
		v.as_object = new object_data{};
		
		v.as_object->type_name = ed.name;
		i64 i = 0;
		for (auto& n : ed.values) {
			value iv{ 
				.type = value_type::i64, 
				.as_i64 = i++ 
			};
			v.as_object->members.push_back({n, iv});
		}
		ctx.scopes[0].values.push_back({ed.name, v});
	};

	ctx.scopes.push_back({});
	for (auto obj : lib.object_types) {
		if (obj->type == ast_node_type::enum_def) {
			add_enum(obj->as_enum_def);
		}
	}
	for (auto& fn : lib.functions) {
		evaluate(ctx, fn);
	}

	evaluate(ctx, find_function(ctx, "main")->scope);
	assert(ctx.ret_value.type == value_type::i64);

	return ctx.ret_value.as_i64;
}