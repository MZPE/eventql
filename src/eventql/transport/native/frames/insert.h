/**
 * Copyright (c) 2016 DeepCortex GmbH <legal@eventql.io>
 * Authors:
 *   - Paul Asmuth <paul@eventql.io>
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
#pragma once
#include <string>
#include <vector>
#include "eventql/eventql.h"
#include "eventql/util/io/inputstream.h"
#include "eventql/util/io/outputstream.h"
#include "eventql/util/return_code.h"

namespace eventql {
namespace native_transport {

class InsertFrame {
public:

  static const uint16_t kOpcode = EVQL_OP_INSERT;

  InsertFrame();

  void setDatabase(const std::string& database);
  void setTable(const std::string& table);
  void setRecordEncoding(uint64_t encoding);
  void setRecordEncodingInfo(const std::string& str);
  void addRecord(const std::string& record);

  const std::string& getDatabase() const;
  const std::string& getTable() const;
  uint64_t getRecordEncoding() const;
  const std::string& getRecordEncodingInfo() const;
  const std::vector<std::string>& getRecords() const;

  ReturnCode parseFrom(InputStream* is);
  ReturnCode writeTo(OutputStream* os) const;
  void clear();

protected:
  uint64_t flags_;
  std::string database_;
  std::string table_;
  uint64_t record_encoding_;
  std::string record_encoding_info_;
  std::vector<std::string> records_;
};

} // namespace native_transport
} // namespace eventql

