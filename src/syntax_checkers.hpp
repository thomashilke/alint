#ifndef SYNTAX_CHECKERS_H
#define SYNTAX_CHECKERS_H

/*
 *  List of possible checks
 *   - Equal assigment
 *     rhs should be an expression
 *   - Equal-dollar assigment
 *     rhs should be a string literal or an identifier
 *   - Equal-percent assigment
 *     rhs should be an identifier
 */


class do_enddo_checker: public basic_visitor {
public:
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
      if (enddo_guard_value != do_guard_value)
        std::cout << l.get_lexem_coordinates()
                  << " warning: DO \"" << do_guard_value
                  << "\" doesn't match ENDDO \""
                  << enddo_guard_value << "\" guard value"
                  << std::endl;
    } else if (l.get_symbol() == symbol::inline_macro_name) {
      if (inline_macro_name.empty()) {
        inline_macro_name.push_back(l.get_value());
      } else {
        if (inline_macro_name.back() != l.get_value())
          std::cout << l.get_lexem_coordinates()
                    << " warning: MACRO \"" << inline_macro_name.back()
                    << "\" don't match ENDMACRO \""
                    << l.get_value() << "\" guard value."
                    << std::endl;
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
      std::cout << get_input_filename(&n) << std::endl;
      break;

    default:
      break;
    }
  }

  virtual void visit(leaf& l) override {
    switch (l.get_symbol()) {
    case symbol::inline_macro_name:
    case symbol::global_macro_name:
    case symbol::local_macro_name:
      std::cout << l.get_value() << std::endl;
      break;

    default:
      break;
    }
  }
};

void show_input_and_macro_dependencies(basic_node* tree) {
  dependency_extractor extractor;
  tree->accept(&extractor);
}

class white_spaces_checker: public basic_visitor {
public:
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
                                            n.get_children()[3]->get_last_lexem_id()))
          std::cout << n.get_children()[1]->get_first_lexem_coordinates()
                    << " warning: white spaces in the initialisation of the for statement."
                    << std::endl;

         // upper boundary
        if (not check_white_spaces_in_range(n.get_children()[5]->get_first_lexem_id(),
                                            n.get_children()[5]->get_last_lexem_id()))
          std::cout << n.get_children()[5]->get_first_lexem_coordinates()
                    << " warning: white spaces in the stop condition of the for statement."
                    << std::endl;


        
        n.get_children()[7]->accept(this); // symbol::stmt_list
      } else {
        // initial condition
        if (not check_white_spaces_in_range(n.get_children()[1]->get_first_lexem_id(),
                                            n.get_children()[3]->get_last_lexem_id()))
          std::cout << n.get_children()[1]->get_first_lexem_coordinates()
                    << " warning: white spaces in the initialisation of the for statement."
                    << std::endl;
        
         // upper boundary
        if (not check_white_spaces_in_range(n.get_children()[5]->get_first_lexem_id(),
                                            n.get_children()[5]->get_last_lexem_id()))
          std::cout << n.get_children()[5]->get_first_lexem_coordinates()
                    << " warning: white spaces in the stop condition of the for statement."
                    << std::endl;

         // step
        if (not check_white_spaces_in_range(n.get_children()[7]->get_first_lexem_id(),
                                            n.get_children()[7]->get_last_lexem_id()))
          std::cout << n.get_children()[7]->get_first_lexem_coordinates()
                    << " warning: white spaces in the step condition of the for statement."
                    << std::endl;

        n.get_children()[9]->accept(this);
      }
      break;

    case symbol::macro_def:
      n.get_children()[2]->accept(this); // symbol::stmt_list
      break;

    case symbol::if_stmt: {
      if (not is_on_new_line(ws[n.get_children()[2]->get_first_lexem_id() - 1]))
          std::cout << n.get_children()[1]->get_first_lexem_coordinates()
                    << " warning: expression following the THEN keyword is not on a new line" << std::endl;

      if (n.get_children().size() == 4) {
        if (not is_on_new_line(ws[n.get_children()[3]->get_first_lexem_id()]))
          std::cout << n.get_children()[3]->get_first_lexem_coordinates()
                    << " warning: expression following the ENDIF keyword is not on a new line"
                    << std::endl;
      } else if (n.get_children().size() == 6) {
        if (not is_on_new_line(ws[n.get_children()[4]->get_first_lexem_id() - 1]))
          std::cout << n.get_children()[3]->get_first_lexem_coordinates()
                    << " warning: expression following the ELSE keyword is not on a new line"
                    << std::endl;

        if (not is_on_new_line(ws[n.get_children()[5]->get_first_lexem_id()]))
          std::cout << n.get_children()[5]->get_first_lexem_coordinates()
                    << " warning: expression following the ENDIF keyword is not on a new line"
                    << std::endl;
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
    case symbol::comment:
    case symbol::visual_comment:
      if (l.get_id() > 1 and not is_on_new_line(ws[l.get_id() - 1]))
        std::cout << l.get_lexem_coordinates()
                  << " warning: comment or visual comment is not on a new line."
                  << std::endl;
      break;
    case symbol::shell_escape:
      if (l.get_id() > 1 and not is_on_new_line(ws[l.get_id() - 1]))
        std::cout << l.get_lexem_coordinates()
                  << "warning: shell escape is not on a new line."
                  << std::endl;
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

  void check_parent_expression(node& n) const {
    std::size_t
      open_parent_id(n.get_first_lexem_id()),
      close_parent_id(n.get_last_lexem_id());

    if(not ws[open_parent_id].empty())
      std::cout << n.get_first_lexem_coordinates()
                << " warning: opening parenthese is followed by white space."
                << std::endl;

    if(not ws[close_parent_id - 1].empty())
      std::cout << n.get_last_lexem_coordinates()
                << " warning: closing parenthese is preceded by white space."
                << std::endl;
    
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


#endif /* SYNTAX_CHECKERS_H */
