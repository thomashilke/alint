#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

class node;
class leaf;

class basic_visitor {
public:
  virtual ~basic_visitor() {}
  virtual void visit(node&) = 0;
  virtual void visit(leaf&) = 0;
};

class basic_node {
public:
  basic_node(symbol s): s(s) {}
  
  virtual ~basic_node() {}
  virtual void show(std::ostream& stream, unsigned int level = 0) const = 0;
  virtual void accept(basic_visitor* v) = 0;

  symbol get_symbol() const {
    return s;
  }
  
protected:
  symbol s;
};


class node: public basic_node {
public:
  template<typename iterator_type>
  node(symbol s, iterator_type begin, iterator_type end): basic_node(s), children(begin, end) {}

  void show(std::ostream& stream, unsigned int level) const {
    stream << std::string(level, ' ') << s << std::endl;
    for (const auto& c: children)
      c->show(stream, level + 2);
  }

  virtual void accept(basic_visitor* v) {
    v->visit(*this);
  }

  const std::vector<basic_node*>& get_children() const {
    return children;
  }
  
private:
  std::vector<basic_node*> children;
};


class leaf: public basic_node {
public:
  leaf(symbol s, const std::string& v): basic_node(s), value(v) {}

  void show(std::ostream& stream, unsigned int level) const {
    stream << std::string(level, ' ') << s <<" (" << value << ")" << std::endl;
  }

  virtual void accept(basic_visitor* v) {
    v->visit(*this);
  }

  const std::string& get_value() const {
    return value;
  }
  
private:
  std::string value;
};

#endif /* SYNTAX_TREE_H */
