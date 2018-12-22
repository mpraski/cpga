#include "reporter.hpp"

using namespace caf;

void reporter_state::write_headers(const std::vector<std::string> &headers) {
  for (const auto &header : headers) {
    *out_stream << header << delimiter;
  }

  *out_stream << std::endl;
}

reporter_state::reporter_state()
    : delimiter{','},
      out_stream{std::make_unique<std::ofstream>()} {
}

void reporter_state::open_stream(const std::string &file,
                                 const std::vector<std::string> &headers) {
  out_stream->open(file.c_str(), std::ios_base::binary | std::ios_base::out);

  if (!out_stream->is_open()) {
    throw std::runtime_error("Unable to open an output stream");
  }

  if (!headers.empty()) {
    write_headers(headers);
  }
}

void reporter_state::close_stream() {
  if (out_stream) {
    out_stream->close();
  }
}

void reporter_state::write_part(const std::string &part) {
  *out_stream << part << delimiter;
}

void reporter_state::write_new_line() {
  *out_stream << std::endl;
}

void time_reporter_state::note_start(const time_point &start,
                                     island_id island) {
  if (start_times.find(island) == start_times.end()) {
    start_times.emplace(island, std::stack<time_point>{});
  }

  start_times[island].push(start);
}

void time_reporter_state::write_info(const time_point &end, actor_phase phase,
                                     size_t generation, island_id island) {
  using namespace std::chrono;
  auto &times = start_times[island];

  if (times.empty()) {
    throw std::runtime_error("Start times stack is empty");
  }

  auto start = times.top();
  times.pop();

  auto s = time_point_cast<milliseconds>(start);
  auto e = time_point_cast<milliseconds>(end);
  auto total = duration_cast<milliseconds>(end - start);

  *out_stream << s.time_since_epoch().count() << delimiter
              << e.time_since_epoch().count() << delimiter << total.count()
              << delimiter << constants::ACTOR_PHASE_MAP[to_underlying(phase)]
              << delimiter << generation << delimiter << island << std::endl;
}

void system_reporter_state::write_message(const time_point &time,
                                          const std::string &message) {
  auto t = std::chrono::time_point_cast<std::chrono::milliseconds>(time);

  *out_stream << t.time_since_epoch().count() << delimiter << message
              << std::endl;
}

behavior reporter(stateful_actor<reporter_state> *self) {
  return {
      [=](init_reporter, const std::string &file, const std::vector<std::string> &headers) {
        self->state.open_stream(file, headers);
      },
      [=](report, const std::string &part) {
        self->state.write_part(part);
      },
      [=](report_new_line) {
        self->state.write_new_line();
      },
      [=](exit_reporter) {
        self->state.close_stream();
        self->quit();
      },
  };
}

behavior time_reporter(stateful_actor<time_reporter_state> *self) {
  return {
      [=](init_reporter, const std::string &file, const std::vector<std::string> &headers) {
        self->state.open_stream(file, headers);
      },
      [=](exit_reporter) {
        self->state.close_stream();
        self->quit();
      },
      [=](note_start, const time_point &start, island_id island) {
        self->state.note_start(start, island);
      },
      [=](note_end, const time_point &end, actor_phase phase, size_t generation, island_id island) {
        self->state.write_info(end, phase, generation, island);
      },
  };
}

behavior system_reporter(stateful_actor<system_reporter_state> *self) {
  return {
      [=](init_reporter, const std::string &file, const std::vector<std::string> &headers) {
        self->state.open_stream(file, headers);
      },
      [=](exit_reporter) {
        self->state.close_stream();
        self->quit();
      },
      [=](report, const time_point &time, const std::string &message) {
        self->state.write_message(time, message);
      },
  };
}
