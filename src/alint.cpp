
#include <fstream>
#include <set>
#include <stack>

#include <cstdlib>

#include <spikes/timer.hpp>

#include <parser/parser.hpp>
#include <lexer/lexer.hpp>
#include "token_source.hpp"

#include "symbol.hpp"
#include "lexer.hpp"
#include "syntax_tree.hpp"
#include "parser.hpp"

#include "syntax_checkers.hpp"


template<typename token_type>
struct error_handler: default_error_handler<token_type> {
public:
  error_handler() : status(true) {}
  virtual ~error_handler() {}
  virtual void operator()(const parse_error<token_type>& e) {
    status = false;
    default_error_handler<token_type>::operator()(e);
  }

  bool status;
};


template<typename token_type>
struct silent_error_handler: default_error_handler<token_type> {
public:
  silent_error_handler() : status(true) {}
  virtual ~silent_error_handler() {}
  virtual void operator()(const parse_error<token_type>&) {
    status = false;
  }

  bool status;
};

std::set<std::string> get_dependencies(const std::string& file,
				       options opt,
				       lr_parser<symbol>& p,
                                       cf_grammar<symbol>& g,
				       alint_token_source& tokens) {
  using token_type = token<symbol>;
  try {
    tokens.set_file(file);
    tree_factory<symbol> factory;

    silent_error_handler<token_type> handler;
    basic_node* tree(parse_input_to_tree<alint_token_source,
		     tree_factory<symbol>,
                     silent_error_handler<token_type>>(p, g, tokens, factory, handler));

    if (tree and handler.status)
      return show_input_and_macro_dependencies(tree, opt);
  }
  catch (const parse_error<token<symbol> >& e) {
    std::cout << file << ": parse failed" << std::endl;
  }
  catch (const std::string& e) {
    std::cout << file << ": parse failed" << std::endl;
  }

  return std::set<std::string>();
}



void analyse_file(const std::string& file, options opt,
                  lr_parser<symbol>& p,
                  cf_grammar<symbol>& g,
                  alint_token_source& tokens) {
  using token_type = token<symbol>;
  try {
    tokens.set_file(file);

    if (opt.parsing_pass) {
      tree_factory<symbol> factory;
      error_handler<token_type> handler;
      basic_node* tree(parse_input_to_tree<alint_token_source,
		       tree_factory<symbol>>(p, g, tokens, factory, handler));
          
      if (tree and handler.status) {
	if (not opt.silent)
	  std::cout << file << ": parsing succeed" << std::endl;

	if (opt.verbose)
	  tree->show(std::cout);

	if (opt.run_checkers) {
	  check_do_enddo_guards(tree);
	  check_white_spaces(tree, tokens.get_white_spaces());
	}

	if (opt.show_dependencies) {
	  if (not opt.recursive_parse) {
	    std::set<std::string> filenames(show_input_and_macro_dependencies(tree, opt));
	    for (const auto& f: filenames)
	      std::cout << f << std::endl;
	  } else {
	    std::set<std::string> visited;
	    std::stack<std::string> unvisited;
	    unvisited.push(file);
	    while (unvisited.size()) {
	      const std::string f(unvisited.top());
	      unvisited.pop();
	      visited.insert(f);

	      std::set<std::string> deps(get_dependencies(f, opt, p, g, tokens));
	      for (const auto& d: deps)
		if (visited.count(d) == 0)
		  unvisited.push(d);
	    }

	    for (const auto& f: visited)
	      std::cout << f << std::endl;
	  }
	} else if (opt.recursive_parse) {
	  std::set<std::string> filenames(show_input_and_macro_dependencies(tree, opt));
	  for (const auto& f: filenames)
	    analyse_file(f, opt, p, g, tokens);
	}

	if (opt.reformat_source)
	  reformat(tree, tokens.get_white_spaces(), std::cout);

	delete tree;
	tree = nullptr;
      }
    } else if (opt.lexing_pass) {
      while (tokens.get().symbol != symbol::eoi) {
	if (opt.verbose)
	  std::cout << tokens.get().symbol << " "
		    << tokens.get().value << std::endl;
	tokens.next();
      }
      if (not opt.silent)
	std::cout << file << ": lexing succeed" << std::endl;
    } else {
      throw std::string("error: no pass to check.");
    }
  }
  catch (const parse_error<token<symbol> >& e) {
    std::cout << e.get_unexpected_token().render_coordinates()
	      << " error: unexpected " << e.get_unexpected_token().symbol;

    if (e.get_unexpected_token().value.size())
      std::cout << " (" << e.get_unexpected_token().value << ")";

    if (e.get_expected_symbols().size() == 1)
      std::cout << " instead of a " << e.get_expected_symbols().front() << ".";
    else
      std::cout << ".";

    std::cout << std::endl;

    const file_source_coordinate_range* c(
					  dynamic_cast<const file_source_coordinate_range*>(
											    e.get_unexpected_token().get_coordinates()));
    show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
  }
  catch (const std::string& e) {
    std::cout << e << std::endl;
  }
}

int main(int argc, char *argv[]) {
  try {
    options opt;

    std::vector<std::string> files;
    for (int i(1); i < argc; ++i) {
      if (argv[i][0] != '-') {
	files.push_back(argv[i]);
      } else {
	if (std::string(argv[i]).size() != 2)
	  throw std::string("unrecognize option: ") + argv[i];

	switch (argv[i][1]) {
	case 'l':
	  opt.lexing_pass = true;
	  opt.parsing_pass = false;
	  break;
	case 'v':
	  opt.verbose = true;
	  break;
        case 'd':
          opt.show_dependencies = true;
          break;
        case 'c':
          opt.run_checkers = true;
          break;
	case 's':
	  opt.silent = true;
	  break;
	case 'f':
	  opt.reformat_source = true;
	  break;
	case 'r':
	  opt.recursive_parse = true;
	  break;
	case 'g':
	  opt.show_grammar = true;
	  break;
	default:
	  throw std::string("unrecognize option: ") + argv[i];
	}
      }
    }

    if (files.empty())
      throw std::string("wrong number of arguments.");

    cf_grammar<symbol> g(build_cf_grammar());
    lr_parser<symbol> p(g);
    
    if (opt.show_grammar)
      p.print(std::cout, g);

    alint_token_source tokens;
    for (const auto& file: files) {
      analyse_file(file, opt, p, g, tokens);
    }
  }
  catch (const std::string& e) {
    std::cout << e << std::endl;
    return 1;
  }

  return 0;
}
