#ifndef _OPTIONS_H_
#define _OPTIONS_H_

struct options {
  options() :
    lexing_pass(false),
    parsing_pass(true),
    run_checkers(false),
    show_dependencies(false),
    show_grammar(false),
    verbose(false),
    silent(false),
    reformat_source(false),
    recursive_parse(false) {
    const char* g_m_dir(std::getenv("ALUCELL_GLOBAL_MACRO_DIR"));
    if (g_m_dir)
      global_macro_dir = g_m_dir;
    else
      std::cout << "warning: environment variable ALUCELL_GLOBAL_MACRO_DIR is not set." << std::endl;
    
    const char* l_m_dir(std::getenv("ALUCELL_LOCAL_MACRO_DIR"));
    if (l_m_dir)
      local_macro_dir = l_m_dir;
    else
      std::cout << "warning: environment variable ALUCELL_LOCAL_MACRO_DIR is not set." << std::endl;
  }

  bool lexing_pass;
  bool parsing_pass;
  bool run_checkers;
  bool show_dependencies;
  bool show_grammar;
  bool verbose;
  bool silent;
  bool reformat_source;
  bool recursive_parse;

  std::string global_macro_dir;
  std::string local_macro_dir;
};

#endif /* _OPTIONS_H_ */
