
#ifndef MG_TYPE_H
#define MG_TYPE_H
class Data_Type : public Base{
protected:
  typedef enum{ t_default, t_real, t_int, t_string, t_fun } type_t;
  type_t _type;
  Data_Type(type_t type) : Base(), _type(type){}
public:
  Data_Type(Data_Type const& t) : Base(), _type(t._type){}
  Data_Type() : Base(), _type(t_default){}
  Data_Type& operator=(Data_Type const& o){ _type = o._type; return *this;}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  bool is_real() const{ return _type==t_real; }
  bool is_int() const{ return _type==t_int; }
  bool is_string() const{ return _type==t_string; }
  bool is_function() const{ return _type==t_fun; }
  operator bool() const {return _type!=t_default;}
  bool operator==(Data_Type const& o)const { return _type == o._type;}
  bool operator!=(Data_Type const& o)const { return !operator==(o);}
};
/*--------------------------------------------------------------------------*/
char const* code_name(Data_Type const*x);
/*--------------------------------------------------------------------------*/
class Data_Type_Real : public Data_Type{
public:
  Data_Type_Real() : Data_Type(t_real){}
};
/*--------------------------------------------------------------------------*/
class Data_Type_Int : public Data_Type{
public:
  Data_Type_Int() : Data_Type(t_int){}
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
