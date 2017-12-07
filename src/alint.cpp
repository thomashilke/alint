
#include <fstream>

#include <parser/parser.hpp>
#include <lexer/lexer.hpp>
#include "token_source.hpp"

enum class symbol {
  /*
   *  Terminal symbol set
   */
  if_kw, if_def_kw, then_kw, else_kw, endif_kw,
  for_kw, to_kw, step_kw, do_kw, enddo_kw,
  endmacro_kw, defmacro_kw, enddefmacro_kw,
  inline_macro_name, local_macro_name, global_macro_name, identifier,
  comment, visual_comment,
  shell_escape,
  fp_number,
  plus, minus, mult, div, equal, percent, at,
  lp, rp, lb, rb,
  literal_string,
  comma, semicolon, eoi,

  /*
   *  Non-terminal symbol set
   */
    start, stmt, stmt_list, input, macro_call, macro_name, macro_arg_list, macro_arg,
    parent_expression, expression, term, factor, expression_list, parameter_list, function_call,
     if_stmt, for_stmt, macro_file, if_clause, macro_def
};

typedef symbol symbol_type;
typedef token<symbol_type> token_type;
  

std::ostream& operator<<(std::ostream& stream, symbol s) {
  switch (s) {
    /*
     *  Terminal symbol set
     */
  case symbol::if_kw: stream << "if_kw"; break;
  case symbol::if_def_kw: stream << "if_def_kw"; break;
  case symbol::then_kw: stream << "then_kw"; break;
  case symbol::else_kw: stream << "else_kw"; break;
  case symbol::endif_kw: stream << "endif_kw"; break;
  case symbol::for_kw: stream << "for_kw"; break;
  case symbol::to_kw: stream << "to_kw"; break;
  case symbol::step_kw: stream << "step_kw"; break;
  case symbol::do_kw: stream << "do_kw"; break;
  case symbol::enddo_kw: stream << "enddo_kw"; break;
  case symbol::endmacro_kw: stream << "endmacro_kw"; break;
  case symbol::defmacro_kw: stream << "defmacro_kw"; break;
  case symbol::enddefmacro_kw: stream << "enddefmacro_kw"; break;
  case symbol::identifier: stream << "identifier"; break;
  case symbol::inline_macro_name: stream << "inline_macro_name"; break;
  case symbol::local_macro_name: stream << "local_macro_name"; break;
  case symbol::global_macro_name: stream << "global_macro_name"; break;
  case symbol::comment: stream << "comment"; break;
  case symbol::visual_comment: stream << "visual_comment"; break;
  case symbol::shell_escape: stream << "shell_escape"; break;
  case symbol::fp_number: stream << "fp_number"; break;
  case symbol::plus: stream << "plus"; break;
  case symbol::minus: stream << "minus"; break;
  case symbol::mult: stream << "mult"; break;
  case symbol::div: stream << "div"; break;
  case symbol::equal: stream << "equal"; break;
  case symbol::percent: stream << "percent"; break;
  case symbol::lp: stream << "lp"; break;
  case symbol::rp: stream << "rp"; break;
  case symbol::lb: stream << "lb"; break;
  case symbol::rb: stream << "rb"; break;
  case symbol::literal_string:  stream << "literal_string"; break;
  case symbol::comma: stream << "comma"; break;
  case symbol::semicolon: stream << "semicolon"; break;
  case symbol::at: stream << "at"; break;
  case symbol::eoi: stream << "eoi"; break;

    /*
     *  Non-terminal symbol set
     */
  case symbol::start: stream << "<start>"; break;
  case symbol::stmt: stream << "<stmt>"; break;
  case symbol::stmt_list: stream << "<stmt_list>"; break;
  case symbol::input: stream << "<input>"; break;
  case symbol::macro_call: stream << "<macro_call>"; break;
  case symbol::macro_name: stream << "<macro_name>"; break;
  case symbol::macro_arg_list: stream << "<macro_arg_list>"; break;
  case symbol::macro_arg: stream << "<macro_arg>"; break;
  case symbol::parent_expression: stream << "<parent_expression>"; break;
  case symbol::expression: stream << "<expression>"; break;
  case symbol::term: stream << "<term>"; break;
  case symbol::factor: stream << "<factor>"; break;
  case symbol::expression_list: stream << "<expression_list>"; break;
  case symbol::parameter_list: stream << "<parameter_list>"; break;
  case symbol::function_call: stream << "<function_call>"; break;
  case symbol::if_stmt: stream << "<if_stmt>"; break;
  case symbol::if_clause: stream << "<if_clause>"; break;
  case symbol::for_stmt: stream << "<for_stmt>"; break;
  case symbol::macro_file: stream << "<macro_file>"; break;
  case symbol::macro_def: stream << "<macro_def>"; break;
  }
  return stream;
}


