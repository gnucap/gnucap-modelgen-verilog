
/*--------------------------------------------------------------------------*/
void Data_Type::parse(CS&) {unreachable();}
void Data_Type::dump(std::ostream&)const {unreachable();}
/*--------------------------------------------------------------------------*/
Data_Type const* MGVAMS_FUNCTION::return_type() const
{
  static Data_Type_Real r;
  return &r;
}
/*--------------------------------------------------------------------------*/
