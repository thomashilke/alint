
#include <fstream>

#include <lexer/lexer.hpp>
#include "token_source.hpp"

enum class symbol {
  input,
  if_kw, then_kw, endif_kw,
  for_kw, to_kw, do_kw, enddo_kw,
  endmacro_kw, defmacro_kw, enddefmacro_kw,
  inline_macro_name, local_macro_name, global_macro_name, identifier,
  comment, visual_comment,
  shell_escape,
  fp_number,
  plus, minus, mult, div, equal, equal_dollar, equal_percent,
  lp, rp, lb, rb,
  literal_string,
  comma, semicolon, indent, eoi
};

typedef symbol symbol_type;
typedef token<symbol_type> token_type;
  

std::ostream& operator<<(std::ostream& stream, symbol s) {
  switch (s) {
    case symbol::input: stream << "input"; break;
    case symbol::if_kw: stream << "if_kw"; break;
    case symbol::then_kw: stream << "then_kw"; break;
    case symbol::endif_kw: stream << "endif_kw"; break;
    case symbol::for_kw: stream << "for_kw"; break;
    case symbol::to_kw: stream << "to_kw"; break;
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
    case symbol::equal_dollar: stream << "equal_dollar"; break;
    case symbol::equal_percent: stream << "equal_percent"; break;
    case symbol::lp: stream << "lp"; break;
    case symbol::rp: stream << "rp"; break;
    case symbol::lb: stream << "lb"; break;
    case symbol::rb: stream << "rb"; break;
    case symbol::literal_string:  stream << "literal_string"; break;
    case symbol::comma: stream << "comma"; break;
    case symbol::semicolon: stream << "semicolon"; break;
    case symbol::indent: stream << "indent"; break;
      
    default:
      break;
  }
  return stream;
}


regex_lexer<token<symbol> > build_alint_lexer() {
  typedef symbol symbol_type;
  typedef token<symbol_type> token_type;

  regex_lexer_builder<token_type> rlb(symbol::eoi);
  rlb.emit(symbol::input, "@\"[^\"\n]*\"");
  rlb.emit(symbol::if_kw, "IF");
  rlb.emit(symbol::then_kw, "THEN");
  rlb.emit(symbol::endif_kw, "ENDIF");
  rlb.emit(symbol::for_kw, "FOR");
  rlb.emit(symbol::to_kw, "TO");
  rlb.emit(symbol::do_kw, "DO\\(\"[^\"]*\"\\)");
  rlb.emit(symbol::enddo_kw, "ENDDO\\(\"[^\"]*\"\\)");
  rlb.emit(symbol::endmacro_kw, "endmacro");
  rlb.emit(symbol::defmacro_kw, "MACRO");
  rlb.emit(symbol::enddefmacro_kw, "ENDMACRO");
  rlb.emit(symbol::indent, "\\n *");
  rlb.emit(symbol::inline_macro_name, "M[_a-zA-Z0-9]+\\.mac");
  rlb.emit(symbol::local_macro_name, "_[_/a-zA-Z0-9]+\\.mac");
  rlb.emit(symbol::global_macro_name, "[a-zA-LN-Z][_/a-zA-Z0-9]*\\.mac");
  rlb.emit(symbol::visual_comment, "##[^\\n]*");
  rlb.emit(symbol::comment, "#[^\\n]*");
  rlb.emit(symbol::shell_escape, "![^\\n]*");
  rlb.emit(symbol::fp_number,  "[+-]?"
                               "((\\.\\d+)|(\\d+\\.)|(\\d+\\.\\d+)|(\\d+))"
                               "([eE][+-]?\\d+)?");
  rlb.emit(symbol::identifier, "[a-zA-Z][a-zA-Z0-9#_\\.]*");
  rlb.emit(symbol::plus, "\\+");
  rlb.emit(symbol::minus, "-");
  rlb.emit(symbol::mult, "\\*");
  rlb.emit(symbol::div, "/");
  rlb.emit(symbol::equal, "=");
  rlb.emit(symbol::equal_dollar, "=$");
  rlb.emit(symbol::equal_percent, "=%");
  rlb.emit(symbol::lp, "\\(");
  rlb.emit(symbol::rp, "\\)");
  rlb.emit(symbol::lb, "{");
  rlb.emit(symbol::rb, "}");
  rlb.emit(symbol::literal_string, "\"[^\"]*\"");
  rlb.emit(symbol::comma, ",");
  rlb.emit(symbol::semicolon, ";");

  rlb.skip("([ \t])*");

  return rlb.build();
}


