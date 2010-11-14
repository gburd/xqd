//  Copyright (c) 2002,2009 Oracle.  All rights reserved.
//  Copyright (c) 2010-2011 Gregory Burd.  All rights reserved.


#include "xdb.hpp"


namespace xqd {
namespace xdb {

}
using namespace std;

xdb::xdb(bool verbose)
  : data_dir_("xdb"),
    env_dir_("env"),
    log_dir__(NULL),
    cache_size_(256 * 1024 * 1024), // default is 256MB
    txn_lg_bsize_(4 * 1024 * 1024), // default is 4MB
    page_size_(4096),  // default is 4K
    txn_nosync_(false), // default DB_TXN_NOSYNC is off
    log_auto_remove_(false), // default DB_LOG_AUTO_REMOVE is off
    dldetect_val_(100 * 1000), // default is 100 millisecond
    chkpoint_val_(60 * 5),
    memp_trickle_val_(30),
    memp_trickle_percent_(60),
    db_flags_(DB_CREATE
	      | DB_AUTO_COMMIT
	      | DB_MULTIVERSION),
    env_flags(DB_CREATE
	      | DB_INIT_LOCK
	      | DB_THREAD
	      | DB_INIT_MPOOL
	      | DB_INIT_LOG
	      | DB_INIT_TXN
	      | DB_RECOVER),

    xml_flags_(0),

    is_replicated_(false),
    rep_localhost_("127.0.0.1"), // local host in replication
    rep_localport_(REP_LOCAL_PORT),  // local port in replication
    rep_remotehost_(NULL), // local host in replication
    rep_remoteport_(0),  // local port in replication
    rep_whoami_(BDB_UNKNOWN),
    rep_master_eid_(DB_EID_INVALID),
    rep_start_policy_(DB_REP_ELECTION),
    rep_nsites_(2),
    rep_ack_policy_(DB_REPMGR_ACKS_ONE_PEER),

    rep_ack_timeout_(50 * 1000),  // 50ms
    rep_chkpoint_delay_(0),
    rep_conn_retry_(30 * 1000 * 1000), // 3 seconds
    rep_elect_timeout_(5 * 1000 * 1000), // 5 seconds
    rep_elect_retry_(10 * 1000 * 1000), // 10 seconds
    rep_heartbeat_monitor_(80 * 1000 * 1000), // 60 seconds
    rep_heartbeat_send_(60 * 1000 * 1000), // 60 seconds

    rep_bulk_(true),

    rep_priority_(100),

    rep_req_min_(40000),
    rep_req_max_(1280000),

    rep_slow_clock_(100),

