#ifndef TIMERS_H
#define TIMERS_H

#include <sys/time.h>
#include <time.h>


// See http://apps.topcoder.com/forums/?module=Thread&threadID=642239&start=0

int get_time_counter = 0;


double get_time() {
  get_time_counter++;
  timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec * 1e-6;
}


double work = 0.0;
void add_work(double delta_work) {
  work += delta_work;
}

double cached_get_time() {
  static auto last = get_time();
  if (work < 0.01)
    return last;

  work = 0.0;
  auto t = get_time();
  if (t - last > 0.05) {
    cerr << "cached_get_time " << (t - last) << endl;
  }
  last = t;
  return t;
}


vector<double> deadlines;

void add_subdeadline(double fraction) {
  assert(!deadlines.empty());
  double now = get_time();
  double remaining = deadlines.back() - now;
  if (remaining < 0) {
    cerr << "# DEADLINE_MISSED = " << remaining << endl;
    deadlines.push_back(deadlines.back());
  }

  deadlines.push_back(now + remaining * fraction);
}


bool check_deadline() {
  return !deadlines.empty() and cached_get_time() > deadlines.back();
}


void set_deadline_from_now(double seconds) {
  deadlines.push_back(get_time() + seconds);
}



map<string, clock_t> timers;

#ifdef USE_TIME_IT
class TimeIt {
private:
  string name;
public:
  TimeIt(string name) : name(name) {
    timers[name] -= clock();
  }
  ~TimeIt() {
    timers[name] += clock();
  }
};
#else
class TimeIt {
public:
  TimeIt(string name) {}
};
#endif


void print_timers(ostream &out) {
  for (auto kv : timers) {
    out << "# " << kv.first << "_time = " << 1.0 * kv.second / CLOCKS_PER_SEC << endl;
  }
}


// On their machines, I saw about 5K get_time() calls per second.
// There are ~60K clock() calls per _CPU_ second, which turns out to be roughly
// the same.

void benchmark_timers(ostream &out) {
  {
    auto start_time = get_time();
    int cnt = 0;
    while (get_time() < start_time + 0.1) cnt++;
    cerr << 10*cnt << " get_time() calls per second" << endl;
  }
  {
    auto start_time = clock();
    int cnt = 0;
    while (clock() < start_time + 0.1*CLOCKS_PER_SEC) cnt++;
    cerr << 10*cnt << " clock() calls per second" << endl;
  }
}


#endif
