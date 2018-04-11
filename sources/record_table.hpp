#ifndef RECORD_TABLE_HPP
#define RECORD_TABLE_HPP

#include "record.hpp"
#include <vector>

extern const U32 default_record_table_size;
extern const char *const default_record_table_filename;

class RecordTable {
public:
  explicit RecordTable(U32 maximum_size);

  void replace_with_random_records();

  U32 add_record(Record record);

  U32 size() const;

  void dump(std::string filename) const;

  void load(std::string filename);

  std::vector<Record>::const_iterator begin() const;

  std::vector<Record>::const_iterator end() const;

private:
  void sort_table();

  void add_record_vector(const std::vector<Record> &new_records);

  std::vector<Record> records;
  U32 maximum_size;
};

#endif
