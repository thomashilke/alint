#ifndef SYNTAX_CHECKERS_H
#define SYNTAX_CHECKERS_H

#include <string>
#include <cstddef>
#include <fstream>

#include "options.hpp"

/*
 *  List of possible checks
 *   - Equal assigment
 *     rhs should be an expression
 *   - Equal-dollar assigment
 *     rhs should be a string literal or an identifier
 *   - Equal-percent assigment
 *     rhs should be an identifier
 *   - Check recursively the macro sources
 *     go into global and local macros,
 *     check that inline macros have been previously defined,
 *   - Check the open and close sequences, and valid dbwrite
 *   - nested for do enddo guard which share a common prefix
 *   - nested loop with negative step (how can i check for a negative step?)
 */

void show_coordinates_in_file(const std::string& filename,
                              std::size_t line_number,
                              std::size_t column_number) {
  std::ifstream file(filename.c_str(), std::ios::in);
  if (not file)
    throw std::string("could not open ") + filename;
  
  std::string line;
  while (line_number > 0) {
    std::getline(file, line);
    --line_number;
  }

  std::cout << line << std::endl;
  std::cout << std::string(column_number, ' ') << "^ here" << std::endl;
}


class do_enddo_checker: public basic_visitor {
public:
  using coord_t = file_source_coordinate_range;
  
  virtual ~do_enddo_checker() {}
  
  virtual void visit(node& n) override {
    switch (n.get_symbol()) {
    case symbol::start:
    case symbol::macro_file:
    case symbol::stmt:
      n.get_children()[0]->accept(this);
      break;
      
    case symbol::for_stmt:
      if (n.get_children().size() == 9) {
        n.get_children()[6]->accept(this); // symbol::do
        n.get_children()[8]->accept(this); // symbol::enddo
        n.get_children()[7]->accept(this); // symbol::stmt_list
      } else {
        n.get_children()[8]->accept(this);
        n.get_children()[10]->accept(this);
        n.get_children()[9]->accept(this);
      }
      break;

    case symbol::macro_def:
      n.get_children()[1]->accept(this); // symbol::inline_macro_name
      n.get_children()[5]->accept(this); // symbol::inline_macro_name
      n.get_children()[2]->accept(this); // symbol::stmt_list
      break;
      

    case symbol::if_stmt:
      n.get_children()[2]->accept(this);
      if (n.get_children().size() == 6)
        n.get_children()[4]->accept(this);
      break;

    case symbol::stmt_list:
      n.get_children()[0]->accept(this);
      if (n.get_children().size() == 2)
        n.get_children()[1]->accept(this);
      break;
      
    default:
      break;
    }
  }

  virtual void visit(leaf& l) override {
    if (l.get_symbol() == symbol::do_kw) {
      do_guard_value = l.get_value().substr(4, l.get_value().size() - 4 - 2);
    } else if (l.get_symbol() == symbol::enddo_kw) {
      std::string enddo_guard_value(l.get_value().substr(7, l.get_value().size() - 7 - 2));
      if (enddo_guard_value != do_guard_value) {
        const coord_t* c(dynamic_cast<const coord_t*>(l.get_lexem_coordinates()));
        std::cout << l.get_lexem_coordinates()->render()
                  << " warning: DO \"" << do_guard_value
                  << "\" doesn't match ENDDO \""
                  << enddo_guard_value << "\" guard value"
                  << std::endl;
        show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());        
      }
    } else if (l.get_symbol() == symbol::inline_macro_name) {
      if (inline_macro_name.empty()) {
        inline_macro_name.push_back(l.get_value());
      } else {
        if (inline_macro_name.back() != l.get_value()) {
          const coord_t* c(dynamic_cast<const coord_t*>(l.get_lexem_coordinates()));
          std::cout << l.get_lexem_coordinates()->render()
                    << " warning: MACRO \"" << inline_macro_name.back()
                    << "\" don't match ENDMACRO \""
                    << l.get_value() << "\" guard value."
                    << std::endl;
          show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
        }
        inline_macro_name.clear();
      }
    }
  }

private:
  std::string do_guard_value;
  std::vector<std::string> inline_macro_name;
};


