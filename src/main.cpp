//
// main.cpp
// ~~~~~~~~~~~~
//
// Copyright (c) 2010 Gregory Burd, All Rights Reserved.
//
// Portions:
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "server.hpp"
#include "file_handler.hpp"


int main(int argc, char* argv[])
{
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

    boost::asio::io_service io_service;

    // Launch the initial server coroutine.
    http::server4::server(io_service, argv[1], argv[2],
        http::server4::file_handler(argv[3]))();

    // Set console control handler to allow server to be stopped.
    console_ctrl_function = boost::bind(
        &boost::asio::io_service::stop, &io_service);
    SetConsoleCtrlHandler(console_ctrl_handler, TRUE);

    // Run the server until stopped.
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "exception: " << e.what() << "\n";
  }

  return 0;
}
