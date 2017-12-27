#ifndef ALINT_PARSER_H
#define ALINT_PARSER_H

cf_grammar<symbol> build_cf_grammar() {
  cf_grammar<symbol> g(symbol::start);

  g.add_production(symbol::start, {symbol::macro_file, symbol::eoi});
  g.add_production(symbol::macro_file, {symbol::endmacro_kw});
  g.add_production(symbol::macro_file, {symbol::stmt_list, symbol::endmacro_kw});
    
  g.add_production(symbol::stmt_list, {symbol::stmt});
  g.add_production(symbol::stmt_list, {symbol::stmt, symbol::stmt_list});
    
  g.add_production(symbol::stmt, {symbol::comment});
  g.add_production(symbol::stmt, {symbol::visual_comment});
  g.add_production(symbol::stmt, {symbol::shell_escape});
  g.add_production(symbol::stmt, {symbol::input});
  g.add_production(symbol::stmt, {symbol::identifier});
  g.add_production(symbol::stmt, {symbol::fp_number});
  g.add_production(symbol::stmt, {symbol::literal_string});
  g.add_production(symbol::stmt, {symbol::mult});
  g.add_production(symbol::stmt, {symbol::macro_call});
  g.add_production(symbol::stmt, {symbol::macro_def});
  g.add_production(symbol::stmt, {symbol::parent_expression});
  g.add_production(symbol::stmt, {symbol::if_stmt});
  g.add_production(symbol::stmt, {symbol::for_stmt});
    
  g.add_production(symbol::input, {symbol::at, symbol::identifier});
  g.add_production(symbol::input, {symbol::at, symbol::literal_string});

  g.add_production(symbol::macro_def, {symbol::defmacro_kw, symbol::inline_macro_name,
        symbol::stmt_list,
        symbol::endmacro_kw,
        symbol::enddefmacro_kw, symbol::inline_macro_name});

  g.add_production(symbol::macro_call, {symbol::macro_name, symbol::lp, symbol::rp});
  g.add_production(symbol::macro_call, {symbol::macro_name, symbol::lp, symbol::macro_arg_list, symbol::rp});

  g.add_production(symbol::macro_name, {symbol::inline_macro_name});
  g.add_production(symbol::macro_name, {symbol::global_macro_name});
  g.add_production(symbol::macro_name, {symbol::local_macro_name});

  g.add_production(symbol::macro_arg_list, {symbol::macro_arg});
  g.add_production(symbol::macro_arg_list, {symbol::macro_arg, symbol::semicolon, symbol::macro_arg_list});

  g.add_production(symbol::macro_arg, {symbol::equal, symbol::identifier});
  g.add_production(symbol::macro_arg, {symbol::percent, symbol::identifier});
  g.add_production(symbol::macro_arg, {symbol::expression});
  g.add_production(symbol::macro_arg, {symbol::literal_string});

    
  g.add_production(symbol::parent_expression, {symbol::lp,
        symbol::identifier,
        symbol::equal,
        symbol::expression,
        symbol::rp});
  g.add_production(symbol::parent_expression, {symbol::lp,
        symbol::identifier,
        symbol::lp,
        symbol::parameter_list,
        symbol::rp,
        symbol::equal,
        symbol::expression,
        symbol::rp});
  g.add_production(symbol::parent_expression, {symbol::lp,
        symbol::identifier,
        symbol::equal,
        symbol::literal_string,
        symbol::rp});
    
  g.add_production(symbol::expression, {symbol::term});
  g.add_production(symbol::expression, {symbol::term, symbol::plus, symbol::expression});
  g.add_production(symbol::expression, {symbol::term, symbol::minus, symbol::expression});

  g.add_production(symbol::term, {symbol::factor});
  g.add_production(symbol::term, {symbol::factor, symbol::mult, symbol::term});
  g.add_production(symbol::term, {symbol::factor, symbol::div, symbol::term});

  g.add_production(symbol::factor, {symbol::minus, symbol::factor});
  g.add_production(symbol::factor, {symbol::plus, symbol::factor});
  g.add_production(symbol::factor, {symbol::fp_number});
  g.add_production(symbol::factor, {symbol::identifier});
  g.add_production(symbol::factor, {symbol::function_call});
  g.add_production(symbol::factor, {symbol::lp, symbol::expression_list, symbol::rp});

  g.add_production(symbol::function_call, {symbol::identifier, symbol::lp, symbol::expression_list, symbol::rp});

  g.add_production(symbol::expression_list, {symbol::expression});
  g.add_production(symbol::expression_list, {symbol::expression,
        symbol::comma,
        symbol::expression_list});

  g.add_production(symbol::parameter_list, {symbol::identifier});
  g.add_production(symbol::parameter_list, {symbol::identifier,
        symbol::comma,
        symbol::parameter_list});

  g.add_production(symbol::if_stmt, {symbol::if_clause, symbol::then_kw,
        symbol::stmt_list, symbol::endif_kw});
  g.add_production(symbol::if_stmt, {symbol::if_clause, symbol::then_kw,
        symbol::stmt_list, symbol::else_kw,
        symbol::stmt_list, symbol::endif_kw});

  g.add_production(symbol::if_clause, {symbol::if_kw, symbol::lp, symbol::expression, symbol::rp});
  g.add_production(symbol::if_clause, {symbol::if_def_kw, symbol::lp, symbol::identifier, symbol::rp});

  g.add_production(symbol::for_stmt, {symbol::for_kw,
        symbol::identifier, symbol::equal, symbol::expression,
        symbol::to_kw, symbol::expression,
        symbol::do_kw,
        symbol::stmt_list,
        symbol::enddo_kw});
  g.add_production(symbol::for_stmt, {symbol::for_kw,
        symbol::identifier, symbol::equal, symbol::expression,
        symbol::to_kw, symbol::expression,
        symbol::step_kw, symbol::expression,
        symbol::do_kw,
        symbol::stmt_list,
        symbol::enddo_kw});

  g.wrap_up();

  return g;
}


template<typename symbol_t>
class tree_factory {
public:
  using symbol_type = symbol_t;
  using token_type = token<symbol_type>;
  using node_type = basic_node;
  
  node_type* build_node(std::list<node_type*>::iterator begin,
                        std::list<node_type*>::iterator end,
                        int rule_id,
                        symbol_type symbol) {

    return new node(symbol, rule_id, begin, end);
  }

  node_type* build_leaf(alint_token_source& src) {
    return new leaf(src.get().symbol, src.get().value,
                    src.get().get_coordinates()->copy(),
                    src.get_lexem_id());
  }
};


#endif /* ALINT_PARSER_H */
