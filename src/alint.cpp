
#include <fstream>

#include <spikes/timer.hpp>

#include <parser/parser.hpp>
#include <lexer/lexer.hpp>
#include "token_source.hpp"

#include "symbol.hpp"
#include "lexer.hpp"
#include "syntax_tree.hpp"
#include "parser.hpp"

#include "syntax_checkers.hpp"


int main(int argc, char *argv[]) {
  try {
    bool lexing_pass(false);
    bool parsing_pass(true);
    bool run_checkers(false);
    bool show_dependencies(false);
    bool verbose(false);
    bool silent(false);

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
        case 'd':
          show_dependencies = true;
          break;
        case 'c':
          run_checkers = true;
          break;
	case 's':
	  silent = true;
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
    if (verbose)
      p.print(std::cout, g);

    alint_token_source tokens;
    for (const auto& file: files) {
      try {
	tokens.set_file(file);

	if (parsing_pass) {
	  tree_factory<symbol> factory;

          basic_node* tree(parse_input_to_tree<alint_token_source,
			   tree_factory<symbol>>(p, tokens, factory));
          
	  if (tree) {
	    if (not silent)
	      std::cout << file << ": parsing succeed" << std::endl;

	    if (verbose)
	      tree->show(std::cout);

            if (run_checkers) {
              check_do_enddo_guards(tree);
              check_white_spaces(tree, tokens.get_white_spaces());
            }

            if (show_dependencies)
              show_input_and_macro_dependencies(tree);

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
	  if (not silent)
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
