
#ifndef MG_FILTER_H
#define MG_FILTER_H
#include "mg_circuit.h"
// TODO: merge into Element_2?
class Filter : public Element_2 {
  std::string _name; // BUG?
  Deps* _deps{NULL};
  Branch_Ref _branch;
  Probe const* _prb=NULL;
  int _num_states{0};
public:
  explicit Filter(std::string const& name);
  explicit Filter() : Element_2() { new_deps(); }
  ~Filter();
  void parse(CS&) override{unreachable();}
  void dump(std::ostream&)const override {unreachable();}

  void set_num_states(int s) {_num_states = s; }
  void set_output(Branch_Ref const& x);
//  Branch_Ref const& branch() const{ return _branch; }
  std::string name() const {
    return _name;
  }
  std::string branch_code_name() const {
    assert(_branch);
    return _branch->code_name();
  }
  size_t num_branches()const;
  std::string code_name()const override;
  Deps const& deps()const { assert(_deps); return *_deps; }

  size_t num_states()const override;
  size_t num_nodes()const override;
  std::string state()const override;
  std::string short_label()const override;
//  Probe const* prb() const;
  bool has_branch() const {
    return _branch;
  }
  void set_deps(Deps const&);
private:
  Deps& deps() { assert(_deps); return *_deps; }
  void new_deps();
}; // Filter
#endif
