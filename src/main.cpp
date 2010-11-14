//
// main.cpp
// ~~~~~~~~~~~~
//
// Copyright (c) 2010 Gregory Burd, All Rights Reserved.
//
// Portions:
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//

#include "config.h"

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "server.hpp"
#include "file_handler.hpp"
#include "xq_handler.hpp"
#include "xdb.hpp"


// GLOBALS
bool daemon_quit = false;


#if defined(_WIN32)

boost::function0<void> console_ctrl_function;

BOOL WINAPI console_ctrl_handler(DWORD ctrl_type)
{
  switch (ctrl_type)
  {
  case CTRL_C_EVENT:
  case CTRL_BREAK_EVENT:
  case CTRL_CLOSE_EVENT:
  case CTRL_SHUTDOWN_EVENT:
    console_ctrl_function();
    return TRUE;
  default:
    return FALSE;
  }
}

int main(int argc, char* argv[])
{
  settings.port = 34933;
  settings.verbose = 0;
  settings.num_threads = 1;
  try
  {
    // Check command line arguments.
    if (argc != 4)
    {
      std::cerr << "Usage: http_server <address> <port> <doc_root>\n";
      std::cerr << "  For IPv4, try:\n";
      std::cerr << "    http_server 0.0.0.0 80 .\n";
      std::cerr << "  For IPv6, try:\n";
      std::cerr << "    http_server 0::0 80 .\n";
      return 1;
    }
    setbuf(stderr, NULL); // Set stderr non-buffering (useful for daemontools)

    boost::asio::io_service io_service;

    // Launch the initial server coroutine.
    xqd::http::server(io_service, argv[1], argv[2],
        xqd::http::file_handler(argv[3]))();

    // Set console control handler to allow server to be stopped.
    console_ctrl_function = boost::bind(
        &boost::asio::io_service::stop, &io_service);
    SetConsoleCtrlHandler(console_ctrl_handler, TRUE);

    xqd::xqd mgr = new xqd::xdb(true);
    // Start checkpoint and deadlock detect threads.
    start_chkpoint_thread();
    start_memp_trickle_thread();
    start_dl_detect_thread();

    // Now, connect to (open) an XML container.
    mgr.connect("test.dbxml");


    // Run the server until stopped.
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "exception: " << e.what() << "\n";
  }

  return 0;
}

#endif // defined(_WIN32)

