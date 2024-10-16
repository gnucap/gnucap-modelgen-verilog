/*                               -*- C++ -*-
 * Copyright (C) 2003 Albert Davis
 *               2023 Felix Salfelder
 *
 * This file is part of "Gnucap", the Gnu Circuit Analysis Package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *------------------------------------------------------------------
 * Reconstructs an infix expression from the RPN.
 *  ===  slightly modified to deal with SYMBOL_ and PARLIST_  ===
 */
#include "mg_token.h"
#include "mg_.h"
/*--------------------------------------------------------------------------*/
static void dump_token(Token const* t, std::ostream& out)
{
  assert(t);
  Expression_ e;
  e.push_back(const_cast<Token*>(t));
  e.dump(out);
  e.pop_back();
}
// void Token::dump(std::ostream& out)const
// {itested();
//   out << _name << ' ';
// }
/*--------------------------------------------------------------------------*/
void Expression_::dump(std::ostream& out)const
{
  std::vector<const Token*> locals; // a way of faking garbage collection.
  std::vector<const Token*> stack;  // actually use this
  // The _list is the expression in RPN.
  // Un-parse it -- back to infix.
  for (const_iterator i = begin(); i != end(); ++i) {
//    trace1("Expression_::dump", (*i)->name());
    if (dynamic_cast<const Token_STOP*>(*i)) { untested();
      stack.push_back(*i);
    }else if (dynamic_cast<const Token_PARLIST*>(*i)
	&& dynamic_cast<const Expression*>((*i)->data())) { untested();
      auto de = prechecked_cast<const Expression*>((*i)->data());
      std::stringstream o;
      o << "(";
      de->dump(o);
      o << ")";
      Token* t = new Token_PARLIST(o.str());
      locals.push_back(t);
      stack.push_back(t);
    }else if (auto tarr = dynamic_cast<const Token_ARRAY_*>(*i)){
      auto de = tarr->args();
      std::stringstream o;
      if(tarr->name() == "}"){
	o << "'{";
      }else{ untested();
	o << "(";
      }
      de->dump(o);
      o << tarr->name();
      Token* t = new Token_SYMBOL(o.str());
      locals.push_back(t);
      stack.push_back(t);
    }else if (auto pl = dynamic_cast<const Token_PARLIST_*>(*i)) { untested();
      unreachable(); // PARLIST_ absorbed into CALL or so
      auto de = pl->args();
      std::stringstream o;
      if(pl->name() == "}"){ untested();
	o << "'{";
      }else{ untested();
	o << "(";
      }
      de->dump(o);
      o << pl->name();
      Token* t = new Token_PARLIST(o.str());
      locals.push_back(t);
      stack.push_back(t);
    }else if (dynamic_cast<const Token_PARLIST*>(*i)) { untested();
      // pop*n  push
      bool been_here = false;
      std::string tmp(")");
      for (;;) { untested();
	if (stack.empty()) {untested();
	  throw Exception("bad expression");
	}else{ untested();
	}
	const Token* t = stack.back();
	stack.pop_back();
	if (dynamic_cast<const Token_STOP*>(t)) { untested();
	  tmp = "(" + tmp;
	  break;
	}else if (dynamic_cast<const Token_SYMBOL*>(t)
	      ||  dynamic_cast<const Token_CONSTANT*>(t)) { untested();
	  if (been_here) { untested();
	    tmp = ", " + tmp;
	  }else{ untested();
	    been_here = true;
	  }
	  tmp = t->full_name() + tmp;
	}else{ untested();
	  unreachable();
	}
      }
      Token* t = new Token_PARLIST(tmp);
      locals.push_back(t);
      stack.push_back(t);
    }else if (dynamic_cast<const Token_CONSTANT*>(*i)|| dynamic_cast<const Token_SYMBOL*>(*i)) {
      if (auto call = dynamic_cast<const Token_CALL*>(*i)) {
	if (auto args = dynamic_cast<const Expression_* /*_?*/>(call->args())) {
	  std::stringstream tmp;
	  tmp << (**i).name() << '(';
	  args->dump(tmp);
	  tmp << ')';
	  Token* n = new Token_SYMBOL(tmp.str(), "");

//	  stack.pop_back();
	  locals.push_back(n);
	  stack.push_back(n);
	}else{
	  stack.push_back(*i);
	}
      }else if (!stack.empty() && (dynamic_cast<const Token_PARLIST*>(stack.back()))) { untested();
	// has parameters (table or function)
	// pop op push
	const Token* t1 = stack.back();
	stack.pop_back();
	Token* t = new Token_SYMBOL((**i).name() + t1->full_name());
	locals.push_back(t);
	stack.push_back(t);
      }else{
	// has no parameters (scalar)
	stack.push_back(*i);
      }
    }else if (auto b = dynamic_cast<const Token_BINOP_*>(*i)) {
      const Token* t2 = b->op2();
      if(t2){
	std::stringstream s2;
	dump_token(t2, s2);
	const Token* t1 = b->op1();
	std::stringstream s1;
	dump_token(t1, s1);
	std::string tmp('(' + s1.str() + ' ' + (**i).name() + ' ' + s2.str() + ')');
	Token* t = new Token_SYMBOL(tmp, "");
	locals.push_back(t);
	stack.push_back(t);
      }else{ untested();
	// upstream
	assert(!stack.empty());
	const Token* bt = stack.back();
	stack.pop_back();
	assert(!stack.empty());
	const Token* t1 = stack.back();
	stack.pop_back();
	std::string tmp('(' + t1->full_name() + ' ' + (**i).name() + ' ' + bt->full_name() + ')');
	Token* t = new Token_SYMBOL(tmp, "");
	locals.push_back(t);
	stack.push_back(t);
      }
    }else if (dynamic_cast<const Token_BINOP*>(*i)) { untested();
      unreachable();
      // pop pop op push
      assert(!stack.empty());
      const Token* t2 = stack.back();
      stack.pop_back();
      assert(!stack.empty());
      const Token* t1 = stack.back();
      stack.pop_back();
      std::string tmp('(' + t1->full_name() + ' ' + (**i).name() + ' ' + t2->full_name() + ')');
      Token* t = new Token_SYMBOL(tmp, "");
      locals.push_back(t);
      stack.push_back(t);
    }else if (auto u = dynamic_cast<const Token_UNARY_*>(*i)) {
      // pop op push
      std::string fn;
      if(u->op1()){
	std::stringstream tmp;
	dump_token(u->op1(), tmp);
	fn = tmp.str();
      }else if(stack.empty()){ untested();
	fn = "BUG";
	unreachable();
      }else{ untested();
	const Token* t1 = stack.back();
	stack.pop_back();
	fn = t1->full_name();
      }
      std::string tmp('(' + (**i).name() + ' ' + fn + ')');
      Token* t = new Token_SYMBOL(tmp, "");
      locals.push_back(t);
      stack.push_back(t);
    }else if (auto t = dynamic_cast<const Token_TERNARY_*>(*i)) {
      std::stringstream tmp;
      tmp << "((";
      if(t->cond()){
	dump_token(t->cond(), tmp);
      }else if(stack.empty()){ untested();
	unreachable();
      }else{ untested();
	const Token* cond = stack.back();
	unreachable();
	tmp << cond->full_name();
	stack.pop_back();
      }

      tmp << ")? (";
      assert(t->true_part());
      t->true_part()->dump(tmp);
      tmp << "):(";
      assert(t->false_part());
      t->false_part()->dump(tmp);
      tmp << "))";
      std::string s = tmp.str();

      Token* n = new Token_SYMBOL(s, "");
      trace2("", locals.size(), stack.size());
      trace1("", tmp.str());
      locals.push_back(n);
      stack.push_back(n);
    }else{ untested();
      unreachable();
      assert(!dynamic_cast<const Token_TERNARY*>(*i));
      assert(!dynamic_cast<const Token_UNARY*>(*i));
    }
  }
  std::string comma;
  for(auto const& i : stack){
    out << comma << i->full_name();
    comma = ", ";
  }
  while (!locals.empty()) {
    delete locals.back();
    locals.pop_back();
  }
} // Expression_::dump
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
