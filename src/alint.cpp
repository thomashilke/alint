
#include <fstream>

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


void analyse_file(const std::string& file, options opt, lr_parser<symbol>& p, alint_token_source& tokens) {
  try {
    tokens.set_file(file);

    if (opt.parsing_pass) {
      tree_factory<symbol> factory;

      basic_node* tree(parse_input_to_tree<alint_token_source,
		       tree_factory<symbol>>(p, tokens, factory));
          
      if (tree) {
	if (not opt.silent)
	  std::cout << file << ": parsing succeed" << std::endl;

	if (opt.verbose)
	  tree->show(std::cout);

	if (opt.run_checkers) {
	  check_do_enddo_guards(tree);
	  check_white_spaces(tree, tokens.get_white_spaces());
	}

	if (opt.show_dependencies) {
	  std::set<std::string> filenames(show_input_and_macro_dependencies(tree, opt));
	  for (const auto& f: filenames)
	    std::cout << f << std::endl;
	}

	if (opt.recursive_parse) {
	  std::set<std::string> filenames(show_input_and_macro_dependencies(tree, opt));
	  for (const auto& f: filenames)
	    analyse_file(f, opt, p, tokens);
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
      analyse_file(file, opt, p, tokens);
    }
  }
  catch (const std::string& e) {
    std::cout << e << std::endl;
    return 1;
  }

  return 0;
}
