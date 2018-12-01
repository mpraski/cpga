#include "reporter.hpp"
#include "caf/all.hpp"

using namespace caf;

void reporter_state::write_headers(const std::vector<std::string>& headers) {
  for (const auto& header : headers) {
    *out_stream << header << delimiter;
  }

  *out_stream << std::endl;
}

reporter_state::reporter_state()
    : delimiter { ',' },
      out_stream { new std::ofstream } {
}

void reporter_state::open_stream(const std::string& file,
                                 const std::vector<std::string>& headers) {
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

void reporter_state::write_part(const std::string& part) {
  *out_stream << part << delimiter;
}

void reporter_state::write_new_line() {
  *out_stream << std::endl;
}

void time_reporter_state::note_start(const time_point& start) {
  start_times.push(start);
}

void time_reporter_state::note_end(const time_point& end) {
  if (start_times.empty()) {
    throw std::runtime_error("Start times stack is empty");
  }

  auto start = start_times.top();
  start_times.pop();

  auto s = std::chrono::time_point_cast<std::chrono::milliseconds>(start);
  auto e = std::chrono::time_point_cast<std::chrono::milliseconds>(end);
  auto total = std::chrono::duration_cast<std::chrono::milliseconds>(
      end - start);

  *out_stream << s.time_since_epoch().count() << delimiter
              << e.time_since_epoch().count() << delimiter << total.count()
              << delimiter;
}

void time_reporter_state::write_info(actor_phase phase, std::size_t generation,
                                     std::size_t island) {
  *out_stream << constants::ACTOR_PHASE_MAP[to_underlying(phase)] << delimiter
              << generation << delimiter << island << std::endl;
}

behavior reporter(stateful_actor<reporter_state>* self) {
  return {
    [=](init_reporter, const std::string& file, const std::vector<std::string>& headers) {
      self->state.open_stream(file, headers);
    },
    [=](report, const std::string& part) {
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

behavior time_reporter(stateful_actor<time_reporter_state>* self) {
  return {
    [=](init_reporter, const std::string& file, const std::vector<std::string>& headers) {
      self->state.open_stream(file, headers);
    },
    [=](exit_reporter) {
      self->state.close_stream();
      self->quit();
    },
    [=](note_start, const time_point& start) {
      self->state.note_start(start);
    },
    [=](note_end, const time_point& end) {
      self->state.note_end(end);
    },
    [=](report_info, actor_phase phase, std::size_t generation, std::size_t island) {
      self->state.write_info(phase, generation, island);
    },
  };
}
