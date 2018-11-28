#pragma once

#include <chrono>
#include <stack>
#include <fstream>
#include <memory>
#include "caf/all.hpp"
#include "atoms.hpp"
#include "constants.hpp"

using namespace caf;
using time_point = std::chrono::time_point<std::chrono::system_clock>;

class reporter_state {
 private:
  void write_headers(const std::vector<std::string>& headers);
 protected:
  char delimiter;
  std::unique_ptr<std::ofstream> out_stream;
 public:
  reporter_state();

  void open_stream(const std::string& file,
                   const std::vector<std::string>& headers);

  void close_stream();

  void write_part(const std::string& part);

  void write_new_line();
};

class time_reporter_state : public reporter_state {
 private:
  std::stack<time_point> start_times;
 public:
  using reporter_state::reporter_state;

  void note_start(const time_point& start);

  void note_end(const time_point& end);

  void write_info(actor_phase phase, std::size_t generation,
                  std::size_t island);
};

behavior reporter(stateful_actor<reporter_state>* self);

behavior time_reporter(stateful_actor<time_reporter_state>* self);
