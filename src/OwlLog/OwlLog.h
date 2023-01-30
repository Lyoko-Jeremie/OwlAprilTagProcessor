// jeremie

#ifndef OWLAPRILTAGPROCESSOR_OWLLOG_H
#define OWLAPRILTAGPROCESSOR_OWLLOG_H

#include <string>

#include <boost/log/trivial.hpp>

namespace OwlLog {
    extern thread_local std::string threadName;

    // https://stackoverflow.com/questions/60977433/including-thread-name-in-boost-log
    void init_logging();

} // OwlLog

#endif //OWLAPRILTAGPROCESSOR_OWLLOG_H
