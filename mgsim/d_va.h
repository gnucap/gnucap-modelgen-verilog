/*                                 -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 *               2023 Felix Salfelder
 * Author: Albert Davis <aldavis@gnu.org>
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
 * DEV_CPOLY_G
 * number of nodes = 2*n_ports
 * number of val, ov = n_ports+1
 * val[0] is the constant part, val[1] is self admittance,
 *   val[2+] are transadmittances, up to n_ports
 * node[0] and node[1] are the output.
 * node[2] up are inputs.
 * node[2*i] and node[2*i+1] correspond to val[i+1]
 */
#include "e_va.h" // TODO
#include <globals.h>
#include <e_elemnt.h>
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class DEV_CPOLY_G : public ELEMENT {
protected:
  double*  _values{NULL};
  double*  _old_values{NULL};
  double*  _m0_{NULL};
  double*  _m1_{NULL};
  int	   _n_ports{0};
  double   _time;
  std::vector<std::string> _current_port_names;
  std::vector<ELEMENT const*> _input;
#ifndef NDEBUG
  int _reason{0};
#endif
  bool _self_is_current{false};
protected:
  explicit DEV_CPOLY_G(const DEV_CPOLY_G& p);
public:
  explicit DEV_CPOLY_G();
  ~DEV_CPOLY_G();
protected: // override virtual
  char	   id_letter()const override	{ untested();unreachable(); return '\0';}
  std::string value_name()const override{ untested();incomplete(); return "";}
  std::string dev_type()const override	{ untested();unreachable(); return "cpoly_g";}
  int	   max_nodes()const override	{return net_nodes();}
  int	   min_nodes()const override	{return net_nodes();}
  int	   matrix_nodes()const override	{return _n_ports*2;}
  int	   net_nodes()const override	{return _n_ports*2;}
  CARD*	   clone()const override	{ untested();return new DEV_CPOLY_G(*this);}
  void	   tr_iwant_matrix()override	{ untested();tr_iwant_matrix_extended();}
  bool	   do_tr()override;
  void	   tr_load()override;
  void	   tr_unload()override;
  double   tr_involts()const override	{ untested();unreachable(); return NOT_VALID;}
  double   tr_involts_limited()const override { untested();unreachable(); return NOT_VALID;}
  double   tr_amps()const override;
  void	   ac_iwant_matrix()override	{ untested();ac_iwant_matrix_extended();}
  void	   ac_load()override;
  COMPLEX  ac_involts()const override	{itested(); return NOT_VALID;}
  COMPLEX  ac_amps()const override	{itested(); return NOT_VALID;}

  bool has_iv_probe()const override { untested();return true;}
  void expand_last()override;
  void expand_current_port(size_t i);

  void set_current_port_by_index(int i, const std::string& s) override {
    if(i==0){
      _self_is_current = true;
    }else if(i<=int(_current_port_names.size())){
      _current_port_names[i-1] = s;
    }else{ untested();
      throw Exception_Too_Many(i, int(_current_port_names.size()), 0);
    }
  }
  std::string port_name(int)const override {untested();
    incomplete();
    unreachable();
    return "";
  }
  double tr_probe_num(const std::string& x)const override;
public:
  void set_parameters(const std::string& Label, CARD* Parent,
		      COMMON_COMPONENT* Common, double Value,
		      int state_count, double state[],
		      int node_count, const node_t nodes[])override;
  //		      const double* inputs[]=0);
protected:
  double abstol() const{ untested();
    auto cv = prechecked_cast<COMMON_VASRC const*>(common());
    assert(cv); // TODO: give feedback
    return cv->flow_abstol();
  }
  bool do_tr_con_chk_and_q();
}; // DEV_CPOLY_G;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
DEV_CPOLY_G::DEV_CPOLY_G(const DEV_CPOLY_G& p)
  :ELEMENT(p),
   _values(NULL),
   _old_values(NULL),
   _n_ports(p._n_ports),
   _time(NOT_VALID)
{
  // not really a copy .. only valid to copy a default
  // too lazy to do it right, and that's all that is being used
  // to do it correctly requires a deep copy
  // just filling in defaults is better than a shallow copy, hence this:
  assert(!p._values);
  assert(!p._old_values);
  assert(p._n_ports == 0);
}
/*--------------------------------------------------------------------------*/
DEV_CPOLY_G::DEV_CPOLY_G()
  :ELEMENT(),
   _values(NULL),
   _old_values(NULL),
   _n_ports(0),
   _time(NOT_VALID)
{
}
/*--------------------------------------------------------------------------*/
void DEV_CPOLY_G::expand_last()
{
  ELEMENT::expand_last();
  for(size_t i=0; i<_current_port_names.size(); ++i){
    expand_current_port(i);
  }
}
/*--------------------------------------------------------------------------*/
void DEV_CPOLY_G::expand_current_port(size_t i)
{
  std::string const& input_label = _current_port_names[i];
  ELEMENT const* input = _input[i];
//  node_t* n = _n + net_nodes() + 2*(i-_input.size()) - IN1;

  assert (input_label != "");
  CARD const* e = find_in_my_scope(input_label);
  input = dynamic_cast<const ELEMENT*>(e);

  if (!e) {untested();
    throw Exception(long_label() + ": " + input_label + " does not exist");
  }else if (!input) {untested();
    throw Exception(long_label() + ": " + input_label + " cannot be used as current probe");
  }else if (input->subckt()) {untested();
    throw Exception(long_label() + ": " + input_label
		    + " has a subckt, cannot be used as current probe");
  }else if (input->has_inode()) {untested();
    incomplete(); // wrong N1
    _n[IN1] = input->n_(IN1);
    _n[IN2].set_to_ground(this);
  }else if (input->has_iv_probe()) {
    size_t IN1 = net_nodes() - 2*_current_port_names.size() + 2*i;
    trace4("flow ecp", i, IN1, net_nodes(), _current_port_names.size());
    _n[IN1] = input->n_(OUT1);
    _n[IN1+1] = input->n_(OUT2);
  }else{ untested();
    throw Exception(long_label() + ": " + input_label + " cannot be used as current probe");
  }
}
/*--------------------------------------------------------------------------*/
DEV_CPOLY_G::~DEV_CPOLY_G()
{
  delete [] _old_values;
  delete [] _m0_;
  delete [] _m1_;
  if (net_nodes() > NODES_PER_BRANCH) {
    delete [] _n;
  }else{
    // it is part of a base class
  }
}
/*--------------------------------------------------------------------------*/
bool DEV_CPOLY_G::do_tr_con_chk_and_q()
{ untested();
  q_load();
#ifndef NDEBUG
  _reason = 0;
#endif

  assert(_old_values);
  set_converged(conchk(_time, _sim->_time0));
  _time = _sim->_time0;
  if(converged()){ untested();
    set_converged(conchk(_old_values[0], _values[0], abstol()));
#ifndef NDEBUG
    if(!converged()){ untested();
      _reason = 1;
      trace4("not converged", long_label(), _old_values[0], _values[0], abstol());
    }else{ untested();
    }
#endif
  }else{ untested();
  }
  for (int i=1; converged() && i<=_n_ports; ++i) { untested();
    // if(i==0){ untested();
    // }else if(_old_values[i] * _values[i] < 0.){ untested();
    //   trace4("sign change", long_label(), i, _old_values[i], _values[i]);
    // }else{ itested();
    // }
    set_converged(conchk(_old_values[i], _values[i] /*, 0.?*/ ));
#ifndef NDEBUG
    if(!converged()){ untested();
      _reason = i;
      trace4("not converged", long_label(), i, _old_values[i], _values[i]);
    }else{ untested();
    }
#endif
  }
  return converged();
}
/*--------------------------------------------------------------------------*/
bool DEV_CPOLY_G::do_tr()
{ untested();
  assert(_values);
  assert(!_loss0);
  trace3("DCG::do_tr", long_label(), _values[0], _values[1]);

  _m0 = CPOLY1(0., _values[0], _values[1]);
  return do_tr_con_chk_and_q();
}
/*--------------------------------------------------------------------------*/
void DEV_CPOLY_G::tr_load()
{
  tr_load_passive();
  _old_values[0] = _values[0];
  _old_values[1] = _values[1];
  for (int i=2; i<=_n_ports; ++i) { untested();
    trace4("tr_load", long_label(), i, _values[i], _old_values[i]);
    tr_load_extended(_n[OUT1], _n[OUT2], _n[2*i-2], _n[2*i-1], &(_values[i]), &(_old_values[i]));
  }
}
/*--------------------------------------------------------------------------*/
void DEV_CPOLY_G::tr_unload()
{ untested();
  std::fill_n(_values, _n_ports+1, 0.);
  _m0.c0 = _m0.c1 = 0.;
  _sim->mark_inc_mode_bad();
  tr_load();
}
/*--------------------------------------------------------------------------*/
double DEV_CPOLY_G::tr_amps()const
{ untested();
  double amps = _m0.c0;
  for (int i=1; i<=_n_ports; ++i) { untested();
    amps += dn_diff(_n[2*i-2].v0(), _n[2*i-1].v0()) * _values[i];
  }
  return amps;
}
/*--------------------------------------------------------------------------*/
void DEV_CPOLY_G::ac_load()
{ untested();
  _acg = _values[1];
  ac_load_passive();
  for (int i=2; i<=_n_ports; ++i) { untested();
    trace4("acload", long_label(), i, _values[i], _old_values[i]);
    ac_load_extended(_n[OUT1], _n[OUT2], _n[2*i-2], _n[2*i-1], _values[i]);
  }
}
/*--------------------------------------------------------------------------*/
/* set: set parameters, used in model building
 */
