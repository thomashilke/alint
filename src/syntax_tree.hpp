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

  virtual const leaf* get_first_leaf() const = 0;
  virtual const leaf* get_last_leaf() const = 0;
  
  virtual std::size_t get_first_lexem_id() const = 0;
  virtual std::size_t get_last_lexem_id() const = 0;

  virtual const source_coordinate_range* get_first_lexem_coordinates() const = 0;
  virtual const source_coordinate_range* get_last_lexem_coordinates() const = 0;
  
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

  virtual const leaf* get_first_leaf() const {
    return children.front()->get_first_leaf();
  }
  
  virtual const leaf* get_last_leaf() const {
    return children.back()->get_last_leaf();
  }
  
  std::size_t get_first_lexem_id() const {
    return children.front()->get_first_lexem_id();
  }

  std::size_t get_last_lexem_id() const {
    return children.back()->get_last_lexem_id();
  }

  const source_coordinate_range* get_first_lexem_coordinates() const {
    return children.front()->get_first_lexem_coordinates();
  }

  const source_coordinate_range* get_last_lexem_coordinates() const {
    return children.back()->get_last_lexem_coordinates();
  }
  
private:
  std::vector<basic_node*> children;
};


class leaf: public basic_node {
public:
  leaf(symbol s, const std::string& v,
       source_coordinate_range* coord,
       std::size_t lexem_id)
    : basic_node(s), value(v), id(lexem_id), coordinates(coord) {}

  void show(std::ostream& stream, unsigned int level) const {
    stream << std::string(level, ' ') << s <<" (" << value << ", "
           << id << ", "
           << coordinates->render() << ")" << std::endl;
  }

  virtual void accept(basic_visitor* v) {
    v->visit(*this);
  }

  const std::string& get_value() const {
    return value;
  }

  std::size_t get_id() const { return id; }

  virtual const leaf* get_first_leaf() const {
    return this;
  }
  
  virtual const leaf* get_last_leaf() const {
    return this;
  }
  
  std::size_t get_first_lexem_id() const {
    return get_id();
  }

  std::size_t get_last_lexem_id() const {
    return get_id();
  }

  const source_coordinate_range* get_lexem_coordinates() const {
    return coordinates;
  }
  
  const source_coordinate_range* get_first_lexem_coordinates() const {
    return coordinates;
  }

  const source_coordinate_range* get_last_lexem_coordinates() const {
    return coordinates;
  }
  
private:
  std::string value;
  std::size_t id;
  source_coordinate_range* coordinates;
};

#endif /* SYNTAX_TREE_H */
