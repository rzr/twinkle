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

#include "request.h"
#include "util.h"
#include "parse_ctrl.h"
#include "protocol.h"
#include "audits/memman.h"
#include <sstream>
#include <cc++/digest.h>

using namespace ost;

bool t_request::authorize(const t_challenge &chlg,
	const string &username, const string &passwd, unsigned long nc,
	const string &cnonce, t_credentials &cr, string &fail_reason) const
{
	string A1, A2;
	string qop = "";

	// Only Digest authentication is supported
	if (chlg.auth_scheme != AUTH_DIGEST) {
		fail_reason = "Authentication scheme " + chlg.auth_scheme;
		fail_reason += " not supported.";
		return false;
	}

	const t_digest_challenge &dchlg = chlg.digest_challenge;

	// Only MD5 algorithm is supported
	if (dchlg.algorithm != ALG_MD5) {
		fail_reason = "Authentication algorithm " + dchlg.algorithm;
		fail_reason += " not supported.";
		return false;
	}

	// Determine QOP
	// If both auth and auth-int are supported by the server, then
	// choose auth to avoid problems with SIP ALGs. A SIP ALG rewrites
	// the body of a message, thereby breaking auth-int authentication.
	if (!dchlg.qop_options.empty()) {
		const list<string>::const_iterator i = find(
			dchlg.qop_options.begin(), dchlg.qop_options.end(),
			QOP_AUTH_INT);
		const list<string>::const_iterator j = find(
			dchlg.qop_options.begin(), dchlg.qop_options.end(),
			QOP_AUTH);
		if (j != dchlg.qop_options.end())
			qop = QOP_AUTH;
		else {
			if (i != dchlg.qop_options.end())
				qop = QOP_AUTH_INT;
			else {
				fail_reason = "Non of the qop values are supported.";
				return false;
			}
		}
	}

	// RFC 2617 3.2.2.2
	A1 = username + ":" + dchlg.realm + ":" + passwd;

	// RFC 2617 3.2.2.3
	if (qop == QOP_AUTH || qop == "") {
		A2 = method2str(method, unknown_method) + ":" + uri.encode();
	} else {
		A2 = method2str(method, unknown_method) + ":" + uri.encode();
		A2 += ":";
		if (body) {
			MD5Digest MD5body;
			MD5body << body->encode();
			ostringstream os;
			os << MD5body;
			A2 += os.str();
		} else {
			MD5Digest MD5body;
			MD5body << "";
			ostringstream os;
			os << MD5body;
			A2 += os.str();
		}
	}

	// RFC 2716 3.2.2.1
	// Caculate digest
	MD5Digest MD5A1;
	MD5Digest MD5A2;
	ostringstream HA1;
	ostringstream HA2;

	MD5A1 << A1;
	MD5A2 << A2;
	HA1 << MD5A1;
	HA2 << MD5A2;

	string x;

	if (qop == QOP_AUTH || qop == QOP_AUTH_INT) {
		x = HA1.str() + ":";
		x += dchlg.nonce + ":";
		x += int2str(nc, "%08x") + ":";
		x += cnonce + ":";
		x += qop + ":";
		x += HA2.str();
	} else {
		x = HA1.str() + ":";
		x += dchlg.nonce + ":";
		x += HA2.str();
	}

	// Create credentials
	cr.auth_scheme = AUTH_DIGEST;
	t_digest_response &dr = cr.digest_response;

	MD5Digest digest;
	digest << x;
	ostringstream dresp;
	dresp << digest;

	dr.dresponse = dresp.str();
	dr.username = username;
	dr.realm = dchlg.realm;
	dr.nonce = dchlg.nonce;
	dr.digest_uri = uri;
	dr.algorithm = ALG_MD5;
	dr.opaque = dchlg.opaque;

	// RFC 2617 3.2.2
	if (qop != "") {
		dr.message_qop = qop;
		dr.cnonce = cnonce;
		dr.nonce_count = nc;
	}

	return true;
}

t_request::t_request() : t_sip_message() {
	method = METHOD_UNKNOWN;
}

t_request::t_request(const t_request &r) : t_sip_message(r),
		uri(r.uri),
		method(r.method),
		unknown_method(r.unknown_method),
		destinations(r.destinations)
{
}

t_request::t_request(const t_method m) : t_sip_message() {
	method = m;
}

void t_request::set_method(const string &s) {
	method = str2method(s);
	if (method == METHOD_UNKNOWN) {
		unknown_method = s;
	}
}

string t_request::encode(bool add_content_length) {
	string s;

	s = method2str(method, unknown_method) + ' ' + uri.encode();
	s += " SIP/";
	s += version;
	s += CRLF;
	s += t_sip_message::encode(add_content_length);
	return s;
}