bool check_do_enddo_guards(basic_node* tree) {
  do_enddo_checker checker;
  tree->accept(&checker);
  return true;
}


class input_filename: public basic_visitor {
public:
  virtual void visit(node& n) {
    if (n.get_symbol() != symbol::input)
      throw std::string("not an input symbol node");
    n.get_children()[1]->accept(this);
  }
  virtual void visit(leaf& l) {
    switch (l.get_symbol()) {
    case symbol::identifier:
      return_value = l.get_value();
      break;
        
    case symbol::literal_string:
      return_value = l.get_value().substr(1, l.get_value().size() - 2);
      break;
      
    default:
      throw std::string("unexpected symbol in an input node production");
    }
  }

  std::string return_value;
};
  

std::string get_input_filename(basic_node* tree) {
  input_filename i_f;
  tree->accept(&i_f);
  return i_f.return_value;
}


class dependency_extractor: public basic_visitor {
public:
  dependency_extractor(const options& opt):
    global_macro_dir(opt.global_macro_dir),
    local_macro_dir(opt.local_macro_dir) {}
    
  virtual ~dependency_extractor() {}
  
  virtual void visit(node& n) override {
    switch (n.get_symbol()) {
    case symbol::start:
    case symbol::macro_file:
    case symbol::stmt:
      n.get_children()[0]->accept(this);
      break;

    case symbol::stmt_list:
      n.get_children()[0]->accept(this);
      if (n.get_children().size() == 2)
        n.get_children()[1]->accept(this);
      break;
      
    case symbol::for_stmt:
      if (n.get_children().size() == 9) {
        n.get_children()[7]->accept(this); // symbol::stmt_list
      } else {
        n.get_children()[9]->accept(this);
      }
      break;

    case symbol::macro_def:
      n.get_children()[2]->accept(this); // symbol::stmt_list
      break;

    case symbol::if_stmt:
      n.get_children()[2]->accept(this);
      if (n.get_children().size() == 6)
        n.get_children()[4]->accept(this);
      break;

    case symbol::macro_call:
      n.get_children()[0]->accept(this);
      break;

    case symbol::macro_name:
      n.get_children()[0]->accept(this);
      break;

    case symbol::input:
      filenames.insert(get_input_filename(&n));
      break;

    default:
      break;
    }
  }

  virtual void visit(leaf& l) override {
    switch (l.get_symbol()) {
    case symbol::inline_macro_name:
      break;
    case symbol::global_macro_name:
      filenames.insert(global_macro_dir + l.get_value());
      break;
    case symbol::local_macro_name:
      filenames.insert(local_macro_dir + l.get_value());
      break;

    default:
      break;
    }
  }

  const std::set<std::string>& get_filenames() const { return filenames; }

  void clear() { filenames.clear(); }
  
private:
  std::string global_macro_dir;
  std::string local_macro_dir;

  std::set<std::string> filenames;
};

std::set<std::string> show_input_and_macro_dependencies(basic_node* tree, const options& opt) {
  dependency_extractor extractor(opt);
  tree->accept(&extractor);
  return extractor.get_filenames();
}

class white_spaces_checker: public basic_visitor {
public:
  using coord_t = file_source_coordinate_range;
  
