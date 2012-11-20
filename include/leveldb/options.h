// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_INCLUDE_OPTIONS_H_
#define STORAGE_LEVELDB_INCLUDE_OPTIONS_H_

#include <stddef.h>
#include <string>
#include <stdint.h>
#include "leveldb/slice.h"

namespace leveldb {

class Cache;
class Comparator;
class Env;
class FilterPolicy;
class Logger;
class Snapshot;
class Statistics;

// DB contents are stored in a set of blocks, each of which holds a
// sequence of key,value pairs.  Each block may be compressed before
// being stored in a file.  The following enum describes which
// compression method (if any) is used to compress a block.
enum CompressionType {
  // NOTE: do not change the values of existing entries, as these are
  // part of the persistent format on disk.
  kNoCompression     = 0x0,
  kSnappyCompression = 0x1,
  kZlibCompression = 0x2,
  kBZip2Compression = 0x3
};

// Compression options for different compression algorithms like Zlib
struct CompressionOptions {
  int window_bits;
  int level;
  int strategy;
  CompressionOptions():window_bits(-14),
                       level(-1),
                       strategy(0){}
  CompressionOptions(int wbits, int lev, int strategy):window_bits(wbits),
                                                       level(lev),
                                                       strategy(strategy){}
};

// Options to control the behavior of a database (passed to DB::Open)
struct Options {
  // -------------------
  // Parameters that affect behavior

  // Comparator used to define the order of keys in the table.
  // Default: a comparator that uses lexicographic byte-wise ordering
  //
  // REQUIRES: The client must ensure that the comparator supplied
  // here has the same name and orders keys *exactly* the same as the
  // comparator provided to previous open calls on the same DB.
  const Comparator* comparator;

  // If true, the database will be created if it is missing.
  // Default: false
  bool create_if_missing;

  // If true, an error is raised if the database already exists.
  // Default: false
  bool error_if_exists;

  // If true, the implementation will do aggressive checking of the
  // data it is processing and will stop early if it detects any
  // errors.  This may have unforeseen ramifications: for example, a
  // corruption of one DB entry may cause a large number of entries to
  // become unreadable or for the entire DB to become unopenable.
  // Default: false
  bool paranoid_checks;

  // Use the specified object to interact with the environment,
  // e.g. to read/write files, schedule background work, etc.
  // Default: Env::Default()
  Env* env;

  // Any internal progress/error information generated by the db will
  // be written to info_log if it is non-NULL, or to a file stored
  // in the same directory as the DB contents if info_log is NULL.
  // Default: NULL
  Logger* info_log;

  // -------------------
  // Parameters that affect performance

  // Amount of data to build up in memory (backed by an unsorted log
  // on disk) before converting to a sorted on-disk file.
  //
  // Larger values increase performance, especially during bulk loads.
  // Up to max_write_buffer_number write buffers may be held in memory 
  // at the same time,
  // so you may wish to adjust this parameter to control memory usage.
  // Also, a larger write buffer will result in a longer recovery time
  // the next time the database is opened.
  //
  // Default: 4MB
  size_t write_buffer_size;

  // The maximum number of write buffers that are built up in memory.
  // The default is 2, so that when 1 write buffer is being flushed to 
  // storage, new writes can continue to the other write buffer.
  // Default: 2
  int max_write_buffer_number;

  // Number of open files that can be used by the DB.  You may need to
  // increase this if your database has a large working set (budget
  // one open file per 2MB of working set).
  //
  // Default: 1000
  int max_open_files;

  // Control over blocks (user data is stored in a set of blocks, and
  // a block is the unit of reading from disk).

  // If non-NULL, use the specified cache for blocks.
  // If NULL, leveldb will automatically create and use an 8MB internal cache.
  // Default: NULL
  Cache* block_cache;

  // Approximate size of user data packed per block.  Note that the
  // block size specified here corresponds to uncompressed data.  The
  // actual size of the unit read from disk may be smaller if
  // compression is enabled.  This parameter can be changed dynamically.
  //
  // Default: 4K
  size_t block_size;

  // Number of keys between restart points for delta encoding of keys.
  // This parameter can be changed dynamically.  Most clients should
  // leave this parameter alone.
  //
  // Default: 16
  int block_restart_interval;


