#pragma once

struct type_context {
	std::vector<std::string> errors;
	std::string result_type;

	std::vector<std::vector<std::pair<std::string, std::string>>> value_types;
	std::vector<std::string> types;
	std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::string>>>> member_types;

	void error(const std::string& msg){ errors.push_back(msg); }
};

void type_check(type_context& ctx, library& lib, ast_node* node) {
	auto is_type_name = [&](const std::string& name) {
		for (auto& s : ctx.types) {
			if (s == name)
				return true;
		}
		return false;
	};
	/*auto get_member_type = [&](const std::string& type, const std::string& member) -> std::string {
		for (auto& [m, t] : ctx.member_types) {
			if (m == type) {
				for (auto& [aa, tt] : t) {
					if(aa == member)
						return tt;
				}
			}
		}
		return "";
	};*/
	auto get_member_type = [&](const std::string& type, const std::string& mem) -> std::string {
		for (auto& [tname, tmembers] : ctx.member_types) {
			if (tname == type) {
				for (auto& [n, v] : tmembers) {
					if (mem == n)
						return v;
				}
			}
		}
		return "";
	};
	std::function<std::string(const std::string&)> get_symbol_type = [&](const std::string& name) -> std::string {
		if (name.find_first_of('.') != name.npos) {
			std::string sub = name.substr(0, name.find_first_of('.'));
			if (is_type_name(sub)) {
				return get_member_type(sub, name.substr(name.find_first_of('.') + 1));
			}
			return get_member_type(get_symbol_type(sub), name.substr(name.find_first_of('.') + 1));
		}
		else{
			for (auto& t : ctx.value_types) {
				for (auto& [n, v] : t) {
					if(n == name)
						return v;
				}
			}
			return "";
		}
	};
	
	switch (node->type) {
		case ast_node_type::lambda:
		{
			for (auto& s : node->as_lambda.scope->as_sequence) {
				type_check(ctx, lib, s);
				// ctx.result_type = "";
			}
			break;
		}
		case ast_node_type::initialize: 
		{
			type_check(ctx, lib, node->as_initialize.value);

			if (node->as_initialize.symbol.type.value_or(ctx.result_type) != ctx.result_type) {
				ctx.error("(Initialize) Type mismatch: '" + *node->as_initialize.symbol.type + "' != '" + ctx.result_type + "'.");
			}
			else {
				node->as_initialize.symbol.type = ctx.result_type;
				ctx.value_types[ctx.value_types.size() - 1].push_back({node->as_initialize.symbol.name, node->as_initialize.symbol.type.value_or("?")});
			}
			break;
		}
		case ast_node_type::number: 
		{
			ctx.result_type = "i64";
			break;
		}
		case ast_node_type::string:
		{
			ctx.result_type = "string";
			break;
		}
		case ast_node_type::loop: 
		{
			if(node->as_loop.condition)
				type_check(ctx, lib, node->as_loop.condition);
			ctx.value_types.push_back({});
			for (auto& s : node->as_loop.scope->as_sequence) {
				type_check(ctx, lib, s);
			}
			ctx.value_types.pop_back();
			ctx.result_type = "";
			break;
		}
		case ast_node_type::comparison: 
		{
			type_check(ctx, lib, node->as_comparison.lhs);
			auto lhs_type = ctx.result_type;
			type_check(ctx, lib, node->as_comparison.rhs);
			auto rhs_type = ctx.result_type;

			if(lhs_type != rhs_type)
				ctx.error("(Comparison) Type mismatch: '" + lhs_type + "' != '" + rhs_type + "'.");

			ctx.result_type = "i64";
			break;
		}
		case ast_node_type::symbol: 
		{
			ctx.result_type = get_symbol_type(node->as_symbol);
			break;
		}
		case ast_node_type::bin_op:
		{
			type_check(ctx, lib, node->as_bin_op.lhs);
			auto lhs_type = ctx.result_type;
			type_check(ctx, lib, node->as_bin_op.rhs);
			auto rhs_type = ctx.result_type;

			if(lhs_type != rhs_type)
				ctx.error("(Binary Op) Type mismatch: '" + lhs_type + "' != '" + rhs_type + "'.");

			ctx.result_type = lhs_type;

			break;
		}
		case ast_node_type::assign:
		{
			auto lhs_t = get_symbol_type(node->as_assign.symbol);
			type_check(ctx, lib, node->as_assign.value);
			auto rhs_t = ctx.result_type;

			if (lhs_t != rhs_t) {
				ctx.error("(Assign) Type mismatch in assign: '" + lhs_t + "' != '" + rhs_t + "'.");
			}
			ctx.result_type = "";

			break;
		}
		case ast_node_type::call: 
		{
			ctx.result_type = "?";
			break;
		}
		case ast_node_type::object_init:
		{
			if (!is_type_name(node->as_object_init.type)) {
				ctx.error("(Object Init) Unknown type name '" + node->as_object_init.type + "'.");
			}
			for (auto& [name, value] : node->as_object_init.initial_values) {
				type_check(ctx, lib, value);
				auto rhs_t = ctx.result_type;
				auto lhs_t = get_member_type(node->as_object_init.type, name);
				if (lhs_t != rhs_t) {
					ctx.error("(Object Init) Member type doesn't match type defined. '" + lhs_t + "' != '" + rhs_t + "'.");
				}
			}
			ctx.result_type = node->as_object_init.type;
			break;
		}
		default: 
		{
			assert(false);
			break;
		}
	}
}


std::vector<std::string> type_check(library& lib) {
	type_context ctx{
		.errors = {},
		.result_type = "",
		.value_types = {},
		.types = {
			"i64",
			"string"
		},
		.member_types = {},
	};

	auto is_type_name = [&](const std::string& name) {
		for (auto& s : ctx.types) {
			if (s == name)
				return true;
		}
		return false;
	};
	ctx.value_types.push_back({});
	for (auto& obj : lib.object_types) {
		if(obj->type == ast_node_type::object_type){
			ctx.types.push_back(obj->as_object_type.name);
			std::vector<std::pair<std::string, std::string>> member_types;
			for (auto& [n, t] : obj->as_object_type.members) {
				if (t.has_value()) {
					if (!is_type_name(*t)) {
						ctx.error("(Unknown type) '" + *t + "'");
					}
					member_types.push_back({ n, *t });
				}
				else
					ctx.error("(Object types) Object doesn't have type definition.");
			}
			ctx.member_types.push_back({ obj->as_object_type.name, member_types });
		}
		else if (obj->type == ast_node_type::enum_def) {
			ctx.types.push_back(obj->as_enum_def.name);
			std::vector<std::pair<std::string, std::string>> mem_types;
			for (auto& m : obj->as_enum_def.values) {
				mem_types.push_back({m, obj->as_enum_def.name});
			}
			ctx.member_types.push_back({obj->as_enum_def.name, mem_types});
		}
		else {
			assert(false);
		}
	}
	for (auto& fn : lib.functions) {
		ctx.value_types[0].push_back({fn->as_function.symbol, "fn"});
		ctx.value_types.push_back({});
		for (auto& [name, type] : fn->as_function.lambda->as_lambda.args) {
			if(!type.has_value()) 
				ctx.error("Function '" + name + "' arg '" + name + "' doesn't have a type.");
			else
				ctx.value_types[0].push_back({name, *type});
		}
		type_check(ctx, lib, fn->as_function.lambda);
		ctx.value_types.pop_back();
	}
	return ctx.errors;
}