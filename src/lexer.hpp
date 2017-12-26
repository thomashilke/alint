#ifndef ALINT_LEXER_H
#define ALINT_LEXER_H

#include "file_utils.hpp"


typedef token<symbol> token_type;

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
      lexer(build_alint_lexer()) {
    lexer.set_source(&source);
    next();
  }

  alint_token_source()
    : file(), source(), lexer(build_alint_lexer()) {
    lexer.set_source(&source);
    next();
  }

  ~alint_token_source() {
    for (auto lexem: lexems)
      delete lexem;
  }

  void set_file(const std::string& filename) {
    file.close();
    for (auto lexem: lexems)
      delete lexem;
    white_spaces.clear();
    lexems.clear();
    
    file.open(filename.c_str(), std::ios::in);
    source.set_file(&file, filename);
    next();
  }

  const token<symbol>& get() const { return *lexems.back(); }
  const std::string& get_skipped_spaces() const { return white_spaces.back(); }
  std::size_t get_lexem_id() const { return white_spaces.size(); }
  
  void next() {
    try {
      lexems.push_back(lexer.get());
      white_spaces.push_back(lexer.get_skipped_characters());
    }
    catch (const lex_error& e) {
      std::cout << e.get_coordinates()->render() << " error: " << e.get_message() << std::endl;
      const file_source_coordinate_range* c(
        dynamic_cast<const file_source_coordinate_range*>(
          e.get_coordinates()));
      show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
      
      lexer.recover();
      next();
    }
  }

  const std::vector<std::string>& get_white_spaces() const {
    return white_spaces;
  }
  
private:
  std::ifstream file;
  file_source<token<symbol> > source;
  regex_lexer<token<symbol> > lexer;

  std::vector<token<symbol>* > lexems;
  std::vector<std::string> white_spaces;
};


#endif /* ALINT_LEXER_H */