  // Compress blocks using the specified compression algorithm.  This
  // parameter can be changed dynamically.
  //
  // Default: kSnappyCompression, which gives lightweight but fast
  // compression.
  //
  // Typical speeds of kSnappyCompression on an Intel(R) Core(TM)2 2.4GHz:
  //    ~200-500MB/s compression
  //    ~400-800MB/s decompression
  // Note that these speeds are significantly faster than most
  // persistent storage speeds, and therefore it is typically never
  // worth switching to kNoCompression.  Even if the input data is
  // incompressible, the kSnappyCompression implementation will
  // efficiently detect that and will switch to uncompressed mode.
  CompressionType compression;

  // Different levels can have different compression policies. There
  // are cases where most lower levels would like to quick compression
  // algorithm while the higher levels (which have more data) use
  // compression algorithms that have better compression but could
  // be slower. This array, if non NULL, should have an entry for
  // each level of the database. This array, if non NULL, overides the
  // value specified in the previous field 'compression'. The caller is
  // reponsible for allocating memory and initializing the values in it
  // before invoking Open(). The caller is responsible for freeing this
  // array and it could be freed anytime after the return from Open().
  // This could have been a std::vector but that makes the equivalent
  // java/C api hard to construct.
  CompressionType* compression_per_level;

  //different options for compression algorithms
  CompressionOptions compression_opts;

  // If non-NULL, use the specified filter policy to reduce disk reads.
  // Many applications will benefit from passing the result of
  // NewBloomFilterPolicy() here.
  //
  // Default: NULL
  const FilterPolicy* filter_policy;

  // Number of levels for this database
  int num_levels;

  // Number of files to trigger level-0 compaction. A value <0 means that
  // level-0 compaction will not be triggered by number of files at all.
  int level0_file_num_compaction_trigger;

  // Soft limit on number of level-0 files. We slow down writes at this point.
  // A value <0 means that no writing slow down will be triggered by number
  // of files in level-0.
  int level0_slowdown_writes_trigger;

  // Maximum number of level-0 files.  We stop writes at this point.
  int level0_stop_writes_trigger;

  // Maximum level to which a new compacted memtable is pushed if it
  // does not create overlap.  We try to push to level 2 to avoid the
  // relatively expensive level 0=>1 compactions and to avoid some
  // expensive manifest file operations.  We do not push all the way to
  // the largest level since that can generate a lot of wasted disk
  // space if the same key space is being repeatedly overwritten.
  int max_mem_compaction_level;

  // Target file size for compaction.
  // target_file_size_base is per-file size for level-1.
  // Target file size for level L can be calculated by
  // target_file_size_base * (target_file_size_multiplier ^ (L-1))
  // For example, if target_file_size_base is 2MB and
  // target_file_size_multiplier is 10, then each file on level-1 will
  // be 2MB, and each file on level 2 will be 20MB,
  // and each file on level-3 will be 200MB.

  // by default target_file_size_base is 2MB.
  int target_file_size_base;
  // by default target_file_size_multiplier is 1, which means
  // by default files in different levels will have similar size.
  int target_file_size_multiplier;

  // Control maximum total data size for a level.
  // max_bytes_for_level_base is the max total for level-1.
  // Maximum number of bytes for level L can be calculated as
  // (max_bytes_for_level_base) * (max_bytes_for_level_multiplier ^ (L-1))
  // For example, if max_bytes_for_level_base is 20MB, and if
  // max_bytes_for_level_multiplier is 10, total data size for level-1
  // will be 20MB, total file size for level-2 will be 200MB,
  // and total file size for level-3 will be 2GB.


  // by default 'max_bytes_for_level_base' is 10MB.
  uint64_t max_bytes_for_level_base;
  // by default 'max_bytes_for_level_base' is 10.
  int max_bytes_for_level_multiplier;

  // Maximum number of bytes in all compacted files.  We avoid expanding
  // the lower level file set of a compaction if it would make the
  // total compaction cover more than
  // (expanded_compaction_factor * targetFileSizeLevel()) many bytes.
  int expanded_compaction_factor;

  // Control maximum bytes of overlaps in grandparent (i.e., level+2) before we
  // stop building a single file in a level->level+1 compaction.
  int max_grandparent_overlap_factor;

  // If non-null, then we should collect metrics about database operations
  Statistics* statistics;

  // If true, then the contents of data files are not synced
  // to stable storage. Their contents remain in the OS buffers till the
  // OS decides to flush them. This option is good for bulk-loading
  // of data. Once the bulk-loading is complete, please issue a
  // sync to the OS to flush all dirty buffesrs to stable storage.
  // Default: false
  bool disableDataSync;

  // If true, then every store to stable storage will issue a fsync.
  // If false, then every store to stable storage will issue a fdatasync.
  // This parameter should be set to true while storing data to
  // filesystem like ext3 which can lose files after a reboot.
  // Default: false
  bool use_fsync;