  white_spaces_checker(const std::vector<std::string>& ws): ws(ws) {}
  virtual void visit(node& n) override {
    switch (n.get_symbol()) {
    case symbol::start:
    case symbol::macro_file:
    case symbol::stmt:
      n.get_children()[0]->accept(this);
      break;

    case symbol::stmt_list:
      n.get_children()[0]->accept(this);
      if (n.get_children().size() == 2)
        n.get_children()[1]->accept(this);
      break;
      
    case symbol::for_stmt:
      if (n.get_children().size() == 9) {
         // initial condition
        if (not check_white_spaces_in_range(n.get_children()[1]->get_first_lexem_id(),
                                            n.get_children()[3]->get_last_lexem_id())) {
          const coord_t* c(dynamic_cast<const coord_t*>(n.get_children()[1]->get_first_lexem_coordinates()));
          std::cout << n.get_children()[1]->get_first_lexem_coordinates()->render()
                    << " warning: white spaces in the initialisation of the for statement."
                    << std::endl;
          show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
        }

         // upper boundary
        if (not check_white_spaces_in_range(n.get_children()[5]->get_first_lexem_id(),
                                            n.get_children()[5]->get_last_lexem_id())) {
          const coord_t* c(dynamic_cast<const coord_t*>(n.get_children()[5]->get_first_lexem_coordinates()));
          std::cout << n.get_children()[5]->get_first_lexem_coordinates()->render()
                    << " warning: white spaces in the stop condition of the for statement."
                    << std::endl;
          show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
        }

	// do
	if (not is_on_new_line(ws[n.get_children()[7]->get_first_lexem_id() - 1])
	    and n.get_children()[7]->get_first_leaf()->get_symbol() != symbol::comment) {
	  const coord_t* c(dynamic_cast<const coord_t*>(n.get_children()[7]->get_first_lexem_coordinates()));
	  std::cout << n.get_children()[7]->get_first_lexem_coordinates()->render()
                    << " warning: expression following the DO keyword is not on a new line" << std::endl;
	  show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
	}

	// enddo
	if (not is_on_new_line(ws[n.get_children()[8]->get_first_lexem_id()])) {
	  const coord_t* c(dynamic_cast<const coord_t*>(n.get_children()[8]->get_first_lexem_coordinates()));
	  std::cout << n.get_children()[8]->get_first_lexem_coordinates()->render()
                    << " warning: expression following the ENDDO keyword is not on a new line" << std::endl;
	  show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
	}
        
        n.get_children()[7]->accept(this); // symbol::stmt_list
      } else {
        // initial condition
        if (not check_white_spaces_in_range(n.get_children()[1]->get_first_lexem_id(),
                                            n.get_children()[3]->get_last_lexem_id())) {
          const coord_t* c(dynamic_cast<const coord_t*>(n.get_children()[1]->get_first_lexem_coordinates()));
          std::cout << n.get_children()[1]->get_first_lexem_coordinates()->render()
                    << " warning: white spaces in the initialisation of the for statement."
                    << std::endl;
          show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
        }
        
         // upper boundary
        if (not check_white_spaces_in_range(n.get_children()[5]->get_first_lexem_id(),
                                            n.get_children()[5]->get_last_lexem_id())) {
          const coord_t* c(dynamic_cast<const coord_t*>(n.get_children()[5]->get_first_lexem_coordinates()));
          std::cout << n.get_children()[5]->get_first_lexem_coordinates()->render()
                    << " warning: white spaces in the stop condition of the for statement."
                    << std::endl;
          show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
        }

         // step
        if (not check_white_spaces_in_range(n.get_children()[7]->get_first_lexem_id(),
                                            n.get_children()[7]->get_last_lexem_id())) {
          const coord_t* c(dynamic_cast<const coord_t*>(n.get_children()[7]->get_first_lexem_coordinates()));
          std::cout << n.get_children()[7]->get_first_lexem_coordinates()->render()
                    << " warning: white spaces in the step condition of the for statement."
                    << std::endl;
          show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
        }

	// do
	if (not is_on_new_line(ws[n.get_children()[8]->get_first_lexem_id()])
	    and n.get_children()[9]->get_first_leaf()->get_symbol() != symbol::comment) {
	  const coord_t* c(dynamic_cast<const coord_t*>(n.get_children()[9]->get_first_lexem_coordinates()));
	  std::cout << n.get_children()[9]->get_first_lexem_coordinates()->render()
                    << " warning: expression following the DO keyword is not on a new line" << std::endl;
	  show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
	}

	// enddo
	if (not is_on_new_line(ws[n.get_children()[10]->get_first_lexem_id()])) {
	  const coord_t* c(dynamic_cast<const coord_t*>(n.get_children()[10]->get_first_lexem_coordinates()));
	  std::cout << n.get_children()[10]->get_first_lexem_coordinates()->render()
                    << " warning: expression following the ENDDO keyword is not on a new line" << std::endl;
	  show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
	}
	
        n.get_children()[9]->accept(this);
      }
      break;

    case symbol::macro_def:
      n.get_children()[2]->accept(this); // symbol::stmt_list
      break;

    case symbol::if_stmt: {
      if (not is_on_new_line(ws[n.get_children()[2]->get_first_lexem_id() - 1])
	  and n.get_children()[2]->get_first_leaf()->get_symbol() != symbol::comment) {
        const coord_t* c(dynamic_cast<const coord_t*>(n.get_children()[2]->get_first_lexem_coordinates()));
        std::cout << n.get_children()[1]->get_first_lexem_coordinates()->render()
                    << " warning: expression following the THEN keyword is not on a new line" << std::endl;
        show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
      }

      if (n.get_children().size() == 4) {
        if (not is_on_new_line(ws[n.get_children()[3]->get_first_lexem_id()])) {
          const coord_t* c(dynamic_cast<const coord_t*>(n.get_children()[3]->get_first_lexem_coordinates()));
          std::cout << n.get_children()[3]->get_first_lexem_coordinates()->render()
                    << " warning: expression following the ENDIF keyword is not on a new line"
                    << std::endl;
          show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
        }
      } else if (n.get_children().size() == 6) {
        if (not is_on_new_line(ws[n.get_children()[4]->get_first_lexem_id() - 1])
	    and n.get_children()[4]->get_first_leaf()->get_symbol() != symbol::comment) {
          const coord_t* c(dynamic_cast<const coord_t*>(n.get_children()[4]->get_first_lexem_coordinates()));
          std::cout << n.get_children()[4]->get_first_lexem_coordinates()->render()
                    << " warning: expression following the ELSE keyword is not on a new line"
                    << std::endl;
          show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
        }

        if (not is_on_new_line(ws[n.get_children()[5]->get_first_lexem_id()])) {
          const coord_t* c(dynamic_cast<const coord_t*>(n.get_children()[5]->get_first_lexem_coordinates()));
          std::cout << n.get_children()[5]->get_first_lexem_coordinates()->render()
                    << " warning: expression following the ENDIF keyword is not on a new line"
                    << std::endl;
          show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
        }
      }
      
      n.get_children()[2]->accept(this);
      if (n.get_children().size() == 6)
        n.get_children()[4]->accept(this);

    }
      break;

    case symbol::macro_name:
      n.get_children()[0]->accept(this);
      break;

    case symbol::parent_expression:
      check_parent_expression(n);
      break;

    default:
      break;
    }
  }