list<string> t_request::encode_env(void) {
	string s;
	list<string> l = t_sip_message::encode_env();
	
	s = "SIPREQUEST_METHOD=";
	s += method2str(method, unknown_method);
	l.push_back(s);
	
	s = "SIPREQUEST_URI=";
	s += uri.encode();
	l.push_back(s);
	
	return l;
}

t_sip_message *t_request::copy(void) const {
	t_sip_message *m =  new t_request(*this);
	MEMMAN_NEW(m);
	return m;
}

t_response *t_request::create_response(int code, string reason) const
{
	t_response *r;

	r = new t_response(code, reason);
	MEMMAN_NEW(r);
	
	r->src_ip_port_request = src_ip_port;
	
	r->hdr_from = hdr_from;
	r->hdr_call_id = hdr_call_id;
	r->hdr_cseq = hdr_cseq;
	r->hdr_via = hdr_via;
	r->hdr_to = hdr_to;

	// Create a to-tag if none was present in the request
	// NOTE: 100 Trying should not get a to-tag
	if (hdr_to.tag.size() == 0 && code != R_100_TRYING) {
		r->hdr_to.set_tag(NEW_TAG);
	}

	// Server
	SET_HDR_SERVER(r->hdr_server);

	return r;
}

bool t_request::is_valid(bool &fatal, string &reason) const {
	if (!t_sip_message::is_valid(fatal, reason)) return false;

	fatal = false;

	if (t_parser::check_max_forwards && !hdr_max_forwards.is_populated()) {
		reason = "Max-Forwards header missing";
		return false;
	}

	// RFC 3261 8.1.1.5
	// The CSeq method must match the request method.
	if (hdr_cseq.method != method) {
		reason = "CSeq method does not match request method";
		return false;
	}

	switch(method) {
	case INVITE:
		if (!hdr_contact.is_populated()) {
			reason = "Contact header missing";
			return false;
		}
		break;
	case PRACK:
		// RFC 3262 7.1
		if (!hdr_rack.is_populated()) {
			reason = "RAck header missing";
			return false;
		}
		break;
	case SUBSCRIBE:
		// RFC 3265 7.1, 7.2
		if (!hdr_contact.is_populated()) {
			reason = "Contact header missing";
			return false;
		}

		if (!hdr_event.is_populated()) {
			reason = "Event header missing";
			return false;
		}
		break;
	case NOTIFY:
		// RFC 3265 7.1, 7.2
		if (!hdr_contact.is_populated()) {
			reason = "Contact header missing";
			return false;
		}

		if (!hdr_event.is_populated()) {
			reason = "Event header missing";
			return false;
		}
		
		// RFC 3265 7.2
		// Subscription-State header is mandatory
		// As an exception Twinkle allows an unsollicited NOTIFY for MWI
		// without a Subscription-State header. Asterisk sends
		// unsollicited NOTIFY requests.
		if (!hdr_to.tag.empty() || 
		    hdr_event.event_type != SIP_EVENT_MSG_SUMMARY)
		{
			if (!hdr_subscription_state.is_populated()) {
				reason = "Subscription-State header missing";
			 	return false;
			 }
		}

		// The Subscription-State header is mandatory.
		// However, Asterisk uses an expired draft for sending
		// unsollicitied NOTIFY messages without a Subscription-State
		// header. As Asterisk is popular, Twinkle allows this.
		break;
	case REFER:
		// RFC 3515 2.4.1
		if (!hdr_refer_to.is_populated()) {
			reason = "Refer-To header missing";
			return false;
		}
		break;
	}
	
	if (hdr_replaces.is_populated()) {
		// RFC 3891 3
		if (method != INVITE) {
			reason = "Replaces header not allowed";
			return false;
		}
	}

	return true;
}

void t_request::add_destinations(const t_user &user_profile, const t_url &dst_uri) {
	if ((user_profile.get_sip_transport() == SIP_TRANS_AUTO ||
	     user_profile.get_sip_transport() == SIP_TRANS_TCP)
	    &&
	    (dst_uri.get_transport().empty() ||
	     cmp_nocase(dst_uri.get_transport(), "tcp") == 0))
	{
		list<t_ip_port> l = dst_uri.get_h_ip_srv("tcp");
		destinations.insert(destinations.end(), l.begin(), l.end());
	}
	
	if ((user_profile.get_sip_transport() == SIP_TRANS_AUTO ||
	    user_profile.get_sip_transport() == SIP_TRANS_UDP)
	   &&
	   (dst_uri.get_transport().empty() ||
	    cmp_nocase(dst_uri.get_transport(), "udp") == 0))
	{
		list<t_ip_port> l = dst_uri.get_h_ip_srv("udp");
		destinations.insert(destinations.end(), l.begin(), l.end());
	}
}

