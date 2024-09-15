#ifndef TIME_LAPSE_H
#define TIME_LAPSE_H

#include <chrono>
#include <iostream>

#define fw(what) std::forward<decltype(what)>(what)

template <
    class result_t   = std::chrono::milliseconds,
    class clock_t    = std::chrono::steady_clock,
    class duration_t = std::chrono::milliseconds
>
auto since(std::chrono::time_point<clock_t, duration_t> const &start)
{
    return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}

template <class DT = std::chrono::milliseconds,
          class ClockT = std::chrono::steady_clock>
class elapsed_timer
{
    using timep_t = typename ClockT::time_point;
    timep_t _start = ClockT::now(), _end = {};

public:
    void tick() {
        _end = timep_t{};
        _start = ClockT::now();
    }

    void tock() { _end = ClockT::now(); }

    template <class T = DT>
    auto duration() const {
        if (_end == timep_t{}) {
            std::cerr << "tock before reporting" << std::endl;
            std::chrono::duration_cast<T>(0);
        }
        return std::chrono::duration_cast<T>(_end - _start);
    }
};

/** Class to measure the execution time of a callable */
template <
    typename TimeT = std::chrono::milliseconds, class ClockT = std::chrono::system_clock
>
struct measure
{
    /*Returns the quantity (count) of the elapsed time as TimeT units */
    template<typename F, typename ...Args> static typename TimeT::rep execution(F&& func, Args&&... args)
    {
        auto start = ClockT::now();

        fw(func)(std::forward<Args>(args)...);

        auto duration = std::chrono::duration_cast<TimeT>(ClockT::now() - start);
        return duration.count();
    }

    /* Returns the duration (in chrono's type system) of the elapsed time */
    template<typename F, typename... Args> static TimeT duration(F&& func, Args&&... args)
    {
        auto start = ClockT::now();

        fw(func)(std::forward<Args>(args)...);

        return std::chrono::duration_cast<TimeT>(ClockT::now() - start);
    }
};

#endif // TIME_LAPSE_H
