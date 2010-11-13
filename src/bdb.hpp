//  Copyright (c) 2003-2010 Christopher M. Kohlhoff.  All rights reserved.
//  Copyright (c) 2010-2011 Gregory Burd.  All rights reserved.


#define BDB_EID_SELF -3
enum bdb_rep_role { BDB_MASTER, BDB_CLIENT, BDB_UNKNOWN };

struct bdb_settings {

	char *db_file;                   // db filename, where dbfile located.
	char *env_home;                  // db env home dir path
	char *log_home;                  // db log home dir path
	u_int64_t cache_size;            // cache size, default is 256MiB
	u_int32_t txn_lg_bsize;          // transaction log buffer size; default is 4MiB
	u_int32_t page_size;             // underlying database pagesize; default is 4KiB
	bool txn_nosync;                 // when true sacrifice some txn durability for performance; default DB_TXN_NOSYNC is false
	bool log_auto_remove;            // when true catastrophic recovery is impossible
	int dldetect_val;                // deadlock detection frequency; 0 to disable, default is 100 millisecond
	int chkpoint_val;                // checkpoint frequency; 0 for disable, default is every five minutes
	int memp_trickle_val;            // memory pool cleaning frequency; 0 for disable, default 30 seconds
	int memp_trickle_percent;        // percent of the pages in the cache that should be cleaned
	u_int32_t db_flags;              // database open flags
	u_int32_t env_flags;             // env open flags
	char *key;                       // for encrypted data, not related to authentication

	// Berkeley DB XML specific settings.
	u_int32_t xml_flags;

	// Berkeley DB/HA (Replication) specific settings.
	bool is_replicated;             // replication ON/OFF

	char *rep_localhost;             // local host in replication
	int rep_localport;               // local port in replication
	char *rep_remotehost;            // remote host in replication
	int rep_remoteport;              // remote port in replication

	int rep_whoami;                  // replication role, BDB_MASTER/BDB_CLIENT/BDB_UNKNOWN
	int rep_master_eid;              // replication master's eid

	u_int32_t rep_start_policy;
	u_int32_t rep_nsites;

	int rep_ack_policy;

	u_int32_t rep_ack_timeout;       // 50ms
	u_int32_t rep_chkpoint_delay;
	u_int32_t rep_conn_retry;        // 3 seconds
	u_int32_t rep_elect_timeout;     // 5 seconds
	u_int32_t rep_elect_retry;       // 10 seconds
	u_int32_t rep_heartbeat_monitor; // only useful to a client; default 60 seconds
	u_int32_t rep_heartbeat_send;    // only available on a master; default 60 seconds

	bool rep_bulk;

	u_int32_t rep_priority;

	u_int32_t rep_req_max;
	u_int32_t rep_req_min;

	u_int32_t rep_slow_clock;

	u_int32_t rep_limit_gbytes;
	u_int32_t rep_limit_bytes;       // 10MiB
};

extern struct bdb_settings bdb_settings;

void bdb_settings_init(void);
void bdb_env_init(void);
void bdb_dbxml_open(void);
void start_chkpoint_thread(void);
void start_memp_trickle_thread(void);
void start_dl_detect_thread(void);
void bdb_dbxml_close(void);
void bdb_env_close(void);
void bdb_chkpoint(void);
void bdbxml_join_env(void);
