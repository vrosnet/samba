/* 
   Unix SMB/CIFS implementation.

   process model: standard (1 process per client connection)

   Copyright (C) Andrew Tridgell 1992-2005
   Copyright (C) James J Myers 2003 <myersjj@samba.org>
   Copyright (C) Stefan (metze) Metzmacher 2004
   
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
#include "lib/events/events.h"
#include "../tdb/include/tdb.h"
#include "lib/socket/socket.h"
#include "smbd/process_model.h"
#include "param/secrets.h"
#include "system/filesys.h"
#include "cluster/cluster.h"
#include "param/param.h"

#ifdef HAVE_SETPROCTITLE
#ifdef HAVE_SETPROCTITLE_H
#include <setproctitle.h>
#endif
#else
#define setproctitle none_setproctitle
static int none_setproctitle(const char *fmt, ...) PRINTF_ATTRIBUTE(1, 2);
static int none_setproctitle(const char *fmt, ...)
{
	return 0;
}
#endif

/*
  called when the process model is selected
*/
static void standard_model_init(struct tevent_context *ev)
{
	signal(SIGCHLD, SIG_IGN);
}

/*
  called when a listening socket becomes readable. 
*/
static void standard_accept_connection(struct tevent_context *ev, 
				       struct loadparm_context *lp_ctx,
				       struct socket_context *sock, 
				       void (*new_conn)(struct tevent_context *,
							struct loadparm_context *, struct socket_context *, 
							struct server_id , void *), 
				       void *private_data)
{
	NTSTATUS status;
	struct socket_context *sock2;
	pid_t pid;
	struct tevent_context *ev2;
	struct socket_address *c, *s;

	/* accept an incoming connection. */
	status = socket_accept(sock, &sock2);
	if (!NT_STATUS_IS_OK(status)) {
		DEBUG(0,("standard_accept_connection: accept: %s\n",
			 nt_errstr(status)));
		/* this looks strange, but is correct. We need to throttle things until
		   the system clears enough resources to handle this new socket */
		sleep(1);
		return;
	}

	pid = fork();

	if (pid != 0) {
		/* parent or error code ... */
		talloc_free(sock2);
		/* go back to the event loop */
		return;
	}

	pid = getpid();

	/* This is now the child code. We need a completely new event_context to work with */
	ev2 = s4_event_context_init(NULL);

	/* the service has given us a private pointer that
	   encapsulates the context it needs for this new connection -
	   everything else will be freed */
	talloc_steal(ev2, private_data);
	talloc_steal(private_data, sock2);

	/* this will free all the listening sockets and all state that
	   is not associated with this new connection */
	talloc_free(sock);
	talloc_free(ev);

	/* we don't care if the dup fails, as its only a select()
	   speed optimisation */
	socket_dup(sock2);
			
	/* tdb needs special fork handling */
	if (tdb_reopen_all(1) == -1) {
		DEBUG(0,("standard_accept_connection: tdb_reopen_all failed.\n"));
	}

	/* Ensure that the forked children do not expose identical random streams */
	set_need_random_reseed();

	/* setup the process title */
	c = socket_get_peer_addr(sock2, ev2);
	s = socket_get_my_addr(sock2, ev2);
	if (s && c) {
		setproctitle("conn c[%s:%u] s[%s:%u] server_id[%d]",
			     c->addr, c->port, s->addr, s->port, pid);
	}
	talloc_free(c);
	talloc_free(s);

	/* setup this new connection.  Cluster ID is PID based for this process modal */
	new_conn(ev2, lp_ctx, sock2, cluster_id(pid, 0), private_data);

	/* we can't return to the top level here, as that event context is gone,
	   so we now process events in the new event context until there are no
	   more to process */	   
	event_loop_wait(ev2);

	talloc_free(ev2);
	exit(0);
}

/*
  called to create a new server task
*/
static void standard_new_task(struct tevent_context *ev, 
			      struct loadparm_context *lp_ctx,
			      const char *service_name,
			      void (*new_task)(struct tevent_context *, struct loadparm_context *lp_ctx, struct server_id , void *), 
			      void *private_data)
{
	pid_t pid;
	struct tevent_context *ev2;

	pid = fork();

	if (pid != 0) {
		/* parent or error code ... go back to the event loop */
		return;
	}

	pid = getpid();

	/* This is now the child code. We need a completely new event_context to work with */
	ev2 = s4_event_context_init(NULL);

	/* the service has given us a private pointer that
	   encapsulates the context it needs for this new connection -
	   everything else will be freed */
	talloc_steal(ev2, private_data);

	/* this will free all the listening sockets and all state that
	   is not associated with this new connection */
	talloc_free(ev);

	/* tdb needs special fork handling */
	if (tdb_reopen_all(1) == -1) {
		DEBUG(0,("standard_accept_connection: tdb_reopen_all failed.\n"));
	}

	/* Ensure that the forked children do not expose identical random streams */
	set_need_random_reseed();

	setproctitle("task %s server_id[%d]", service_name, pid);

	/* setup this new task.  Cluster ID is PID based for this process modal */
	new_task(ev2, lp_ctx, cluster_id(pid, 0), private_data);

	/* we can't return to the top level here, as that event context is gone,
	   so we now process events in the new event context until there are no
	   more to process */	   
	event_loop_wait(ev2);

	talloc_free(ev2);
	exit(0);
}


/* called when a task goes down */
_NORETURN_ static void standard_terminate(struct tevent_context *ev, struct loadparm_context *lp_ctx, 
					  const char *reason) 
{
	DEBUG(2,("standard_terminate: reason[%s]\n",reason));

	/* this reload_charcnv() has the effect of freeing the iconv context memory,
	   which makes leak checking easier */
	reload_charcnv(lp_ctx);

	talloc_free(ev);

	/* terminate this process */
	exit(0);
}

/* called to set a title of a task or connection */
static void standard_set_title(struct tevent_context *ev, const char *title) 
{
	if (title) {
		setproctitle("%s", title);
	} else {
		setproctitle(NULL);
	}
}

static const struct model_ops standard_ops = {
	.name			= "standard",
	.model_init		= standard_model_init,
	.accept_connection	= standard_accept_connection,
	.new_task               = standard_new_task,
	.terminate              = standard_terminate,
	.set_title              = standard_set_title,
};

/*
  initialise the standard process model, registering ourselves with the process model subsystem
 */
NTSTATUS process_model_standard_init(void)
{
	return register_process_model(&standard_ops);
}
