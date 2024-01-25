
#ifndef MG_DISCIPLINE_H
#define MG_DISCIPLINE_H
#include "mg_base.h"
#include "mg_attrib.h"
/*--------------------------------------------------------------------------*/
class Nature : public Base {
  String_Arg		_identifier;
  String_Arg		_parent_nature;
  Attribute_List	_attributes;
public:
  void set_owner(Block const*){}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  Nature() {}
  const String_Arg&	identifier()const	{return _identifier;}
  const String_Arg&  key()const	  {return _identifier;}
  const String_Arg&	parent_nature()const	{return _parent_nature;}
  const Attribute_List&	attributes()const	{return _attributes;}
  const String_Arg&	operator[](const String_Arg& k)const {return _attributes[k];}
  const String_Arg&	access()const		{return _attributes["access"];}
  const String_Arg&	abstol()const		{return _attributes["abstol"];}
};
typedef Collection<Nature> Nature_List;
/*--------------------------------------------------------------------------*/
class Discipline : public Base {
  String_Arg	_identifier;
  String_Arg	_potential_ident;
  String_Arg	_domain_ident;
  String_Arg	_flow_ident;
  Nature const* _flow{NULL};
  Nature const* _potential{NULL};
  Block const* _owner{NULL};
public:
  void set_owner(Block const* c) {_owner=c;}
  Block const* owner() {return _owner;}
  const String_Arg&  key()const	  {return _identifier;}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  explicit Discipline() {}
  const String_Arg&  identifier()const	    {return _identifier;}
  const String_Arg&  potential_ident()const {return _potential_ident;}
  const String_Arg&  domain_ident()const    {return _domain_ident;}
  const String_Arg&  flow_ident()const	    {return _flow_ident;}

  Nature const* flow() const{return _flow;}
  Nature const* potential()const{return _potential;}
};
typedef Collection<Discipline> Discipline_List;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
