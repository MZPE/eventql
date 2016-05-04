/**
 * Copyright (c) 2015 - The CM Authors <legal@clickmatcher.com>
 *   All Rights Reserved.
 *
 * This file is CONFIDENTIAL -- Distribution or duplication of this material or
 * the information contained herein is strictly forbidden unless prior written
 * permission is obtained.
 */
#pragma once
#include "stx/stdtypes.h"
#include "stx/autoref.h"
#include "stx/option.h"
#include "eventql/core/TSDBService.h"
#include "eventql/mapreduce/MapReduceTaskBuilder.h"
#include "eventql/mapreduce/MapReduceScheduler.h"
#include <jsapi.h>

using namespace stx;

namespace zbase {

class JavaScriptContext : public RefCounted {
public:
  static const size_t kDefaultMemLimit = 1024 * 1024 * 128;
  static JSClass kGlobalJSClass;

  JavaScriptContext(
      const String& customer,
      RefPtr<MapReduceJobSpec> job,
      TSDBService* tsdb,
      RefPtr<MapReduceTaskBuilder> task_builder,
      RefPtr<MapReduceScheduler> scheduler,
      size_t memlimit = kDefaultMemLimit);

  ~JavaScriptContext();

  void loadProgram(const String& program);

  void loadClosure(
      const String& source,
      const String& globals,
      const String& params);

  void callMapFunction(
      const String& json_string,
      Vector<Pair<String, String>>* tuples);

  void callReduceFunction(
      const String& key,
      const Vector<String>& values,
      Vector<Pair<String, String>>* tuples);

  String callSerializeFunction(
      const String& key,
      const String& value);

protected:

  struct ReduceCollectionIter {
    static JSClass kJSClass;
    static bool hasNext(JSContext* ctx, unsigned argc, JS::Value* vp);
    static bool getNext(JSContext* ctx, unsigned argc, JS::Value* vp);
    size_t cur;
    const Vector<String>* data;
  };

  void storeError(
      const String& error,
      size_t line = 0,
      size_t column = 0);

  static void dispatchError(
      JSContext* ctx,
      const char* message,
      JSErrorReport* report);

  static bool dispatchLog(
      JSContext* ctx,
      unsigned argc,
      JS::Value* vp);

  static bool returnResult(
      JSContext* ctx,
      unsigned argc,
      JS::Value* vp);

  static bool listPartitions(
      JSContext* ctx,
      unsigned argc,
      JS::Value* vp);

  static bool executeMapReduce(
      JSContext* ctx,
      unsigned argc,
      JS::Value* vp);

  void enumerateTuples(
      JS::RootedValue* src,
      Vector<Pair<String, String>>* dst) const;

  String customer_;
  RefPtr<MapReduceJobSpec> job_;
  TSDBService* tsdb_;
  RefPtr<MapReduceTaskBuilder> task_builder_;
  RefPtr<MapReduceScheduler> scheduler_;
  JSRuntime* runtime_;
  JSContext* ctx_;
  JS::PersistentRooted<JSObject*> global_;
  String current_error_;
  size_t current_error_line_;
  size_t current_error_column_;
};

} // namespace zbase
