//  Copyright (c) 2010-2011 Gregory Burd.  All rights reserved.

#define PACKAGE "xqd"
#define MAX_VERBOSITY_LEVEL 2
#define REP_LOCAL_PORT 16180

struct settings {
	int port;
	int verbose;
	int num_threads;
};

extern struct settings settings;
extern bool daemon_quit;
