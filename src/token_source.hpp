#ifndef TOKEN_SOURCE_H
#define TOKEN_SOURCE_H

template<typename token_type>
class token_source {
 public:
  token_source() : src(NULL), current(NULL), dirty(true) {}

  void set_source(regex_lexer<token_type>* s) {
    src = s;
    
  }

  token_type* get() {
    if (dirty)
      current = src->get();
    
    token_type* tmp(current);
    current = NULL;
    dirty = true;
    return tmp;
  }
  
  token_type* peek() {
    if (dirty)
      current = src->get();
    return current;
  }
  
 private:
  regex_lexer<token_type>* src;
  token_type* current;
  bool dirty;
};

#endif /* TOKEN_SOURCE_H */
