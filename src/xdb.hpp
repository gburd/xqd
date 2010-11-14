//  Copyright (c) 2002,2009 Oracle.  All rights reserved.
//  Copyright (c) 2010-2011 Gregory Burd.  All rights reserved.


#ifndef __XDB_HPP
#define	__XDB_HPP

#include "dbxml/DbXml.hpp"
#include <vector>
#include <string>

#define BDB_EID_SELF -3

namespace xqd {

class xdb
{
public:
  xdb(bool verbose);
  virtual ~xdb();

  void setDataDirectory(std::string dir);
  void setDataEncryptionKey(std::string key);
  void setTxnLogDirectory(std::string dir);
  void setRegionDirectory(std::string dir);
  void connect(bool verbose);
  void disconnect(void);
  void recover(void);
  void backup(void);
  void archive(void);
  void checkpoint(void);

protected:
  DB_ENV &env;                      // the database environment

private:
  // Berkeley DB settings.
  std::string data_dir_;            // db filename, where dbfile located.
  std::string regions_dir_;         // db env home dir path
  std::string log_dir_;             // db log home dir path
  u_int64_t cache_size_;            // cache size, default is 256MiB
  u_int32_t txn_lg_bsize_;          // transaction log buffer size; default is 4MiB
  u_int32_t page_size_;             // underlying database pagesize; default is 4KiB
  bool txn_nosync_;                 // when true sacrifice some txn durability for performance; default DB_TXN_NOSYNC is false
  bool log_auto_remove_;            // when true catastrophic recovery is impossible
  int dldetect_val_;                // deadlock detection frequency; 0 to disable, default is 100 millisecond
  int chkpoint_val_;                // checkpoint frequency; 0 for disable, default is every five minutes
  int memp_trickle_val_;            // memory pool cleaning frequency; 0 for disable, default 30 seconds
  int memp_trickle_percent_;        // percent of the pages in the cache that should be cleaned
  u_int32_t env_flags_;             // env open flags
  char *key_;                       // for encrypted data, not related to authentication

  // Berkeley DB XML settings.
  u_int32_t xml_flags_;             // database open flags


  // Berkeley DB/HA (Replication) specific settings.
  bool is_replicated_;             // replication ON/OFF

  char *rep_localhost_;            // local host in replication
  int rep_localport_;              // local port in replication
  char *rep_remotehost_;           // remote host in replication
  int rep_remoteport_;             // remote port in replication

  int rep_whoami_;                 // replication role, BDB_MASTER/BDB_CLIENT/BDB_UNKNOWN
  int rep_master_eid_;             // replication master's eid

  u_int32_t rep_start_policy_;
  u_int32_t rep_nsites_;

  int rep_ack_policy_;

  u_int32_t rep_ack_timeout_;       // 50ms
  u_int32_t rep_chkpoint_delay_;
  u_int32_t rep_conn_retry_;        // 3 seconds
  u_int32_t rep_elect_timeout_;     // 5 seconds
  u_int32_t rep_elect_retry_;       // 10 seconds
  u_int32_t rep_heartbeat_monitor_; // only useful to a client; default 60 seconds
  u_int32_t rep_heartbeat_send_;    // only available on a master; default 60 seconds

  bool rep_bulk_;

  u_int32_t rep_priority_;

  u_int32_t rep_req_max_;
  u_int32_t rep_req_min_;

  u_int32_t rep_slow_clock_;

  u_int32_t rep_limit_gbytes_;
  u_int32_t rep_limit_bytes_;       // 10MiB

  enum bdb_rep_role { BDB_MASTER, BDB_CLIENT, BDB_UNKNOWN };
};

} // namespace xqd

#endif // XDB_HPP
