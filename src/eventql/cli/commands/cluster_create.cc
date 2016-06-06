/**
 * Copyright (c) 2016 zScale Technology GmbH <legal@zscale.io>
 * Authors:
 *   - Paul Asmuth <paul@zscale.io>
 *   - Laura Schlimmer <laura@zscale.io>
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
#include <eventql/cli/commands/cluster_create.h>
#include <eventql/util/cli/flagparser.h>
#include "eventql/config/config_directory_zookeeper.h"

namespace eventql {
namespace cli {

const String ClusterCreate::kName_ = "cluster-create";
const String ClusterCreate::kDescription_ = "cluster add server"; //FIXME

ClusterCreate::ClusterCreate(
    RefPtr<ProcessConfig> process_cfg) :
    process_cfg_(process_cfg) {}

Status ClusterCreate::execute(
    const std::vector<std::string>& argv,
    FileInputStream* stdin_is,
    OutputStream* stdout_os,
    OutputStream* stderr_os) {
  auto zookeeper_addr = process_cfg_->getString("evqlctl", "zookeeper_addr");
  if (zookeeper_addr.isEmpty()) {
    stderr_os->write("Error: no zookeeper address provided"); //FIXME
    return Status(eFlagError);
  }

  ::cli::FlagParser flags;
  flags.defineFlag(
      "cluster_name",
      ::cli::FlagParser::T_STRING,
      true,
      NULL,
      NULL,
      "node name",
      "<string>");

  try {
    flags.parseArgv(argv);

   auto cdir = mkScoped(
        new ZookeeperConfigDirectory(
              zookeeper_addr.get(),
              None<String>(),
              ""));

    cdir->startAndJoin(flags.getString("cluster_name"));

    ClusterConfig cfg;
    cdir->updateClusterConfig(cfg);

    cdir->stop();

  } catch (const Exception& e) {
    stderr_os->write(StringUtil::format(
        "$0: $1\n",
        e.getTypeName(),
        e.getMessage()));
    return Status(e);
  }

  return Status::success();
}

const String& ClusterCreate::getName() const {
  return kName_;
}

const String& ClusterCreate::getDescription() const {
  return kDescription_;
}

const String& ClusterCreate::printHelp() const {
  auto help_str = StringUtil::format("$0: $1", kName_, kDescription_);
  return help_str;
}

} // namespace cli
} // namespace eventql