/*
void parse_alucell_script(token_source<token_type>* t_src) {
  typedef symbol symbol_type;
  typedef token<symbol_type> token_type;
  
  token_type* current(t_src->get());

  switch (current->symbol) {
  case symbol::comment:
  case symbol::lp:
  case symbol::if_kw:
  case symbol::identifier:
  case symbol::inline_macro_name:
  case symbol::local_macro_name:
  case symbol::global_macro_name:
  case symbol::shell_escape:
  case symbol::for_kw:
  case symbol::defmacro_kw:
  case symbol::endmacro_kw:

  default:
    throw std::string("Unexpected token");
  }
}
*/

int main(int argc, char *argv[]) {
  const std::string global_macro_prefix("/usr/scratch/master/alu-data/trunk/macros/");
  const std::string local_macro_prefix("macros/");
  const std::string inline_macro_prefix("tmp/");
  
  try {
    std::ifstream file(argv[1], std::ios::in);

    file_source<token_type> ss(&file, argv[1]);
    regex_lexer<token_type> lex(build_alint_lexer());

    lex.set_source(&ss);

    token_source<token_type> t_src;
    t_src.set_source(&lex);

    token_type* current(t_src.get());
    while (current->symbol != symbol::eoi) {
      
      if (current->symbol == symbol::input) {
        const std::string filename(current->value.substr(2, current->value.size() - 3));
        std::cout << current->symbol << ": \'" << current->value << "'"
                  << std::endl;
        std::ifstream* f(new std::ifstream(filename.c_str(), std::ios::in));
        if (not f->good())
          throw std::string("Main loop error: failed to open " + filename);
        lex.set_source(new file_source<token_type>(f, filename));
        
      } else if (current->symbol == symbol::global_macro_name) {
        const std::string filename(global_macro_prefix + current->value);
        std::cout << current->symbol << ": \'" << current->value << "'" << std::endl;
        std::ifstream* f(new std::ifstream(filename.c_str(), std::ios::in));
        if (not f->good())
          throw std::string("Main loop error: failed to open " + filename);
        lex.set_source(new file_source<token_type>(f, filename));
      } else if (current->symbol == symbol::local_macro_name) {
        const std::string filename(local_macro_prefix + current->value);
        std::cout << current->symbol << ": \'" << current->value << "'" << std::endl;
        std::ifstream* f(new std::ifstream(filename.c_str(), std::ios::in));
        if (not f->good())
          throw std::string("Main loop error: failed to open " + filename);
        lex.set_source(new file_source<token_type>(f, filename));
      } else if (current->symbol == symbol::inline_macro_name) {
        const std::string filename(inline_macro_prefix + current->value + "x");
        std::cout << current->symbol << ": \'" << current->value << "'" << std::endl;
        std::ifstream* f(new std::ifstream(filename.c_str(), std::ios::in));
        if (not f->good())
          throw std::string("Main loop error: failed to open " + filename);
        lex.set_source(new file_source<token_type>(f, filename));
      } else if (current->symbol == symbol::indent) {
        std::cout << current->symbol << " " << current->value.size() - 1 << " levels" << std::endl;
      } else {
        std::cout << current->symbol << ": \'" << current->value << "'" <<std::endl;
      }
    
      current = t_src.get();
    }

  }
  catch (const std::string& e) {
    std::cout << e << std::endl;
  }
    
  
  return 0;
}
