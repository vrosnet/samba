/* 
   Unix SMB/CIFS implementation.

   util time testing

   Copyright (C) Jelmer Vernooij <jelmer@samba.org> 2008
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "includes.h"
#include "torture/torture.h"

static bool test_null_time(struct torture_context *tctx)
{
	torture_assert(tctx, null_time(0), "0");
	torture_assert(tctx, null_time(0xFFFFFFFF), "0xFFFFFFFF");
	torture_assert(tctx, null_time(-1), "-1");
	torture_assert(tctx, !null_time(42), "42");
	return true;
}

static bool test_null_nttime(struct torture_context *tctx)
{
	torture_assert(tctx, null_nttime(-1), "-1");
	torture_assert(tctx, null_nttime(-1), "-1");
	torture_assert(tctx, !null_nttime(42), "42");
	return true;
}


static bool test_http_timestring(struct torture_context *tctx)
{
	const char *start = "Thu, 01 Jan 1970";
	torture_assert(tctx, !strncmp(start, http_timestring(tctx, 42), 
										 strlen(start)), "42");
	torture_assert_str_equal(tctx, "never", 
							 http_timestring(tctx, get_time_t_max()), "42");
	return true;
}

static bool test_timestring(struct torture_context *tctx)
{
	const char *start = "Thu Jan  1";
	torture_assert(tctx, !strncmp(start, timestring(tctx, 42), strlen(start)),
				   "42");
	return true;
}



struct torture_suite *torture_local_util_time(TALLOC_CTX *mem_ctx)
{
	struct torture_suite *suite = torture_suite_create(mem_ctx, "TIME");

	torture_suite_add_simple_test(suite, "null_time", test_null_time);
	torture_suite_add_simple_test(suite, "null_nttime", test_null_nttime);
	torture_suite_add_simple_test(suite, "http_timestring", 
								  test_http_timestring);
	torture_suite_add_simple_test(suite, "timestring", 
								  test_timestring);

	return suite;
}