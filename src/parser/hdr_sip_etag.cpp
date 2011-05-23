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

#include "hdr_sip_etag.h"

t_hdr_sip_etag::t_hdr_sip_etag() : t_header("SIP-ETag") {};

void t_hdr_sip_etag::set_etag(const string &_etag) {
	populated = true;
	etag = _etag;
}

string t_hdr_sip_etag::encode_value(void) const {
	if (!populated) return "";
	return etag;
}