    rep_limit_gbytes_(0),
    rep_limit_bytes_(10 * 1024 * 1024) // 10MB
{
  bool ok = false;

  if (is_replicated_)
    ok = setup_bdb_ha_env(verbose);
  else
    ok = setup_bdb_tds_env(verbose);

  if (!ok)
    return NULL;
}

xqd::~xqd()
{
  bdb_chkpoint();
  bdb_xml_close();
  bdb_env_close();
}

void setDataDirectory(std::string dir)
{
  data_dir_ = dir;
}

void setDataEncryptionKey(std::string key)
{
  key_ = key;
}

void setTxnLogDirectory(std::string dir)
{
  log_dir_ = dir;
}

void setRegionDirectory(std::string dir)
{
  regions_dir_ = dir;
}

private void bdb_err_callback(const DB_ENV *env, const char *errpfx, const char *msg)
{
	time_t curr_time = time(NULL);
	char time_str[32];
	strftime(time_str, 32, "%c", localtime(&curr_time));
	fprintf(stderr, "[%s] [%s] \"%s\"\n", errpfx, time_str, msg);
}

private void bdb_msg_callback(const DB_ENV *env, const char *msg)
{
	time_t curr_time = time(NULL);
	char time_str[32];
	strftime(time_str, 32, "%c", localtime(&curr_time));
	fprintf(stderr, "[%s] [%s] \"%s\"\n", PACKAGE, time_str, msg);
}

private bool setup_bdb_tds_env(bool verbose)
{
  int ret;

  // TODO: block block all signals until end of scope, then resend them
  if ((ret = db_env_create(&env, 0)) != 0) {
    fprintf(stderr, "db_env_create: %s\n", db_strerror(ret)); // TODO: change fprintf to "cerr << ..."
    return false;
  }

  if (key_) {
    if ((ret = env->set_encrypt(env, key_, DB_ENCRYPT_AES)) != 0) {
      fprintf(stderr, "set_encrypt: %s\n", db_strerror(ret));
      return false;
    }
  }

  /* Set err and msg display */
  env->set_errpfx(env, PACKAGE);
  env->set_errfile(env, stderr); // TODO: pipe to logging/syslog at some point
  env->set_msgfile(env, stderr);
  env->set_errcall(env, bdb_err_callback);
  env->set_msgcall(env, bdb_msg_callback);

  // Set Berkeley DB to be a bit verbose to let us know what's going on.
  if (verbose) {
    if ((ret = env.set_verbose(env, DB_VERB_FILEOPS_ALL, 1)) != 0) {
      fprintf(stderr, "env->set_verbose[DB_VERB_FILEOPS_ALL]: %s\n",
	      db_strerror(ret));
      return false;
    }
    if ((ret = env.set_verbose(env, DB_VERB_DEADLOCK, 1)) != 0) {
      fprintf(stderr, "env->set_verbose[DB_VERB_DEADLOCK]: %s\n",
	      db_strerror(ret));
      return false;
    }
    if ((ret = env.set_verbose(env, DB_VERB_RECOVERY, 1)) != 0) {
      fprintf(stderr, "env->set_verbose[DB_VERB_RECOVERY]: %s\n",
	      db_strerror(ret));
      return false;
    }
  }

  // Set the transaction log directory.
  if (log_dir_ != NULL){
    // If the log dir doesn't exists, we create it.
    if (0 != access(log_home, F_OK)) {
      if (0 != mkdir(log_home, 0750)) {
	fprintf(stderr, "mkdir log_dir error:[%s]\n", log_dir_);
	return false;
      }
    }
    env.set_lg_dir(env, log_home_);
  }

  // Set memory pool (MPOOL) size.
  if (sizeof(void *) == 4 && cache_size_ > (1024uLL * 1024uLL * 1024uLL * 2uLL)) {
    fprintf(stderr, "32bit box, max 2GB memory pool allowed\n");
    return false;
  }
  env.set_cachesize(env, (u_int32_t) (cache_size_ / (1024uLL * 1024uLL * 1024uLL)),
		      (u_int32_t) (cache_size_ % (1024uLL * 1024uLL * 1024uLL)),
		      (int) (cache_size_ / (1024uLL * 1024uLL * 1024uLL * 4uLL) + 1uLL) );

  /* NOTICE:
     Set DB_TXN_NOSYNC flag because we'll either always run as a two node
     or more replica group, or otherwise we'll depend on the OS not crashing for
     transaction durability. */
  if (txn_nosync_){
    env.set_flags(env, DB_TXN_NOSYNC, 1);
  }

  // Size the lock regions.
  env.set_lk_max_lockers(env, 20000); // TODO: make these 3 calculated/configurable/both at some point
  env.set_lk_max_locks(env, 20000);   // they should be based on expected size of server.
  env.set_lk_max_objects(env, 20000);
  env.set_lk_detect(env, DB_LOCK_DEFAULT);

  // Set at least max active transactions.
  env.set_tx_max(env, 10000); // TODO: config this based on expected size of server.

  // Set transaction log buffer size.
  env.set_lg_bsize(env, txn_lg_bsize);

  /* NOTE:
   *  If set, Berkeley DB will automatically remove log files that are no longer needed.
   *  Automatic log file removal is likely to make catastrophic recovery impossible.
   *  Replication applications will rarely want to configure automatic log file removal as it
   *  increases the likelihood a master will be unable to satisfy a client's request
   for a recent log record. */
  if (!is_replicated_ && log_auto_remove_){
    fprintf(stderr, "log_auto_remove\n");
    env->log_set_config(env, DB_LOG_AUTO_REMOVE, 1);
  }

  // If the environment regions dir doesn't exist, we create it.
  if (0 != access(regions_dir_, F_OK)) {
    if (0 != mkdir(regions_dir_, 0750)) {
      fprintf(stderr, "mkdir regions_dir error:[%s]\n", regions_dir_);
      return false;
    }
  }

  // If the data dir doesn't exist, we create it.
  if (0 != access(data_dir_, F_OK)) {
    if (0 != mkdir(data_dir_, 0750)) {
      fprintf(stderr, "mkdir data_dir error:[%s]\n", data_dir_);
      return false;
    }
  }
  env.set_create_dir(env, data_dir_home_);
  return true;
}

private void bdb_event_callback(DB_ENV *env, u_int32_t which, void *info)
{
  switch (which) {
  case DB_EVENT_PANIC:
    env->errx(env, "evnet: DB_EVENT_PANIC, we got panic, recovery should be run.");
    break;
  case DB_EVENT_REP_CLIENT:
    env->errx(env, "event: DB_EVENT_REP_CLIENT, I<%s:%d> am now a replication client.",
	      bdb_settings.rep_localhost, bdb_settings.rep_localport);
    bdb_settings.rep_whoami = BDB_CLIENT;
    break;
  case DB_EVENT_REP_ELECTED:
    env->errx(env, "event: DB_EVENT_REP_ELECTED, I<%s:%d> has just won an election.",
	      bdb_settings.rep_localhost, bdb_settings.rep_localport);
    break;
  case DB_EVENT_REP_MASTER:
    env->errx(env, "event: DB_EVENT_REP_MASTER, I<%s:%d> am now a replication master.",
	      bdb_settings.rep_localhost, bdb_settings.rep_localport);
    bdb_settings.rep_whoami = BDB_MASTER;
    bdb_settings.rep_master_eid = BDB_EID_SELF;
    break;
  case DB_EVENT_REP_NEWMASTER:
    bdb_settings.rep_master_eid = *(int*)info;
    env->errx(env, "event: DB_EVENT_REP_NEWMASTER, a new master<eid: %d> has been established, "
	      "but not me<%s:%d>", bdb_settings.rep_master_eid,
	      bdb_settings.rep_localhost, bdb_settings.rep_localport);
    break;
  case DB_EVENT_REP_PERM_FAILED:
    env->errx(env, "event: DB_EVENT_REP_PERM_FAILED, insufficient acks, "
	      "the master will flush the txn log buffer");
    break;
  case DB_EVENT_REP_STARTUPDONE:
    if (bdb_settings.rep_whoami == BDB_CLIENT){
      env->errx(env, "event: DB_EVENT_REP_STARTUPDONE, I has completed startup synchronization and"
		" is now processing live log records received from the master.");
    }
    break;
  case DB_EVENT_WRITE_FAILED:
    env->errx(env, "event: DB_EVENT_WRITE_FAILED, I wrote to stable storage failed.");
    break;
  default:
    env->errx(env, "ignoring event %d", which);
  }
}

private bool setup_bdb_ha_env(bool verbose) {
  if (!setup_bdb_tds_env(verbose)) {
    return false;
  }
  if(is_replicated) {
    env_flags |= DB_INIT_REP;

    // Add more verbose messages that to help us to debug/monitor activity.
    if (settings.verbose > 1) {
      if ((ret = env.set_verbose(env, DB_VERB_REPLICATION, 1)) != 0) {
	fprintf(stderr, "env->set_verbose[DB_VERB_REPLICATION]: %s\n",
		db_strerror(ret));
	
      }
    }

    // Set up the event hook, so we can log when interesting HA things happen.
    env->set_event_notify(env, bdb_event_callback);

    // Ack policy can have a great impact in performance, lantency and consistency.
    env->repmgr_set_ack_policy(env, rep_ack_policy_);

    // Timeout configs for the replication manager
    env->rep_set_timeout(env, DB_REP_ACK_TIMEOUT, rep_ack_timeout_);
    env->rep_set_timeout(env, DB_REP_CHECKPOINT_DELAY, rep_chkpoint_delay_);
    env->rep_set_timeout(env, DB_REP_CONNECTION_RETRY, rep_conn_retry_);
    env->rep_set_timeout(env, DB_REP_ELECTION_TIMEOUT, rep_elect_timeout_);
    env->rep_set_timeout(env, DB_REP_ELECTION_RETRY, rep_elect_retry_);

    /* NOTE:
       The "monitor" time should always be at least a little bit longer than the "send" time. */
    env->rep_set_timeout(env, DB_REP_HEARTBEAT_MONITOR, rep_heartbeat_monitor_);
    env->rep_set_timeout(env, DB_REP_HEARTBEAT_SEND, rep_heartbeat_send_);

    /* NOTE:
     *  Bulk transfers simply cause replication messages to accumulate
     *  in a buffer until a triggering event occurs.
     *  Bulk transfer occurs when:
     *  1. Bulk transfers are configured for the master environment, and
     *  2. the message buffer is full or
     *  3. a permanent record (for example, a transaction commit or a checkpoint record)
     *     is placed in the buffer for the replica. */
    env->rep_set_config(env, DB_REP_CONF_BULK, rep_bulk_);
    env->rep_set_priority(env, rep_priority_);
    env->rep_set_request(env, rep_req_min_, rep_req_max_);
    env->rep_set_limit(env, rep_limit_gbytes_, rep_limit_bytes_);

    // Publish the local site communication channel.
    if ((ret = env->repmgr_set_local_site(env, rep_localhost_, rep_localport_, 0)) != 0) {
      fprintf(stderr, "repmgr_set_local_site[%s:%d]: %s\n",
	      rep_localhost_, rep_localport_, db_strerror(ret));
      return false;
    }
    // Add a remote site, most times this is will be the master.
    if(NULL != rep_remotehost_) {
      if ((ret = env->repmgr_add_remote_site(env, rep_remotehost_, rep_remoteport_, NULL, 0)) != 0) {
	fprintf(stderr, "repmgr_add_remote_site[%s:%d]: %s\n", rep_remotehost_, rep_remoteport_, db_strerror(ret));
	return false;
      }
    }
    /* NOTE:
     *  nsite is important for electing, default nvotes is (nsite/2 + 1)
     *  if nsite is equal to 2, then nvotes is 1 */
    if ((ret = env->rep_set_nsites(env, rep_nsites_)) != 0) {
      fprintf(stderr, "rep_set_nsites: %s\n", db_strerror(ret));
      return false;
    }
  }

  if ((ret = env->open(env, regions_dir_, env_flags_, 0)) != 0) {
    if (ret == DB_VERSION_MISMATCH) {
      // TODO: is this an opportunity to upgrade, or a real error?
      fprintf(stderr, "db_env_open: enviornment mismatch -- %s\n", db_strerror(ret));
    } else {
      fprintf(stderr, "db_env_open: %s\n", db_strerror(ret));
    }
    return false;
  }

  if(is_replicated_) {
    // NOTICE: repmgr_start must run after daemon !!!
    if ((ret = env->repmgr_start(env, 3, rep_start_policy_)) != 0) {
      fprintf(stderr, "env->repmgr_start: %s\n", db_strerror(ret));
      return false;
    }
    // Sleep 5 second for electing or client startup.
    if (rep_start_policy_ == DB_REP_ELECTION ||
	rep_start_policy_ == DB_REP_CLIENT) {
      sleep(5);
    }
  }
  return true;
}

void connect(bool verbose)
{
  if (is_replicated_) {
    bool opened = false;

    // Ensure that replicas to get a full set of data from the master, then open db.
    while(!opened) {
      if (BDB_CLIENT == rep_whoami_) {
	xml_flags_ = DB_AUTO_COMMIT;
      } else if (BDB_MASTER == rep_whoami_) {
	xml_flags_ = DB_CREATE | DB_AUTO_COMMIT;
      } else {
	/* FALLTHROUGH -- do nothing in this case. */
      }
      bdb_dbxml_close();

      XmlManager mgr(env, xml_flags_ | DBXML_ADOPT_DBENV);
      mgr.setDefaultPageSize(page_size_);
      opened = true;
    }
}

void disconnect(void)
{
}

void recover(void)
{
}

void backup(void)
{
}

void archive(void)
{
}

void checkpoint(void)
{
}

/* for atexit cleanup */
void bdb_xml_close(XmlManager &mgr)
{
  int ret = 0;

  if (mgr != NULL) {
    ret = mgr->close(0);
    if (0 != ret){
      fprintf(stderr, "mgr->close: %s\n", db_strerror(ret));
    }else{
      mgr = NULL;
      fprintf(stderr, "mgr->close: OK\n");
    }
  }
}

/* for atexit cleanup */
void bdb_env_close(void)
{
  int ret = 0;
  if (env != NULL) {
    ret = env->close(env, 0);
    if (0 != ret){
      fprintf(stderr, "env->close: %s\n", db_strerror(ret));
    }else{
      env = NULL;
      fprintf(stderr, "env->close: OK\n");
    }
  }
}

/* for atexit cleanup */
void bdb_chkpoint(void)
{
  int ret = 0;
  if (env != NULL){
    ret = env->txn_checkpoint(env, 0, 0, 0);
    if (0 != ret){
      fprintf(stderr, "env->txn_checkpoint: %s\n", db_strerror(ret));
    }else{
      fprintf(stderr, "env->txn_checkpoint: OK\n");
    }
  }
}

} // namespace xdb
} // namespace xqd
