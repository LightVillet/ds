
struct context_common_s {
};

struct context_common_s context_common;

// ATTN context
struct context_attn_s {
  int  ready;
  context_attn_s() {
     ready = 0;
  }
};

struct context_bully {
	bool waiting_coordinator = false;
	bool waiting_ping = false;
	int start_time = 0;
	int delta = 50;
} context_bully;

struct context_attn_s context_attn;

