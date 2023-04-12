/*$Id: mg_in_discipline.cc,v 26.11 2006/12/31 23:12:39 al Exp $ -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 * Author: Albert Davis <aldavis@ieee.org>
 *
 * This file is part of "Gnucap", the Gnu Circuit Analysis Package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
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
 */
#include "mg_.h"
/*--------------------------------------------------------------------------*/
/* A.1.6    3.6.1
+ nature_declaration ::=
+	  "nature"  nature_identifier  [  ":"  parent_nature  ]  [  ";"  ]
+		{ nature_item }
+	  "endnature"
+ parent_nature ::=
+	  nature_identifier
-	| discipline_identifier  potential_or_flow
+ nature_item ::=
+	  nature_attribute
+ nature_attribute ::=
+	  nature_attribute_identifier  "="  nature_attribute_expression  ";"
// A.9.3
+ nature_identifier ::=
+	  identifier
+ nature_attribute_identifier ::=
?	  "abstol"
?	| "access"
?	| "ddt_nature"
?	| "idt_nature"
?	| "units"
+	| identifier
*/
void Nature::parse(CS& file)
{
  // file >> "nature "; from caller
  file >> _identifier;
  (file >> ':') && (file >> _parent_nature);
  file >> ';';
 
  size_t here = file.cursor();
  for (;;) {
    ONE_OF // nature_item
      || (file >> _attributes)
      ;
    if (file.umatch("endnature")) {
      break;
    }else if (!file.more()) {
      file.warn(0, "premature EOF (nature)");
      break;
    }else if (file.stuck(&here)) {
      file.warn(0, "bad nature");
      break;
    }else{
    }
  }
  assert(file);
}
/*--------------------------------------------------------------------------*/
void Nature::dump(std::ostream& out)const
{
  out << "nature " << identifier();
  if (parent_nature() != "") {
    out << " : " << parent_nature();
  }
  out << "\n"
      << attributes()
      << "endnature\n\n";
}
/*--------------------------------------------------------------------------*/
/* A.1.7    3.6.2
+ discipline_declaration ::=
+	  "discipline"  discipline_identifier [ ";" ]
+		{ discipline_item }
+	  "enddiscipline"
+ discipline_item ::=
+	  nature_binding
-	| discipline_domain_binding
-	| nature_attribute_override
+ nature_binding ::=
+	  potential_or_flow  nature_identifier  ";"
+ potential_or_flow ::=
+	  "potential"
+	| "flow"
- discipline_domain_binding ::=
-	  "domain"  discrete_or_continuous  ";"
- discrete_or_continuous ::=
-	  "discrete"
-	| "continuous"
- nature_attribute_override ::=
-	  potential_or_flow  "."  nature_attribute
*/
void Discipline::parse(CS& file)
{
  // file >> "discipline "; from caller
  file >> _identifier;
  
  size_t here = file.cursor();
  for (;;) {
    ONE_OF // discipline_item
      || ((file >> "potential") && (file >> _potential_ident >> ';'))
      || ((file >> "flow")	&& (file >> _flow_ident >> ';'))
      ;
    if (file.umatch("enddiscipline")) {
      break;
    }else if (!file.more()) {
      file.warn(0, "premature EOF (discipline)");
      break;
    }else if (file.stuck(&here)) {
      file.warn(0, "bad discipline");
      break;
    }else{
    }
  }
  assert(file);

  assert(owner());
  File const* f = dynamic_cast<File const*>(owner());
  assert(f);

  if(_potential_ident!=""){
	  auto i = f->nature_list().find(_potential_ident);
	  if(i != f->nature_list().end()){
		  _potential = *i;
	  }else{ untested();
			file.warn(0, "invalid potential nature: " + _potential_ident.to_string());
	  }
	}else{
	}

  if(_flow_ident!=""){
	  auto i = f->nature_list().find(_flow_ident);
	  if(i != f->nature_list().end()){
		  _flow = *i;
	  }else{ untested();
			file.warn(0, "invalid flow nature: " + _flow_ident.to_string());
	  }
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void Discipline::dump(std::ostream& out)const
{
  out <<
    "discipline "    << identifier()      << "\n"
    "  potential \"" << potential_ident() << "\";\n"
    "  flow \""	     << flow_ident()	  << "\";\n"
    "enddiscipline\n\n";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