  virtual void visit(leaf& l) override {
    switch (l.get_symbol()) {
    case symbol::comment: {
      const coord_t* c(dynamic_cast<const coord_t*>(l.get_lexem_coordinates()));
      if (   (l.get_id() == 1 and     is_on_new_line(ws[l.get_id() - 1]) and is_indented(ws[l.get_id() - 1]))
          or (l.get_id() == 1 and not is_on_new_line(ws[l.get_id() - 1]) and not ws[l.get_id() - 1].empty())
          or (l.get_id() > 1 and is_on_new_line(ws[l.get_id() - 1]) and is_indented(ws[l.get_id() - 1]))) {
        std::cout << l.get_lexem_coordinates()->render()
                  << " warning: comment is indented."
                  << std::endl;
        show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
      }
    }
      break;
      
    case symbol::visual_comment: {
      const coord_t* c(dynamic_cast<const coord_t*>(l.get_lexem_coordinates()));
      if (l.get_id() > 1 and not is_on_new_line(ws[l.get_id() - 1])) {
        std::cout << l.get_lexem_coordinates()->render()
                  << " warning: visual comment is not on a new line."
                  << std::endl;
        show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
      }
      if (   (l.get_id() == 1 and     is_on_new_line(ws[l.get_id() - 1]) and is_indented(ws[l.get_id() - 1]))
          or (l.get_id() == 1 and not is_on_new_line(ws[l.get_id() - 1]) and not ws[l.get_id() - 1].empty())
          or (l.get_id() > 1 and is_on_new_line(ws[l.get_id() - 1]) and is_indented(ws[l.get_id() - 1]))) {
        std::cout << l.get_lexem_coordinates()->render()
                  << " warning: visual comment is indented."
                  << std::endl;
        show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
      }
    }
      break;
    case symbol::shell_escape: {
      const coord_t* c(dynamic_cast<const coord_t*>(l.get_lexem_coordinates()));
	
      if (l.get_id() > 1 and not is_on_new_line(ws[l.get_id() - 1])) {
	std::cout << l.get_lexem_coordinates()->render()
                  << "warning: shell escape is not on a new line."
                  << std::endl;
        show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
      }
      if (   (l.get_id() == 1 and     is_on_new_line(ws[l.get_id() - 1]) and is_indented(ws[l.get_id() - 1]))
          or (l.get_id() == 1 and not is_on_new_line(ws[l.get_id() - 1]) and not ws[l.get_id() - 1].empty())
	  or (l.get_id() > 1 and is_on_new_line(ws[l.get_id() - 1]) and is_indented(ws[l.get_id() - 1]))) {
        std::cout << l.get_lexem_coordinates()->render()
                  << " warning: shell escape is indented."
                  << std::endl;
        show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
      }
    }
      break;

    default:
      break;
    }
  }

private:
  const std::vector<std::string>& ws;

