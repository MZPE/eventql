/**
 * Copyright (c) 2016 DeepCortex GmbH <legal@eventql.io>
 * Authors:
 *   - Laura Schlimmer <laura@eventql.io>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License ("the license") as
 * published by the Free Software Foundation, either version 3 of the License,
 * or any later version.
 *
 * In accordance with Section 7(e) of the license, the licensing of the Program
 * under the license does not imply a trademark license. Therefore any rights,
 * title and interest in our trademarks remain entirely with us.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You can be released from the requirements of the license by purchasing a
 * commercial license. Buying such a license is mandatory as soon as you develop
 * commercial activities involving this program without disclosing the source
 * code of your own applications
 */
#include <iostream>
#include <sstream>
#include <iomanip>
#include <unistd.h>
#include "eventql/eventql.h"
#include "eventql/util/application.h"
#include "eventql/util/io/inputstream.h"
#include "eventql/util/io/TerminalOutputStream.h"
#include "eventql/util/cli/flagparser.h"
#include "eventql/cli/benchmark.h"

static const char kEraseEscapeSequence[] = "\e[2K";

int main(int argc, const char** argv) {
  cli::FlagParser flags;

  flags.defineFlag(
      "host",
      cli::FlagParser::T_STRING,
      false,
      "h",
      "localhost",
      "eventql server hostname",
      "<host>");

  flags.defineFlag(
      "port",
      cli::FlagParser::T_INTEGER,
      false,
      "p",
      "9175",
      "eventql server port",
      "<port>");

  flags.defineFlag(
      "database",
      cli::FlagParser::T_STRING,
      false,
      "d",
      NULL,
      "database",
      "<db>");

  flags.defineFlag(
      "connections",
      cli::FlagParser::T_INTEGER,
      false,
      "c",
      "10",
      "number of connections",
      "<threads>");

  flags.defineFlag(
      "rate",
      cli::FlagParser::T_INTEGER,
      false,
      "r",
      "1",
      "number of requests per second",
      "<rate>");

  flags.defineFlag(
      "num",
      cli::FlagParser::T_INTEGER,
      false,
      "n",
      NULL,
      "total number of request (inifite if unset)",
      "<rate>");

  flags.defineFlag(
      "help",
      cli::FlagParser::T_SWITCH,
      false,
      "?",
      NULL,
      "help",
      "<help>");

  flags.defineFlag(
      "version",
      cli::FlagParser::T_SWITCH,
      false,
      "v",
      NULL,
      "print version",
      "<switch>");

  Application::init();

  try {
    flags.parseArgv(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << StringUtil::format("ERROR: $0\n", e.what());
    return 1;
  }

  Vector<String> cmd_argv = flags.getArgv();
  bool print_help = flags.isSet("help");
  bool print_version = flags.isSet("version");
  if (print_version || print_help) {
    std::cout << StringUtil::format(
        "EventQL $0 ($1)\n"
        "Copyright (c) 2016, DeepCortex GmbH. All rights reserved.\n\n",
        eventql::kVersionString,
        eventql::kBuildID);
  }

  if (print_version) {
    return 1;
  }

  if (print_help) {
    std::cout <<
        "Usage: $ evqlslap [OPTIONS]\n\n"
        "   -h, --host <hostname>     Set the EventQL server hostname\n"
        "   -p, --port <port>         Set the EventQL server port\n"
        "   -D, --database <db>       Select a database\n"
        "   -c, --connections <num>   Number of concurrent connections\n"
        "   -r, --rate <rate>         Maximum rate of requests in RPS\n"
        "   -n, --num <num>           Maximum total number of request (default is inifite)\n"
        "   -?, --help                Display the help text and exit\n"
        "   -v, --version             Display the version of this binary and exit\n";

    return 1;
  }

  auto request_handler = []() {
    usleep(1000);
    return ReturnCode::success();
  };

  bool line_dirty = false;
  const bool is_tty = ::isatty(STDOUT_FILENO);
  auto num = flags.isSet("num") ? flags.getInt("num") : -1;

  auto on_progress = [&num, &is_tty, &line_dirty](
      eventql::cli::BenchmarkStats* stats) {
    std::stringstream line;
    if (line_dirty && is_tty) {
      line << kEraseEscapeSequence << "\r";
    }

    line << std::fixed
         << "Running..."
         << " rate="
         << std::setprecision(2) << stats->getRollingRPS()
         << "r/s"
         << ", avg_runtime="
         << std::setprecision(2)
         << stats->getRollingAverageRuntime() / double(kMicrosPerMilli)
         << "ms"
         << ", total="
         << stats->getTotalRequestCount();

    if (num > 0) {
      line << " ("
           << std::setprecision(2)
           << double(stats->getTotalRequestCount()) / double(num) * 100.0
           << "%)";
    }

    line << ", running="
         << stats->getRunningRequestCount()
         << ", errors="
         << stats->getTotalErrorCount()
         << "("
         << std::setprecision(2) << stats->getTotalErrorRate() * 100.0f
         << "%)";

    if (!is_tty) {
      line << "\n";
    }

    line_dirty = true;
    std::cerr << line.str();
  };

  eventql::cli::Benchmark benchmark(
      flags.getInt("connections"),
      flags.getInt("rate"),
      num);

  benchmark.setRequestHandler(request_handler);
  benchmark.setProgressCallback(on_progress);
  benchmark.setProgressRateLimit(kMicrosPerSecond / 10);

  auto rc = benchmark.connect(
      flags.getString("host"),
      flags.getInt("port"),
      {});

  if (rc.isSuccess()) {
    rc = benchmark.run();
  }

  if (rc.isSuccess()) {
    std::cout << "\nsuccess" << std::endl;
    return 0;
  } else {
    std::cerr << "ERROR: " << rc.getMessage() << std::endl;
    return 1;
  }

  return 0;
}

