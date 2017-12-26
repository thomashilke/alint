#include <fstream>

#include <cstdlib>

#include <spikes/timer.hpp>

#include <parser/parser.hpp>
#include <lexer/lexer.hpp>
#include "../src/token_source.hpp"

#include "../src/symbol.hpp"
#include "../src/lexer.hpp"
#include "../src/syntax_tree.hpp"
#include "../src/parser.hpp"

template<typename token_source_type>
bool parse_input_with_recovery(lr_parser<typename token_source_type::symbol_type>& parser,
                               cf_grammar<typename token_source_type::symbol_type>& grammar,
                               token_source_type& input) {
  using symbol_type = typename token_source_type::symbol_type;
  
  std::list<unsigned int> state_stack;
  std::list<symbol_type> symbol_stack;
  state_stack.push_back(0);

  while(state_stack.back() != parser.accepting_state) {
    const auto terminal_map_item(parser.terminal_map.find(input.get().symbol));
    if (terminal_map_item == parser.terminal_map.end())
      throw std::string("parse error near ") + input.get().render_coordinates();
    
    const unsigned int terminal_id(terminal_map_item->second);
    const int action(parser.transitions_table[ state_stack.back() ][ terminal_id ]);

    if(action > 0) {  // shift
      state_stack.push_back(action - 1);
      symbol_stack.push_back(input.get().symbol);
      input.next();
    } else if(action < 0) {  // reduce
      const unsigned int production_rule_id(-action-1);
      const unsigned int non_terminal_symbol_id(parser.non_terminal_map[parser.reduce_symbol[production_rule_id]]);

      pop(state_stack, parser.rule_lengths[production_rule_id]);
      state_stack.push_back(parser.goto_table[ state_stack.back() ][ non_terminal_symbol_id ] - 1);
      
      pop(symbol_stack, parser.rule_lengths[production_rule_id]);
      symbol_stack.push_back(parser.reduce_symbol[production_rule_id]);
    } else {
      std::cout << input.get().get_coordinates()->render()
                << ": syntax error, unexpected " << input.get().symbol << std::endl;

      /*
       *  Print the state stack
       */
      /*std::cout << "state stack: ";
      for (const auto& s: state_stack)
        std::cout << s << " ";
        std::cout << std::endl;*/

      /*
       *  Print the symbol stack
       */
      /*std::cout << "symbol stack: ";
      for (const auto& s: symbol_stack)
        std::cout << s << " ";
        std::cout << std::endl;*/
      
      /*
       *  Save the state and symbol stack in a random access structure
       */
      std::vector<unsigned int> states(state_stack.begin(), state_stack.end());
      std::vector<symbol_type> symbols(symbol_stack.begin(), symbol_stack.end());
      
      /*
       *  build maps from symbol ids to symbols.
       */
      std::map<unsigned int, symbol> symbol_id_map;
      for (const auto& i: parser.non_terminal_map)
        symbol_id_map[i.second] = i.first;

      for (const auto& i: parser.terminal_map)
        symbol_id_map[i.second] = i.first;

      for (unsigned int i(states.size()); i > 0; --i) {
        /*if (i == 1)
          std::cout << "available reduction goals following nothing at stack location "
                    << i-1 << " which accepts a "
                    << input.get().symbol << ": ";
        else
          std::cout << "available reduction goals following a "
                    << symbols[i-2]
                    << " at stack location " << i-1 << " which accepts a "
                    << input.get().symbol << ": ";*/

        std::set<symbol> candidate_reduction_goals;
        const std::vector<short>& goto_line(parser.goto_table[states[i-1]]);
        for (const auto& s: parser.non_terminal_map)
          if (goto_line[s.second]) {
            const unsigned int candidate_state(goto_line[s.second] - 1);
            if (parser.transitions_table[candidate_state][terminal_id])
              candidate_reduction_goals.insert(grammar.get_important_goal(s.first));
          }
        /*
         * Display the candidate set
         */
        /*for (const auto& s: candidate_reduction_goals)
          std::cout << s << " ";
          std::cout << std::endl;*/

        
        if (not candidate_reduction_goals.empty()) {
          const symbol_type reduction_goal(*candidate_reduction_goals.begin());
          const unsigned int reduction_goal_id(parser.non_terminal_map[reduction_goal]);
          
          pop(state_stack, states.size() - i);
          state_stack.push_back(parser.goto_table[ state_stack.back() ][ reduction_goal_id ] - 1);
      
          pop(symbol_stack, states.size() - i);
          symbol_stack.push_back(reduction_goal);

          break;
        }
      }
        
      //return false;
    }
  }
  return true;
}


int main(int argc, char** argv) {
  try {
    if (argc != 2)
      throw std::string("please give me a filename.");
    const std::string filename(argv[1]);

    cf_grammar<symbol> g(build_cf_grammar());
    lr_parser<symbol> p(g);

    //p.print(std::cout, g);
    
    alint_token_source tokens;
    tokens.set_file(filename);

    default_error_handler<token_type> handler;
    bool result(parse_input<alint_token_source>(p, g, tokens, handler));
    //bool result(parse_input_with_recovery<alint_token_source>(p, g, tokens));

    std::cout << (result ? "good" : "bad") << std::endl;
  }
  catch (const std::string& e) {
    std::cout << e << std::endl;
  }
  return 0;
}