std::string to_string(const symbol& s) {
  std::ostringstream oss;
  oss << s;
  return oss.str();
}

regex_lexer<token<symbol> > build_alint_lexer() {
  typedef symbol symbol_type;
  typedef token<symbol_type> token_type;

  regex_lexer_builder<token_type> rlb(symbol::eoi);
  rlb.emit(symbol::at, "@");
  rlb.emit(symbol::if_kw, "IF");
  rlb.emit(symbol::if_def_kw, "(IFDEFINED)|(IFMMDEFINED)|(IFDBDEFINED)|(IFNOTDEFINED)|(IFASCIIFILE)");
  rlb.emit(symbol::then_kw, "THEN");
  rlb.emit(symbol::else_kw, "ELSE");
  rlb.emit(symbol::endif_kw, "ENDIF");
  rlb.emit(symbol::for_kw, "FOR");
  rlb.emit(symbol::to_kw, "TO");
  rlb.emit(symbol::step_kw, "STEP");
  rlb.emit(symbol::do_kw, "DO\\(\"[^\"]*\"\\)");
  rlb.emit(symbol::enddo_kw, "ENDDO\\(\"[^\"]*\"\\)");
  rlb.emit(symbol::endmacro_kw, "endmacro");
  rlb.emit(symbol::defmacro_kw, "MACRO");
  rlb.emit(symbol::enddefmacro_kw, "ENDMACRO");
  rlb.emit(symbol::inline_macro_name, "M[_a-zA-Z0-9]+\\.mac");
  rlb.emit(symbol::local_macro_name, "_[_/a-zA-Z0-9]+\\.mac");
  rlb.emit(symbol::global_macro_name, "[a-zA-LN-Z][-_/a-zA-Z0-9]*\\.mac");
  rlb.emit(symbol::visual_comment, "##[^\\n]*");
  rlb.emit(symbol::comment, "#[^\\n]*");
  rlb.emit(symbol::shell_escape, "![^\\n]*");
  rlb.emit(symbol::fp_number,  "((\\.\\d+)|(\\d+\\.)|(\\d+\\.\\d+)|(\\d+))"
                               "([eE][+-]?\\d+)?");
  rlb.emit(symbol::identifier, "[_a-zA-Z\\.']([a-zA-Z0-9_\\./']|(#{[^}]*}))*");
  rlb.emit(symbol::plus, "\\+");
  rlb.emit(symbol::minus, "-");
  rlb.emit(symbol::mult, "\\*");
  rlb.emit(symbol::div, "/");
  rlb.emit(symbol::equal, "(=$)|(=%)|(=\\*)|(=)");
  rlb.emit(symbol::percent, "%");
  rlb.emit(symbol::lp, "\\(");
  rlb.emit(symbol::rp, "\\)");
  rlb.emit(symbol::lb, "{");
  rlb.emit(symbol::rb, "}");
  rlb.emit(symbol::literal_string, "\"[^\"]*\"");
  rlb.emit(symbol::comma, ",");
  rlb.emit(symbol::semicolon, ";");

  rlb.skip("([ \t\n\r])*");

  return rlb.build();
}

