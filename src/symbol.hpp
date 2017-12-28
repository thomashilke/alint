#ifndef SYMBOL_H
#define SYMBOL_H

#include <iostream>

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
  case symbol::lp: stream << "left-parent"; break;
  case symbol::rp: stream << "right-parent"; break;
  case symbol::lb: stream << "left-curly-bracket"; break;
  case symbol::rb: stream << "right-curly-bracket"; break;
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

#endif /* SYMBOL_H */
