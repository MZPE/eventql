/**
 * This file is part of the "libfnord" project
 *   Copyright (c) 2015 Paul Asmuth
 *
 * FnordMetric is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef _FNORD_TSDB_STREAMCHUNK_H
#define _FNORD_TSDB_STREAMCHUNK_H
#include <stx/stdtypes.h>
#include <stx/option.h>
#include <stx/UnixTime.h>
#include <stx/protobuf/MessageSchema.h>
#include <stx/http/httpconnectionpool.h>
#include <zbase/core/Table.h>
#include <zbase/core/RecordRef.h>
#include <zbase/core/PartitionInfo.pb.h>
#include <zbase/core/PartitionSnapshot.h>
#include <zbase/core/PartitionWriter.h>
#include <zbase/core/PartitionReader.h>
#include <zbase/core/ReplicationScheme.h>
#include <cstable/CSTableReader.h>

using namespace stx;

namespace zbase {
class Table;
class PartitionReplication;

using PartitionKey =
    std::tuple<
        String,     // namespace
        String,     // table
        SHA1Hash>;  // partition

class Partition : public RefCounted {
public:

  static RefPtr<Partition> create(
      const String& tsdb_namespace,
      RefPtr<Table> table,
      const SHA1Hash& partition_key,
      const String& db_path);

  static RefPtr<Partition> reopen(
      const String& tsdb_namespace,
      RefPtr<Table> table,
      const SHA1Hash& partition_key,
      const String& db_path);

  Partition(
      RefPtr<PartitionSnapshot> snap,
      RefPtr<Table> table);

  ~Partition();

  SHA1Hash uuid() const;

  RefPtr<PartitionWriter> getWriter();
  RefPtr<PartitionReader> getReader();
  RefPtr<PartitionSnapshot> getSnapshot();
  RefPtr<Table> getTable();
  PartitionInfo getInfo() const;

  RefPtr<PartitionReplication> getReplicationStrategy(
      RefPtr<ReplicationScheme> repl_scheme,
      http::HTTPConnectionPool* http);

protected:
  PartitionSnapshotRef head_;
  RefPtr<Table> table_;
  RefPtr<PartitionWriter> writer_;
  std::mutex writer_lock_;
};

}
#endif
