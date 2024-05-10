/*                        -*- C++ -*-
 * Copyright (C) 2024 Felix Salfelder
 * Author: Felix Salfelder
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
 * noise
 */
#include <e_elemnt.h>
#include <globals.h>
#include <io_trace.h>
#include <u_xprobe.h>
#include <u_status.h>
#include <u_cardst.h>

/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class COMMON_NOISE : public COMMON_COMPONENT {
  std::string _name; // PARAMETER<string>?
protected:
  COMMON_NOISE(COMMON_NOISE const& p)
    : COMMON_COMPONENT(p), _name(p._name) {}
public:
  COMMON_NOISE(int c) : COMMON_COMPONENT(c) {}
  COMMON_COMPONENT* clone()const override {
    return new COMMON_NOISE(*this);
  }
  std::string name()const override {return "white_noise";}
  std::string noise_id()const {return _name;}
  bool operator==(const COMMON_COMPONENT& x)const override
  {
    const COMMON_NOISE* p = dynamic_cast<const COMMON_NOISE*>(&x);
    bool rv = p
      && _name == p->noise_id()
      && COMMON_COMPONENT::operator==(x);
    if (rv) {
    }else{
    }
    return rv;
  }

  int param_count()const override {
    return 2 + COMMON_COMPONENT::param_count();
  }
  std::string param_name(int I, int j)const override { untested();
    if (j == 0) { untested();
      return param_name(I);
    }else if (I >= COMMON_COMPONENT::param_count()) { untested();
      return "";
    }else{ untested();
      return COMMON_COMPONENT::param_name(I, j);
    }
  }
  std::string param_name(int i)const override {
    int idx = COMMON_NOISE::param_count() - 1 - i;
    if(idx==0){
      return "pwr";
    }else if(idx==1){
      return "name";
    }else{ untested();
      return COMMON_COMPONENT::param_name(i);
    }
  }
  std::string param_value(int i)const override {
    int idx = COMMON_NOISE::param_count() - 1 - i;
    if(idx==0) {
      return _value.string();
    }else if(idx==1) {
      return "\"" + _name + "\"";
    }else{ untested();
      return COMMON_COMPONENT::param_value(i);
    }
  }
  bool param_is_printable(int i)const override {
    int idx = COMMON_NOISE::param_count() - 1 - i;
    if(idx==0) {
      return _value.has_hard_value();
    }else if(idx==1) {
      return _name!="";
    }else{
      return COMMON_COMPONENT::param_is_printable(i);
    }
  }
  int set_param_by_name(std::string N, std::string V)override {
    trace2("CN::spbn", N, V);
    if(N == "name"){
      if(V.size() && V[0]=='"' && V[V.size()-1] == '"'){
	_name = V.substr(1, V.size()-2);
      }else{
	_name = V;
      }
      return 1;
    }else if(N == "pwr"){
      _value = V;
      return 2;
    }else if(N == "$mfactor"){ untested();
      // incomplete, mfactor transition.
      try{ COMMON_COMPONENT::set_param_by_name("$mfactor", V); }catch(Exception const&){}
      return 2+COMMON_COMPONENT::set_param_by_name("m", V);
    }else{ untested();
      throw Exception_No_Match(N);
    }
  }
  void precalc_last(CARD_LIST const* scope)override {
    COMMON_COMPONENT::precalc_last(scope);
    _value.e_val(1., scope);
    trace2("pl", _value, _value.string());
  }
public:
  virtual double do_noise(ELEMENT const* e, std::string const&)const;
}; // COMMON_NOISE
COMMON_NOISE ccn(CC_STATIC);
/*--------------------------------------------------------------------------*/
class DEV_NOISE : public ELEMENT /* COMPONENT? */ {
  double* _values{NULL};
 // std::string _name; // PARAMETER<string>?
public:
  explicit DEV_NOISE(COMMON_COMPONENT* p) : ELEMENT(p){ }
  explicit DEV_NOISE(const DEV_NOISE& p) : ELEMENT(p){ }
  explicit DEV_NOISE() : ELEMENT() { }
public: // make noise
  double value()const{
    if(_values){
      return *_values;
    }else{
      return ELEMENT::value(); // common->_value?
    }
  }
  XPROBE ac_probe_ext(std::string const&)const override;
  double noise_num(std::string const&n)const override {
//    if(n=="loss"){ untested();
//      return _loss0;
//    }
    auto cc = prechecked_cast<COMMON_NOISE const*>(common());
    assert(cc);
    double ev = cc->do_noise(this, n);
    trace4("DEV_NOISE::noise_num", long_label(), mfactor(), ev, _loss0);
    if(_values){
      return mfactor() * _values[1] * ev;
    }else if(_loss0){ untested();
      return mfactor() * ev;
    }else{
      return mfactor() * ev;
    }
  }
private: // override virtual
  void set_parameters(const std::string& Label, CARD *Owner,
		      COMMON_COMPONENT *Common, double Value,
		      int n_states, double states[],
		      int n_nodes, const node_t nodes[])override;
  void precalc_last()override;
  char	   id_letter()const override{return '\0';}
  std::string value_name()const override{return "";}
  std::string dev_type()const override{
    assert(common());
    return common()->name();
  }
  int	max_nodes()const override {return 2;}
  int	min_nodes()const override {return 2;}
  int	matrix_nodes()const override {return 2;}
  int	net_nodes()const override {return 2;}
  bool	has_iv_probe()const override {return false;}
  bool	use_obsolete_callback_parse()const override {return false;}
  CARD*	clone()const override {return new DEV_NOISE(*this);}
  void tr_iwant_matrix()override{}
  void tr_begin()override {}
  void tr_load()override {}
  void tr_unload()override {}
  void ac_iwant_matrix()override {}
  void ac_begin()override {}
  void dc_advance()override {}
  void tr_advance()override {}
  void tr_regress()override {}
  double tr_amps()const override { return 0.;}
  bool tr_needs_eval()const override {return false;}
  TIME_PAIR tr_review()override {return TIME_PAIR();}
  bool do_tr()override {return true;}
  void do_ac()override {}
  void ac_load()override {
    if(_loss0){
      trace1("AC_LOAD", value());
      incomplete();
      _acg = 100;//value(); // 1; // _vy0[1] * _sim->_jomega; BUG. value?
 //     ac_load_source();
      ac_load_shunt(); // 4 pt +- loss
    }else{
      ac_load_shunt(); // 4 pt +- loss
    }
  }
private: // purely virtual in ELEMENT
  double tr_involts()const override {return tr_outvolts();}
  double tr_involts_limited()const override { return 0.; }
  COMPLEX  ac_involts()const override {return ac_outvolts();}

private:
  std::string port_name(int i)const override{
    assert(i >= 0);
    assert(i < 2);
    static std::string names[] = {"p", "n"};
    return names[i];
  }
};
DEV_NOISE ww(&ccn);
static DISPATCHER<CARD>::INSTALL dwn(&device_dispatcher, "white_noise|va_white_noise", &ww);
/*--------------------------------------------------------------------------*/
class COMMON_FLICK : public COMMON_NOISE {
protected:
  PARAMETER<double> _e;
  COMMON_FLICK(COMMON_FLICK const& c) : COMMON_NOISE(c), _e(c._e) {}
public:
  COMMON_FLICK(int c) : COMMON_NOISE(c) {}
  COMMON_NOISE* clone()const override {return new COMMON_FLICK(*this);}
  bool operator==(const COMMON_COMPONENT& x)const override {
    const COMMON_FLICK* p = dynamic_cast<const COMMON_FLICK*>(&x);
    bool rv = p
      && _e == p->_e
      && COMMON_NOISE::operator==(x);
    if (rv) {
    }else{
    }
    return rv;
  }
  std::string name()const override {return "flicker_noise";}
  int param_count()const override {
    return 1 + COMMON_NOISE::param_count();
  }
  std::string param_name(int I, int j)const override { untested();
    if (j == 0) { untested();
      return param_name(I);
    }else if (I >= COMMON_NOISE::param_count()) { untested();
      return "";
    }else{ untested();
      return COMMON_NOISE::param_name(I, j);
    }
  }
  std::string param_name(int i)const override {
    int idx = COMMON_FLICK::param_count() - 1 - i;
    if(idx==0){
      return "e";
    }else{
      return COMMON_NOISE::param_name(i);
    }
  }
  std::string param_value(int i)const override {
    int idx = COMMON_FLICK::param_count() - 1 - i;
    if(idx==0){
      return _e.string();
    }else{
      return COMMON_NOISE::param_value(i);
    }
  }
  bool param_is_printable(int i)const override {
    int idx = COMMON_FLICK::param_count() - 1 - i;
    if(idx==0){
      return _e.has_hard_value();
    }else{
      return COMMON_NOISE::param_is_printable(i);
    }
  }
  int set_param_by_name(std::string N, std::string V)override {
    trace2("flick:spbn", N, V);
    if(N == "e"){
      _e = V;
      return 1;
    }else{
      return 1 + COMMON_NOISE::set_param_by_name(N, V);
    }
  }
  double do_noise(ELEMENT const* e, std::string const& what)const override {
    double freq = _sim->_freq;
    return std::pow(freq, -_e) * COMMON_NOISE::do_noise(e, what);
  }
  void precalc_last(CARD_LIST const* scope)override {
    _e.e_val(1., scope);
    COMMON_NOISE::precalc_last(scope);
  }

};
COMMON_FLICK ccf(CC_STATIC);
static DEV_NOISE dfn(&ccf);
static DISPATCHER<CARD>::INSTALL ddf(&device_dispatcher, "flicker_noise|va_flicker_noise", &dfn);
/*--------------------------------------------------------------------------*/
class MEAS_NOISE : public ELEMENT {
private:
  explicit MEAS_NOISE(const MEAS_NOISE& p) : ELEMENT(p) {}
public:
  explicit MEAS_NOISE(COMMON_COMPONENT* cc) : ELEMENT(cc) {}
  explicit MEAS_NOISE()	: ELEMENT() {}
private: // override virtual
  char id_letter()const override {return '\0';}
  std::string value_name()const override {return "#";}
  std::string dev_type()const override {return "meas_noise";}
  CARD* clone()const override {return new MEAS_NOISE(*this);}
//  void set_param_by_index(int, std::string& V)override { untested();
//    incomplete();
//  }
//  bool is_valid()const;
  //int param_count()const override { untested();
  //        return 1 + ELEMENT::param_count();
  //}
  int max_nodes()const override	{return 2;}
  int min_nodes()const override	{return 1;}
  int matrix_nodes()const override	{return 0;}
  int int_nodes()const override	{return 0;}
  int net_nodes()const override	{return _net_nodes;}
private: // purely virtual in ELEMENT
  void tr_iwant_matrix()override {}
  void ac_iwant_matrix()override {}
  double tr_involts()const override { return 0.; }
  double tr_involts_limited()const override { return 0.; }
  COMPLEX ac_involts()const override { return 0.; }
  XPROBE ac_probe_ext(const std::string&)const override;

//  void precalc_last()override { untested();
//	  ELEMENT::precalc_last();
//	  trace1("", mfactor());
//  }
private:
  std::string port_name(int i)const override {
    assert(i < 2);
    static std::string names[] = {"p", "n"};
    return names[i];
  }
};
MEAS_NOISE pp(&ccn);
static DISPATCHER<CARD>::INSTALL d(&device_dispatcher, "noise_probe|meas_noise", &pp);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void DEV_NOISE::precalc_last()
{
  ELEMENT::precalc_last();
  if(_values){
    CARDSTASH z(this);
    detach_common();
    assert(!common());
    set_mfactor(_values[1]);
    trace4("mf hack", long_label(), mfactor(), _values[0], _values[1]);
    COMPONENT::precalc_first();
    z.restore();
  }else{
  }
  auto cc = prechecked_cast<COMMON_NOISE const*>(common());
  assert(cc);
  if(_values){
  }else{
    _value = cc->value();
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void DEV_NOISE::set_parameters(const std::string& Label, CARD *Owner,
			       COMMON_COMPONENT *Common, double Value,
			       int n_states, double states[],
			       int n_nodes, const node_t nodes[])
{
  bool first_time = (net_nodes() == 0);

  set_label(Label);
  trace3("VAACS::set_parameters", long_label(), n_nodes, n_states);
  set_owner(Owner);
  set_value(Value);
  attach_common(Common);

  if (first_time) { untested();
    // _current_port_names.resize(n_states - 1 - n_nodes/2);
    // _input.resize(n_states - 1 - n_nodes/2);
    // _n_ports = n_states-1; // set net_nodes
    // assert(size_t(_n_ports) == n_nodes/2 + _current_port_names.size());
    assert(n_nodes==2);
  }else{
  }

  _values = states;
  std::fill_n(_values, n_states, 0.);
  //std::fill_n(_old_values, n_states, 0.);
  //assert(n_nodes <= net_nodes());
  notstd::copy_n(nodes, n_nodes, _n); // copy more in expand_last
  assert(net_nodes() == 2);
  _loss1 = _loss0 = 1.;
}
/*--------------------------------------------------------------------------*/
double COMMON_NOISE::do_noise(ELEMENT const* e, std::string const& what)const
{
  auto E = prechecked_cast<DEV_NOISE const*>(e);
  assert(E);
  // Assumed that everything is evaluated as AC was called just before
  // invariant ?? :
  if(_name == what || what=="") {
    trace5("COMMON_NOISE::do_noise", long_label(), _name, what, E->value(), _value);

    double i_noise_r = E->value();
    int n1 = e->n_(OUT1).m_();
    int n2 = e->n_(OUT2).m_();
    COMPLEX v = _sim->_noise[n1] - _sim->_noise[n2];
    trace3("done noise", long_label(), v, i_noise_r);

    return std::norm(v)*i_noise_r; // "norm" is the squared magnitude
  }else{
    trace2("COMMON_NOISE::do_noise miss", what, _name);
    return 0.;
  }
}
/*--------------------------------------------------------------------------*/
void set_sens_port(const node_t& n1, const node_t& n2)
{
  size_t noise_size = CKT_BASE::_sim->_total_nodes+1;
  assert(CKT_BASE::_sim->_noise);
  std::fill_n(CKT_BASE::_sim->_noise, noise_size, 0.);

  if(n1.m_()>0){
    assert(n1.m_()<int(noise_size));
    CKT_BASE::_sim->_noise[n1.m_()] = 1.;
  }else{ untested();
    // noises[0] shouldnt do anything
  }
  if(n2.m_()>0){ untested();
    assert(n2.m_()<int(noise_size));
    CKT_BASE::_sim->_noise[n2.m_()] = -1.;
  }else{
    // noises[0] shouldnt do anything
  }
}
/*--------------------------------------------------------------------------*/
double port_noise(const node_t& n1, const node_t& n2, std::string what)
{
  set_sens_port(n1, n2);
  ::status.back.start();
  CKT_BASE::_sim->_acx.fbsubt(CKT_BASE::_sim->_noise);
  ::status.back.stop();
  return CARD_LIST::card_list.noise_num(what);
}
/*--------------------------------------------------------------------------*/
XPROBE DEV_NOISE::ac_probe_ext(const std::string& x) const
{
  trace2("ac noise", long_label(), mfactor());
  if (Umatch(x, "n|value")) {
    return XPROBE(value());
  }else if (Umatch(x, "m")) { untested();
    return XPROBE(mfactor());
  }else{
    return ELEMENT::ac_probe_ext(x);
  }
}
/*--------------------------------------------------------------------------*/
XPROBE MEAS_NOISE::ac_probe_ext(const std::string& x)const
{
  auto cc = prechecked_cast<COMMON_NOISE const*>(common());
  assert(cc);
  std::string const& name = cc->noise_id();
  if (Umatch(x, "npwr")) {
    return XPROBE(port_noise(_n[0], _n[1], name));
  }else if (Umatch(x, "nv")) {
    return XPROBE(sqrt(port_noise(_n[0], _n[1], name)));
  }else{
    return ELEMENT::ac_probe_ext(x);
  }
}
/*--------------------------------------------------------------------------*/
} // anonymous
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
