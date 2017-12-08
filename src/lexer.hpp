#ifndef ALINT_LEXER_H
#define ALINT_LEXER_H

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
      lexer(build_alint_lexer()),
      current(nullptr) {
    lexer.set_source(&source);
    current = lexer.get();
  }

  alint_token_source()
    : file(), source(), lexer(build_alint_lexer()), current(nullptr) {
    lexer.set_source(&source);
    current = lexer.get();
  }

  ~alint_token_source() {
    delete current;
  }

  void set_file(const std::string& filename) {
    file.close();
    file.open(filename.c_str(), std::ios::in);
    source.set_file(&file, filename);
    next();
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


#endif /* ALINT_LEXER_H */
