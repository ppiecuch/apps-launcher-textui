#ifndef CHAISCRIPT_LIB

#include "chaiscript/chaiscript.hpp"

#include <memory>

std::unique_ptr<chaiscript::parser::ChaiScript_Parser_Base> create_chaiscript_parser() {
  return std::make_unique<chaiscript::parser::ChaiScript_Parser<chaiscript::eval::Noop_Tracer, chaiscript::optimizer::Optimizer_Default>>();
}

std::shared_ptr<chaiscript::Module> create_chaiscript_stdlib() {
  return chaiscript::Std_Lib::library();
}

#endif // CHAISCRIPT_LIB