void DEV_CPOLY_G::set_parameters(const std::string& Label, CARD *Owner,
				 COMMON_COMPONENT *Common, double Value,
				 int n_states, double states[],
				 int n_nodes, const node_t nodes[])
  //				 const double* inputs[])
{ untested();
  bool first_time = (net_nodes() == 0);
//  bool first_time = _sim->is_first_expand();

  set_label(Label);
  trace4("DEV_CPOLY_G::set_parameters", long_label(), n_nodes, n_states, first_time);
  set_owner(Owner);
  set_value(Value);
  attach_common(Common);

  if (first_time) { untested();
    _current_port_names.resize(n_states - 1 - n_nodes/2);
    _input.resize(n_states - 1 - n_nodes/2);
    _n_ports = n_states-1; // set net_nodes
    assert(size_t(_n_ports) == n_nodes/2 + _current_port_names.size());

    assert(!_old_values);
    _old_values = new double[n_states];

    if (net_nodes() > NODES_PER_BRANCH) { untested();
      // allocate a bigger node list
      _n = new node_t[net_nodes()];
    }else{ untested();
      // use the default node list, already set
    }      
  }else{ untested();
    assert(_n_ports == n_states-1);
    assert(_old_values);
    // assert(net_nodes() == n_nodes); // current ports?
  }

  _values = states;
  std::fill_n(_values, n_states, 0.);
  std::fill_n(_old_values, n_states, 0.);
  assert(n_nodes <= net_nodes());
  notstd::copy_n(nodes, n_nodes, _n); // copy more in expand_last
  assert(net_nodes() == _n_ports * 2);
}
/*--------------------------------------------------------------------------*/
double DEV_CPOLY_G::tr_probe_num(const std::string& x)const
{
  CS cmd(CS::_STRING, x);
  if (Umatch(x, "loss ")) {
    return _loss0;
#ifndef NDEBUG
  }else if (Umatch(x, "conv ")) { untested();
    return converged();
  }else if (Umatch(x, "reason ")) { untested();
    return _reason;
#endif
  }else if (cmd.umatch("val")) {
    int nn = cmd.ctoi();
    return _values[nn]; // range??
  }else if (Umatch(x, "abstol ")) { untested();
    return abstol();
  }else{
    return ELEMENT::tr_probe_num(x);
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
