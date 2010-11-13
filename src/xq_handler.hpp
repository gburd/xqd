//  Copyright (c) 2003-2010 Christopher M. Kohlhoff.  All rights reserved.
//  Copyright (c) 2010-2011 Gregory Burd.  All rights reserved.

#ifndef HTTP_SERVER4_xq_HANDLER_HPP
#define HTTP_SERVER4_xq_HANDLER_HPP

#include <string>

namespace xqd {
namespace http {

struct reply;
struct request;

/// The common handler for all incoming requests.
class xq_handler
{
public:
  /// Construct with a directory containing files to be served.
  explicit xq_handler(const std::string& doc_root);

  /// Handle a request and produce a reply.
  void operator()(const request& req, reply& rep);

private:
  /// The directory containing the files to be served.
  //TODO: DbXmlEnv  // std::string doc_root_;

  /// Perform URL-decoding on a string. Returns false if the encoding was
  /// invalid.
  static bool url_decode(const std::string& in, std::string& out);
};

} // namespace server4
} // namespace http

#endif // HTTP_SERVER4_xq_HANDLER_HPP