  // This number controls how often a new scribe log about
  // db deploy stats is written out.
  // -1 indicates no logging at all.
  // Default value is 1800 (half an hour).
  int db_stats_log_interval;

  // This specifies the log dir.
  // If it is empty, the log files will be in the same dir as data.
  // If it is non empty, the log files will be in the specified dir,
  // and the db data dir's absolute path will be used as the log file
  // name's prefix.
  std::string db_log_dir;

  // Disable compaction triggered by seek.
  // With bloomfilter and fast storage, a miss on one level
  // is very cheap if the file handle is cached in table cache
  // (which is true if max_open_files is large).
  bool disable_seek_compaction;

  // The periodicity when obsolete files get deleted. The default
  // value is 0 which means that obsolete files get removed after
  // every compaction run.
  uint64_t delete_obsolete_files_period_micros;
  
  // Maximum number of concurrent background compactions.
  // Default: 1
  int max_background_compactions;

  // Specify the maximal size of the info log file. If the log file
  // is larger than `max_log_file_size`, a new info log file will
  // be created.
  // If max_log_file_size == 0, all logs will be written to one
  // log file.
  size_t max_log_file_size;

  // Puts are delayed when any level has a compaction score that
  // exceeds rate_limit. This is ignored when <= 1.0.
  double rate_limit;

  // Disable block cache. If this is set to false,
  // then no block cache should be used, and the block_cache should
  // point to a NULL object.
  bool no_block_cache;

  // Number of shards used for table cache.
  int table_cache_numshardbits;

  // Create an Options object with default values for all fields.
  Options();

  void Dump(Logger * log) const;

  // This method allows an application to modify/delete a key-value at 
  // the time of compaction. The compaction process invokes this
  // method for every kv that is being compacted. A return value
  // of false indicates that the kv should be preserved in the
  // output of this compaction run and a return value of true
  // indicates that this key-value should be removed from the 
  // output of the compaction.  The application can inspect
  // the existing value of the key, modify it if needed and
  // return back the new value for this key. The application
  // should allocate memory for the Slice object that is used to
  // return the new value and the leveldb framework will
  // free up that memory.
  // The compaction_filter_args, if specified here, are passed
  // back to the invocation of the CompactionFilter.
  void* compaction_filter_args;
  bool (*CompactionFilter)(void* compaction_filter_args, 
         int level, const Slice& key, 
         const Slice& existing_value, Slice** new_value);

  // Disable automatic compactions. Manual compactions can still
  // be issued on this database.
  bool disable_auto_compactions;
};

// Options that control read operations
struct ReadOptions {
  // If true, all data read from underlying storage will be
  // verified against corresponding checksums.
  // Default: false
  bool verify_checksums;

  // Should the data read for this iteration be cached in memory?
  // Callers may wish to set this field to false for bulk scans.
  // Default: true
  bool fill_cache;

  // If "snapshot" is non-NULL, read as of the supplied snapshot
  // (which must belong to the DB that is being read and which must
  // not have been released).  If "snapshot" is NULL, use an impliicit
  // snapshot of the state at the beginning of this read operation.
  // Default: NULL
  const Snapshot* snapshot;

  ReadOptions()
      : verify_checksums(false),
        fill_cache(true),
        snapshot(NULL) {
  }
  ReadOptions(bool cksum, bool cache) :
              verify_checksums(cksum), fill_cache(cache),
              snapshot(NULL) {
  }
};

// Options that control write operations
struct WriteOptions {
  // If true, the write will be flushed from the operating system
  // buffer cache (by calling WritableFile::Sync()) before the write
  // is considered complete.  If this flag is true, writes will be
  // slower.
  //
  // If this flag is false, and the machine crashes, some recent
  // writes may be lost.  Note that if it is just the process that
  // crashes (i.e., the machine does not reboot), no writes will be
  // lost even if sync==false.
  //
  // In other words, a DB write with sync==false has similar
  // crash semantics as the "write()" system call.  A DB write
  // with sync==true has similar crash semantics to a "write()"
  // system call followed by "fsync()".
  //
  // Default: false
  bool sync;

  // If true, writes will not first go to the write ahead log,
  // and the write may got lost after a crash.
  bool disableWAL;

  WriteOptions()
      : sync(false),
        disableWAL(false) {
  }
};

// Options that control flush operations
struct FlushOptions {
  // If true, the flush will wait until the flush is done.
  // Default: true
  bool wait;

  FlushOptions()
      : wait(true) {
  }
};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_INCLUDE_OPTIONS_H_
