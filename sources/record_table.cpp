#include "record_table.hpp"
#include <fstream>
#include <sstream>

const U32 default_record_table_size = 100;
const char *const default_record_table_filename = "data/records.txt";

RecordTable::RecordTable(U32 maximum_size) : maximum_size(maximum_size) {
}

void RecordTable::replace_with_random_records() {
  records.clear();
  add_record(Record("Adam", 4000));
  add_record(Record("Bree", 3600));
  add_record(Record("Cora", 200));
  add_record(Record("Dave", 800));
  add_record(Record("Elmo", 600));
}

void RecordTable::sort_table() {
  std::stable_sort(std::rbegin(records), std::rend(records));
}

U32 RecordTable::add_record(Record record) {
  records.push_back(record);
  sort_table();
  if (records.size() > maximum_size) {
    records.pop_back();
  }
  const auto distance = std::distance(std::begin(records), std::find(std::begin(records), std::end(records), record));
  return static_cast<U32>(distance + 1);
}

void RecordTable::add_record_vector(const std::vector<Record> &new_records) {
  for (const auto &new_record : new_records) {
    records.push_back(new_record);
  }
  sort_table();
  while (records.size() > maximum_size) {
    records.pop_back();
  }
}

U32 RecordTable::size() const {
  return static_cast<U32>(records.size());
}

void RecordTable::dump(std::string filename) const {
  std::ofstream stream(filename);
  for (const auto &record : records) {
    stream << '"' << record.get_name() << '"' << ',' << record.get_score() << '\n';
  }
}

void RecordTable::load(std::string filename) {
  records.clear();
  std::ifstream stream(filename);
  std::string line;
  std::vector<Record> loaded_records;
  while (std::getline(stream, line)) {
    const auto comma = line.find(',');
    const auto name_string = line.substr(1, comma - 2);
    const auto score_string = line.substr(comma + 1);
    U32 score = 0;
    std::stringstream score_stream(score_string);
    score_stream >> score;
    loaded_records.emplace_back(name_string, score);
  }
  add_record_vector(loaded_records);
}

std::vector<Record>::const_iterator RecordTable::begin() const {
  return records.cbegin();
}

std::vector<Record>::const_iterator RecordTable::end() const {
  return records.cend();
}
