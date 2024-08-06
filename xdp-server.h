/*
 * xdp-server.h -- integration of AF_XDP into nsd
 *
 * Copyright (c) 2024, NLnet Labs. All rights reserved.
 *
 * See LICENSE for the license.
 *
 */

#ifndef XDP_SERVER_H
#define XDP_SERVER_H

#include <stdint.h>
#include <xdp/xsk.h>

#include "region-allocator.h"

/* TODO: check if number is sensible */
#define XDP_NUM_FRAMES 4096
#define XDP_FRAME_SIZE XSK_UMEM__DEFAULT_FRAME_SIZE
#define XDP_BUFFER_SIZE XDP_NUM_FRAMES * XDP_FRAME_SIZE
/* TODO: check if number is sensible */
#define XDP_RX_BATCH_SIZE 64
#define XDP_INVALID_UMEM_FRAME UINT64_MAX
#define XSK_RING_PROD__NUM_DESCS XSK_RING_PROD__DEFAULT_NUM_DESCS
#define XSK_RING_CONS__NUM_DESCS XSK_RING_CONS__DEFAULT_NUM_DESCS

struct xsk_umem_info {
	struct xsk_ring_prod fq;
	struct xsk_ring_cons cq;
	struct xsk_umem *umem;
	void *buffer;

	uint64_t umem_frame_addr[XDP_NUM_FRAMES];
	uint32_t umem_frame_free;
};

struct xsk_socket_info {
	struct xsk_ring_cons rx;
	struct xsk_ring_prod tx;
	struct xsk_umem_info *umem;
	struct xsk_socket *xsk;

	uint32_t outstanding_tx;
};

struct xdp_server {
	/* NSD global settings */
	region_type *region;
	char const *interface_name;
	char const *bpf_prog_filename;
	char const *bpf_bpffs_path;
	int bpf_prog_should_load;

	/* track bpf objects and file descriptors */
	int xsk_map_fd;
	int bpf_prog_fd;
	int bpf_prog_id;
	struct bpf_map *xsk_map;
	struct xdp_program *bpf_prog;

	int interface_index;
	int queue_count;
	uint32_t queue_index;

	struct query **queries;
	void *nsd;

	/* these items/arrays are shared between processes */
	/* the number of sockets corresponds to the queue_count */
	/* these are allocated using mmap and are automatically unmapped on exit */
	struct xsk_umem_info *umems;
	struct xsk_socket_info *xsks;
};

/*
 * Handle reading and writing packets via XDP
 */
void xdp_handle_recv_and_send(struct xdp_server *xdp);

/*
 * Initialize NSD global XDP settings
 *
 *	- load XDP program if configured
 *	- set limits
 */
int xdp_server_init(struct xdp_server *xdp);

/*
 * Cleanup NSD global XDP settings
 *
 *	- unload XDP program if loaded by NSD
 *	- unpin BPF map if pinned and loaded by NSD
 */
int xdp_server_cleanup(struct xdp_server *xdp);

#endif /* XDP_SERVER_H */