  static bool is_on_new_line(const std::string& ws) {
    return ws.find('\n') != std::string::npos;
  }

  static bool is_indented(const std::string& ws) {
    const std::string::size_type nl_position(ws.rfind('\n'));
    return nl_position != ws.size() - 1 and nl_position != std::string::npos;
  }

  void check_parent_expression(node& n) const {
    std::size_t
      open_parent_id(n.get_first_lexem_id()),
      close_parent_id(n.get_last_lexem_id());

    if(not ws[open_parent_id].empty()) {
      const coord_t* c(dynamic_cast<const coord_t*>(n.get_first_lexem_coordinates()));
      std::cout << c->render()
                << " warning: opening parenthese is followed by white space."
                << std::endl;
      show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
    }

    if(not ws[close_parent_id - 1].empty()) {
      const coord_t* c(dynamic_cast<const coord_t*>(n.get_last_lexem_coordinates()));
      std::cout << c->render()
                << " warning: closing parenthese is preceded by white space."
                << std::endl;
      show_coordinates_in_file(c->get_filename(), c->get_line(), c->get_column());
    }
  }

  bool check_white_spaces_in_range(std::size_t first, std::size_t last) const {
    for (std::size_t i(first); i < last; ++i)
      if (not ws[i].empty())
        return false;
    return true;
  }
};

void check_white_spaces(basic_node* tree,
                        const std::vector<std::string>& ws) {
  white_spaces_checker checker(ws);
  tree->accept(&checker);
}


class basic_ast_printer {
public:
  basic_ast_printer(std::ostream& stream,
		    const std::vector<std::string>& ws)
    : stream(stream), white_spaces(ws) {}
  
  virtual ~basic_ast_printer() {}

  virtual void print(leaf& l) = 0;
  
protected:
  std::ostream& stream;
  const std::vector<std::string>& white_spaces;
};

class default_ast_printer: public basic_ast_printer {
public:
  default_ast_printer(std::ostream& stream,
		      const std::vector<std::string>& ws,
                      std::size_t indentation)
    : basic_ast_printer(stream, ws), indentation(indentation) {}
  
  virtual ~default_ast_printer() {}

  virtual void print(leaf& l) {
    switch (l.get_symbol()) {
    case symbol::comment:
    case symbol::visual_comment:
    case symbol::shell_escape: {
      const std::size_t lexem_id(l.get_id()-1);
      const std::size_t newlines(std::count(this->white_spaces[lexem_id].begin(),
					    this->white_spaces[lexem_id].end(),'\n'));
      
      basic_ast_printer::stream
	<< std::string(newlines > 1 ? newlines : 1, '\n')
	<< l.get_value();
    }
      break;
      
      
    default:
      basic_ast_printer::stream
	<< reindent(basic_ast_printer::white_spaces[l.get_id() - 1])
	<< l.get_value();
      break;
    }
  }

private:
  std::size_t indentation;

  std::string reindent(std::string ws) {
    std::string::size_type nl_position(ws.rfind('\n'));
    if (nl_position == std::string::npos) {
      return ws;
    } else {
      return ws.replace(nl_position + 1, ws.size() - nl_position - 2,
                        std::string(indentation, ' '));
    }
  }
};

class reformat_printer: public basic_visitor {
public:
  reformat_printer(std::ostream& stream,
		   const std::vector<std::string>& white_spaces)
    : stream(stream), white_spaces(white_spaces), indentation(0) {
    printers.push_back(new default_ast_printer(stream, white_spaces, indentation));
  }
  
