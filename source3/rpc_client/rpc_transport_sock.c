/*
 *  Unix SMB/CIFS implementation.
 *  RPC client transport over a socket
 *  Copyright (C) Volker Lendecke 2009
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "includes.h"

#undef DBGC_CLASS
#define DBGC_CLASS DBGC_RPC_CLI

struct rpc_transport_sock_state {
	int fd;
};

static int rpc_transport_sock_state_destructor(struct rpc_transport_sock_state *s)
{
	if (s->fd != -1) {
		close(s->fd);
		s->fd = -1;
	}
	return 0;
}

struct rpc_sock_read_state {
	ssize_t received;
};

static void rpc_sock_read_done(struct tevent_req *subreq);

static struct tevent_req *rpc_sock_read_send(TALLOC_CTX *mem_ctx,
					     struct event_context *ev,
					     uint8_t *data, size_t size,
					     void *priv)
{
	struct rpc_transport_sock_state *sock_transp = talloc_get_type_abort(
		priv, struct rpc_transport_sock_state);
	struct tevent_req *req, *subreq;
	struct rpc_sock_read_state *state;

	req = tevent_req_create(mem_ctx, &state, struct rpc_sock_read_state);
	if (req == NULL) {
		return NULL;
	}

	subreq = async_recv_send(state, ev, sock_transp->fd, data, size, 0);
	if (subreq == NULL) {
		goto fail;
	}
	tevent_req_set_callback(subreq, rpc_sock_read_done, req);
	return req;
 fail:
	TALLOC_FREE(req);
	return NULL;
}

static void rpc_sock_read_done(struct tevent_req *subreq)
{
	struct tevent_req *req = tevent_req_callback_data(
		subreq, struct tevent_req);
	struct rpc_sock_read_state *state = tevent_req_data(
		req, struct rpc_sock_read_state);
	int err;

	state->received = async_recv_recv(subreq, &err);
	if (state->received == -1) {
		tevent_req_nterror(req, map_nt_error_from_unix(err));
		return;
	}
	tevent_req_done(req);
}

static NTSTATUS rpc_sock_read_recv(struct tevent_req *req, ssize_t *preceived)
{
	struct rpc_sock_read_state *state = tevent_req_data(
		req, struct rpc_sock_read_state);
	NTSTATUS status;

	if (tevent_req_is_nterror(req, &status)) {
		return status;
	}
	*preceived = state->received;
	return NT_STATUS_OK;
}

struct rpc_sock_write_state {
	ssize_t sent;
};

static void rpc_sock_write_done(struct tevent_req *subreq);

static struct tevent_req *rpc_sock_write_send(TALLOC_CTX *mem_ctx,
					      struct event_context *ev,
					      const uint8_t *data, size_t size,
					      void *priv)
{
	struct rpc_transport_sock_state *sock_transp = talloc_get_type_abort(
		priv, struct rpc_transport_sock_state);
	struct tevent_req *req, *subreq;
	struct rpc_sock_write_state *state;

	req = tevent_req_create(mem_ctx, &state, struct rpc_sock_write_state);
	if (req == NULL) {
		return NULL;
	}
	subreq = async_send_send(state, ev, sock_transp->fd, data, size, 0);
	if (subreq == NULL) {
		goto fail;
	}
	tevent_req_set_callback(subreq, rpc_sock_write_done, req);
	return req;
 fail:
	TALLOC_FREE(req);
	return NULL;
}

static void rpc_sock_write_done(struct tevent_req *subreq)
{
	struct tevent_req *req = tevent_req_callback_data(
		subreq, struct tevent_req);
	struct rpc_sock_write_state *state = tevent_req_data(
		req, struct rpc_sock_write_state);
	int err;

	state->sent = async_send_recv(subreq, &err);
	if (state->sent == -1) {
		tevent_req_nterror(req, map_nt_error_from_unix(err));
		return;
	}
	tevent_req_done(req);
}

static NTSTATUS rpc_sock_write_recv(struct tevent_req *req, ssize_t *psent)
{
	struct rpc_sock_write_state *state = tevent_req_data(
		req, struct rpc_sock_write_state);
	NTSTATUS status;

	if (tevent_req_is_nterror(req, &status)) {
		return status;
	}
	*psent = state->sent;
	return NT_STATUS_OK;
}

NTSTATUS rpc_transport_sock_init(TALLOC_CTX *mem_ctx, int fd,
				 struct rpc_cli_transport **presult)
{
	struct rpc_cli_transport *result;
	struct rpc_transport_sock_state *state;

	result = talloc(mem_ctx, struct rpc_cli_transport);
	if (result == NULL) {
		return NT_STATUS_NO_MEMORY;
	}
	state = talloc(result, struct rpc_transport_sock_state);
	if (state == NULL) {
		TALLOC_FREE(result);
		return NT_STATUS_NO_MEMORY;
	}
	result->priv = state;

	state->fd = fd;
	talloc_set_destructor(state, rpc_transport_sock_state_destructor);

	result->trans_send = NULL;
	result->trans_recv = NULL;
	result->write_send = rpc_sock_write_send;
	result->write_recv = rpc_sock_write_recv;
	result->read_send = rpc_sock_read_send;
	result->read_recv = rpc_sock_read_recv;

	*presult = result;
	return NT_STATUS_OK;
}
