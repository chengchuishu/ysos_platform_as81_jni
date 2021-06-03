/**
 * 
 * g++ -std=c++11 -DBOOST_LOG_DYN_LINK log.cpp -lboost_log -lpthread -lboost_log_setup -lboost_thread -lboost_system
 * 
 **/

#include <string>
#include <fstream>
#include <iostream>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/log/attributes/current_process_name.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/attribute_cast.hpp>
#include <boost/log/attributes/attribute_value.hpp>
#include <boost/log/sinks/async_frontend.hpp>

// Related headers
#include <boost/log/sinks/unbounded_fifo_queue.hpp>
#include <boost/log/sinks/unbounded_ordering_queue.hpp>
#include <boost/log/sinks/bounded_fifo_queue.hpp>
#include <boost/log/sinks/bounded_ordering_queue.hpp>
#include <boost/log/sinks/drop_on_overflow.hpp>
#include <boost/log/sinks/block_on_overflow.hpp>

#include <thread>
#include <chrono>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace attrs = boost::log::attributes;

enum severity_level
{
    normal,
    notification,
    warning,
    error,
    critical
};
std::ostream &operator<<(std::ostream &strm, severity_level level)
{
    static const char *strings[] =
        {
            "normal",
            "notification",
            "warning",
            "error",
            "critical"};

    if (static_cast<std::size_t>(level) < sizeof(strings) / sizeof(*strings))
        strm << strings[level];
    else
        strm << static_cast<int>(level);

    return strm;
}

typedef sinks::asynchronous_sink<sinks::text_ostream_backend, sinks::bounded_fifo_queue<1000, sinks::drop_on_overflow>> sink_t;

boost::shared_ptr<sink_t> init_logging()
{
    logging::add_common_attributes();
    boost::shared_ptr<logging::core> core = logging::core::get();
    boost::shared_ptr<sinks::text_ostream_backend> backend = boost::make_shared<sinks::text_ostream_backend>();
    boost::shared_ptr<sink_t> sink(new sink_t(backend));
    core->add_sink(sink);
    core->add_global_attribute("ThreadID", attrs::current_thread_id());
    core->add_global_attribute("Process", attrs::current_process_name());
    sink->set_filter(expr::attr<severity_level>("Severity") >= warning);
    sink->set_formatter(
        expr::stream
        << "["
        << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f") << "]["
        << expr::attr<attrs::current_thread_id::value_type>("ThreadID") << ":"
        << expr::attr<unsigned int>("LineID") << "]["
        << expr::attr<std::string>("Process")
        << "][" << expr::attr<severity_level>("Severity")
        << "] "
        << ":" << expr::smessage);
    {
        sink_t::locked_backend_ptr p = sink->locked_backend();
        p->add_stream(boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));
    }
    return sink;
}

void stop_logging(boost::shared_ptr<sink_t> &sink)
{
    boost::shared_ptr<logging::core> core = logging::core::get();
    // Remove the sink from the core, so that no records are passed to it
    core->remove_sink(sink);
    // Break the feeding loop
    sink->stop();
    // Flush all log records that may have left buffered
    sink->flush();

    sink.reset();
}
int main(int, char *[])
{
    boost::shared_ptr<sink_t> sink = init_logging();

    src::severity_channel_logger_mt<severity_level> lg(keywords::channel = "net");
    std::thread th1([&]() {
        BOOST_LOG_SEV(lg, warning) << "Hello world!";
        BOOST_LOG_SEV(lg, error) << "Hello world!";
        BOOST_LOG_SEV(lg, warning) << "Hello world!";
        BOOST_LOG_SEV(lg, warning) << "Hello world!";
        BOOST_LOG_SEV(lg, warning) << "Hello world!";
        BOOST_LOG_SEV(lg, warning) << "Hello world!";
        BOOST_LOG_SEV(lg, warning) << "Hello world!";
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        BOOST_LOG_SEV(lg, warning) << "Hello world!";
        BOOST_LOG_SEV(lg, warning) << "Hello world!";
    });
    BOOST_LOG_SEV(lg, warning) << "Hello world!";
    BOOST_LOG_SEV(lg, warning) << "Hello world!";
    BOOST_LOG_SEV(lg, warning) << "Hello world!";
    BOOST_LOG_SEV(lg, warning) << "Hello world!";
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    BOOST_LOG_SEV(lg, error) << "Hello world!";
    BOOST_LOG_SEV(lg, warning) << "Hello world!";
    BOOST_LOG_SEV(lg, warning) << "Hello world!";
    BOOST_LOG_SEV(lg, warning) << "Hello world!";
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    BOOST_LOG_SEV(lg, warning) << "Hello world!";

    th1.join();
    stop_logging(sink);

    return 0;
}