  virtual ~reformat_printer() {}

  virtual void visit(node& n) override {
    switch (n.get_symbol()) {
    case symbol::start:
    case symbol::stmt_list:
    case symbol::stmt:
    case symbol::input:
    case symbol::macro_call:
    case symbol::macro_name:
    case symbol::macro_arg_list:
    case symbol::macro_arg:
    case symbol::parent_expression:
    case symbol::expression:
    case symbol::term:
    case symbol::factor:
    case symbol::expression_list:
    case symbol::parameter_list:
    case symbol::function_call:
    case symbol::macro_file:
    case symbol::if_clause:
      for (auto child: n.get_children())
        child->accept(this);
      break;

    case symbol::macro_def:
      for (std::size_t i(0); i < 2; ++i)
        n.get_children()[i]->accept(this);

      indent();

      for (std::size_t i(2); i < 4; ++i)
        n.get_children()[i]->accept(this);

      deindent();

      for (std::size_t i(4); i < 6; ++i)
        n.get_children()[i]->accept(this);
      break;

    case symbol::if_stmt:
      for (std::size_t i(0); i < 2; ++i)
        n.get_children()[i]->accept(this);

      indent();

      if (n.get_children().size() == 4) {
        n.get_children()[2]->accept(this);

        deindent();

        n.get_children()[3]->accept(this);
      } else {
        n.get_children()[2]->accept(this);

        deindent();

        n.get_children()[3]->accept(this);

        indent();

        n.get_children()[4]->accept(this);

        deindent();

        n.get_children()[5]->accept(this);
      }
      break;

    case symbol::for_stmt:
      if (n.get_children().size() == 9) {
        for (std::size_t i(0); i < 7; ++i)
          n.get_children()[i]->accept(this);

        indent();

        n.get_children()[7]->accept(this);

        deindent();

        n.get_children()[8]->accept(this);
      } else {
        for (std::size_t i(0); i < 9; ++i)
          n.get_children()[i]->accept(this);

        indent();

        n.get_children()[9]->accept(this);

        deindent();

        n.get_children()[10]->accept(this);
      }

      break;

    default:
      throw std::string("this should not happen: unhandled non terminal symbol");
      break;
    }
  }

  virtual void visit(leaf& l) override {
    switch (l.get_symbol()) {
    case symbol::at:
    case symbol::if_kw:
    case symbol::if_def_kw:
    case symbol::then_kw:
    case symbol::else_kw:
    case symbol::endif_kw:
    case symbol::for_kw:
    case symbol::to_kw:
    case symbol::step_kw:
    case symbol::do_kw:
    case symbol::enddo_kw:
    case symbol::endmacro_kw:
    case symbol::defmacro_kw:
    case symbol::enddefmacro_kw:
    case symbol::inline_macro_name:
    case symbol::local_macro_name:
    case symbol::global_macro_name:
    case symbol::comment:
    case symbol::visual_comment:
    case symbol::shell_escape:
    case symbol::fp_number:
    case symbol::identifier:
    case symbol::plus:
    case symbol::minus:
    case symbol::mult:
    case symbol::div:
    case symbol::equal:
    case symbol::percent:
    case symbol::lp:
    case symbol::rp:
    case symbol::lb:
    case symbol::rb:
    case symbol::literal_string:
    case symbol::semicolon:
    case symbol::comma:
      printers.back()->print(l);
      break;
      
    default:
      throw std::string("this should not happen: unhandled terminal symbol");
      break;
    }
  }

private:
  std::ostream& stream;
  const std::vector<std::string>& white_spaces;
  std::vector<basic_ast_printer*> printers;
  std::size_t indentation;

  void indent() {
    indentation += 2;
    printers.push_back(new default_ast_printer(stream, white_spaces, indentation));
  }

  void deindent() {
    delete printers.back();
    printers.pop_back();
    indentation -= 2;
  }
};

void reformat(basic_node* tree,
	      const std::vector<std::string>& white_spaces,
	      std::ostream& stream) {
  reformat_printer printer(stream, white_spaces);
  tree->accept(&printer);
}


#endif /* SYNTAX_CHECKERS_H */