void t_request::calc_destinations(const t_user &user_profile) {
	destinations.clear();

	// Send a REGISTER to the registrar if provisioned.
	if (method == REGISTER && user_profile.get_use_registrar()) {
		add_destinations(user_profile, user_profile.get_registrar());
		return;
	}
	
	// Bypass the proxy for an out-of-dialog SUBSCRIBE if provisioned.
	if (method == SUBSCRIBE && hdr_to.tag.empty()) {
		if (hdr_event.event_type == SIP_EVENT_MSG_SUMMARY) {
			if (!user_profile.get_mwi_via_proxy()) {
				// Take Request-URI
				add_destinations(user_profile, uri);
				return;
			}
		}
	}

	if (!user_profile.get_use_outbound_proxy() ||
	    (hdr_to.tag != "" && !user_profile.get_all_requests_to_proxy())) {
		// A mid dialog request will go to the host in the contact
		// header (put in the request-URI in this request) or route list
		// specified in the final response of the invite (the Route-header in
		// this request).
		// Note that an ACK for a failed INVITE (3XX-6XX) will be
		// sent by the transaction layer to the ipaddr/port of the
		// INVITE.
		if (hdr_route.is_populated() && hdr_route.route_to_first_route) {
			// Take URI from first route-header
			t_url &u = hdr_route.route_list.front().uri;
			add_destinations(user_profile, u);
		} else {
			// Take Request-URI
			add_destinations(user_profile, uri);
		}
	}

	// Send request to outbound proxy if configured
	if (user_profile.get_use_outbound_proxy()) {
		if (user_profile.get_non_resolvable_to_proxy() && !destinations.empty())
		{
			// The destination has been resolved, so do not
			// use the outbound proxy in this case.
			return;
		}

		if (user_profile.get_all_requests_to_proxy() || hdr_to.tag == "") {
			// All requests should go to the proxy.
			// Override destination by the outbound proxy address.
			destinations.clear();
			add_destinations(user_profile, user_profile.get_outbound_proxy());
		}
	}
}

void t_request::get_destination(t_ip_port &ip_port, const t_user &user_profile) {
	if (destinations.empty()) calc_destinations(user_profile);
	
	// RFC 3261 18.1.1
	// If the message size is larger than 1300 then the message must be
	// sent over TCP.
	// If the request-URI indicated an explicit transport, then the
	// destination calculation picked the possible destinations already.
	// The size cannot influence this calculation anymore.
	if (user_profile.get_sip_transport() == SIP_TRANS_AUTO &&
	    !destinations.empty() &&
	    destinations.front().transport == "tcp" &&
	    get_encoded_size() <= user_profile.get_sip_transport_udp_threshold() &&
	    uri.get_transport().empty())
	{
		// The message can be sent over UDP. Remove all TCP destinations.
		while (!destinations.empty() && destinations.front().transport == "tcp") {
			destinations.pop_front();
		}
	}
	
	get_current_destination(ip_port);
}

void t_request::get_current_destination(t_ip_port &ip_port) {
	if (destinations.empty()) {
		// No destinations could be found.
		ip_port.transport = "udp";
		ip_port.ipaddr = 0;
		ip_port.port = 0;
	} else {
		// Return first destination
		ip_port = destinations.front();
	}
}

bool t_request::next_destination(void) {		
	if (destinations.size() <= 1) return false;
	
	// Remove current destination
	destinations.pop_front();
	return true;	
}

void t_request::set_destination(const t_ip_port &ip_port) {
	destinations.clear();
	destinations.push_back(ip_port);
}

bool t_request::www_authorize(const t_challenge &chlg, const string &username,
	       const string &passwd, unsigned long nc,
	       const string &cnonce, t_credentials &cr, string &fail_reason)
{
	if (!authorize(chlg, username, passwd, nc, cnonce, cr, fail_reason)) {
		return false;
	}

	hdr_authorization.add_credentials(cr);

	return true;
}

bool t_request::proxy_authorize(const t_challenge &chlg, const string &username,
	       const string &passwd, unsigned long nc,
	       const string &cnonce, t_credentials &cr, string &fail_reason)
{
	if (!authorize(chlg, username, passwd, nc, cnonce, cr, fail_reason)) {
		return false;
	}

	hdr_proxy_authorization.add_credentials(cr);

	return true;
}

void t_request::calc_local_ip(void) {
	t_ip_port dst;
	
	get_current_destination(dst);
	if (dst.ipaddr != 0) {
		local_ip_ = get_src_ip4_address_for_dst(dst.ipaddr);
	}
}
