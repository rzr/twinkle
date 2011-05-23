/*
    Copyright (C) 2005-2008  Michel de Boer <michel@twinklephone.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "definitions.h"
#include "hdr_record_route.h"
#include "parse_ctrl.h"
#include "util.h"

void t_route::add_param(const t_parameter &p) {
	params.push_back(p);
}

void t_route::set_params(const list<t_parameter> &l) {
	params = l;
}

string t_route::encode(void) const {
	string s;

	if (display.size() > 0) {
		s += '"';
		s += escape(display, '"');
		s += '"';
		s += ' ';
	}

	s += '<';
	s += uri.encode();
	s += '>';

	s += param_list2str(params);
	return s;
}

t_hdr_record_route::t_hdr_record_route() : t_header("Record-Route") {}

void t_hdr_record_route::add_route(const t_route &r) {
	populated = true;
	route_list.push_back(r);
}

string t_hdr_record_route::encode(void) const {
	return (t_parser::multi_values_as_list ? 
			t_header::encode() : encode_multi_header());
}

string t_hdr_record_route::encode_multi_header(void) const {
	string s;

	if (!populated) return s;

	for (list<t_route>::const_iterator i = route_list.begin();
	     i != route_list.end(); i++)
	{
		s += header_name;
		s += ": ";
		s += i->encode();
		s += CRLF;
	}

	return s;
}

string t_hdr_record_route::encode_value(void) const {
	string s;

	if (!populated) return s;

	for (list<t_route>::const_iterator i = route_list.begin();
	     i != route_list.end(); i++)
	{
		if (i != route_list.begin()) s += ",";
		s += i->encode();
	}

	return s;
}