class alint_token_source {
public:
  using symbol_type = symbol;
  using token_type = token<symbol_type>;
  
  alint_token_source(const std::string& filename)
    : file(filename.c_str(), std::ios::in),
      source(&file, filename),
      lexer(build_alint_lexer()),
      current(nullptr) {
    lexer.set_source(&source);
    current = lexer.get();
  }

  ~alint_token_source() {
    delete current;
  }

  const token<symbol>& get() const { return *current; }

  void next() {
    delete current;
    current = nullptr;
    current = lexer.get();
  }
  
private:
  std::ifstream file;
  file_source<token<symbol> > source;
  regex_lexer<token<symbol> > lexer;

  token<symbol>* current;
};


class basic_node {
public:
  virtual ~basic_node() {}
  virtual void show(std::ostream& stream, unsigned int level = 0) const = 0;
};


class node: public basic_node {
public:
  template<typename iterator_type>
  node(symbol s, iterator_type begin, iterator_type end): s(s), children(begin, end) {}

  void show(std::ostream& stream, unsigned int level) const {
    stream << std::string(level, ' ') << s << std::endl;
    for (const auto& c: children)
      c->show(stream, level + 2);
  }
  
private:
  symbol s;
  std::vector<basic_node*> children;
};


class leaf: public basic_node {
public:
  leaf(symbol s, const std::string& v): s(s), value(v) {}

  void show(std::ostream& stream, unsigned int level) const {
    stream << std::string(level, ' ') << s <<" (" << value << ")" << std::endl;
  }
  
private:
  symbol s;
  std::string value;
};


template<typename symbol_t>
class tree_factory {
public:
  using symbol_type = symbol_t;
  using token_type = token<symbol_type>;
  using node_type = basic_node;
  
  node_type* build_node(std::list<node_type*>::iterator begin,
                        std::list<node_type*>::iterator end,
                        unsigned int /* rule_id */,
                        symbol_type symbol) {

    return new node(symbol, begin, end);
  }
  
  node_type* build_leaf(const token_type& t) {
    return new leaf(t.symbol, t.value);
  }
};


int main(int argc, char *argv[]) {
  try {
    bool lexing_pass(false);
    bool parsing_pass(true);
    bool verbose(false);

    std::vector<std::string> files;
    for (int i(1); i < argc; ++i) {
      if (argv[i][0] != '-') {
	files.push_back(argv[i]);
      } else {
	if (std::string(argv[i]).size() != 2)
	  throw std::string("unrecognize option: ") + argv[i];

	switch (argv[i][1]) {
	case 'l':
	  lexing_pass = true;
	  parsing_pass = false;
	  break;
	case 'v':
	  verbose = true;
	  break;
	default:
	  throw std::string("unrecognize option: ") + argv[i];
	}
      }
    }

    if (files.empty())
      throw std::string("wrong number of arguments.");

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

    lr_parser<symbol> p(g);
    if (verbose)
      p.print(std::cout, g);

    for (const auto& file: files) {
      try {
	alint_token_source tokens(file);

	if (parsing_pass) {
	  tree_factory<symbol> factory;
	  basic_node* tree(parse_input_to_tree<alint_token_source,
			   tree_factory<symbol>>(p, tokens, factory));
	  if (tree) {
	    std::cout << file << ": parsing succeed" << std::endl;
	    if (verbose)
	      tree->show(std::cout);
	    delete tree;
	    tree = nullptr;
	  }
	} else if (lexing_pass) {
	  while (tokens.get().symbol != symbol::eoi) {
	    if (verbose)
	      std::cout << tokens.get().symbol << " "
			<< tokens.get().value << std::endl;
	    tokens.next();
	  }
	  std::cout << file << ": lexing succeed" << std::endl;
	} else {
	  throw std::string("error: no pass to check.");
	}
      }

      catch (const std::string& e) {
	std::cout << e << std::endl;
      }
    }
  }
  catch (const std::string& e) {
    std::cout << e << std::endl;
    return 1;
  }

  return 0;
}
