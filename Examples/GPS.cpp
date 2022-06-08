#include <span>
#include <mutex>

#include <boost/asio.hpp>
#include <fmt/core.h>

#include <Stuff/IO/Delim.hpp>
#include <Stuff/IO/GPS.hpp>

namespace asio = boost::asio;

template<typename Executor = asio::any_io_executor>
struct SerialConnection {
    using executor_type = Executor;

    explicit SerialConnection(executor_type const& ex)
        : m_executor(ex)
        , m_serial_port(ex) {}

    void connect(std::string const& device) {
        m_serial_port.open(device);
        m_recv_promise = std::promise<void>{};
        start_receive();
    }

    std::shared_future<void> get_recv_future() {
        return m_recv_future.share();
    }

    void join_recv() {
        get_recv_future().get();
    }

    virtual void new_data(std::span<const uint8_t> data) = 0;

private:
    executor_type m_executor;
    boost::asio::basic_serial_port<Executor> m_serial_port;
    std::array<uint8_t, 16> m_recv_buffer;
    std::promise<void> m_recv_promise{};
    std::future<void> m_recv_future{};

    void start_receive() {
        m_serial_port.async_read_some(asio::buffer(m_recv_buffer), [this](boost::system::error_code ec, size_t bytes) {
            if (ec) {
                try {
                    throw boost::wrapexcept(ec, boost::source_location());
                } catch (...) { m_recv_promise.set_exception(std::current_exception()); }
                return;
            }

            std::span<const uint8_t> received{m_recv_buffer.data(), bytes};

            new_data(received);

            start_receive();
        });
    }
};

template<typename Executor = asio::any_io_executor>
struct GPSConnection final : public SerialConnection<Executor> {
    using executor_type = Executor;

    explicit GPSConnection(executor_type const& ex)
        : SerialConnection<Executor>(ex) {}

    void new_data(std::span<const uint8_t> data) override {
        // fmt::print("Received {} bytes\n", data.size());

        for (const auto b : data) {
            if (m_reader.update(b))
                continue;

            if (!m_reader.have_match()) {
                fmt::print("overflow while reading\n");
            }

            const auto line_span = m_reader.get_without_delimiter();

            const std::string_view line_str(reinterpret_cast<const char*>(line_span.data()), line_span.size());
            m_gps_state.feed_line(line_str);

            if (m_gps_state.time)
                fmt::print("Time      : {}\n", *m_gps_state.time);
            if (m_gps_state.location)
                fmt::print("Location  : {}, {}\n", m_gps_state.location->first, m_gps_state.location->second);

            fmt::print("Satellites: {}\n", m_gps_state.connected_satellites);

            std::vector<uint8_t> line {line_span.begin(), line_span.end()};
            line.push_back(0);
            fmt::print("{}\n", reinterpret_cast<const char*>(line.data()));

            m_reader.reset();
            m_reader.update(b);
        }
    }

private:
    Stf::Delim::ByteReader<96, 2> m_reader {{'\r', '\n'}};
    Stf::GPS::GPSState m_gps_state {};
};

int main(int argc, char** argv) {
    if (argc != 2)
        return 1;

    asio::io_context io_ctx{};

    GPSConnection connection(io_ctx.get_executor());
    connection.connect(argv[1]);

    //asio::executor_work_guard<asio::io_context::executor_type> guard{io_ctx.get_executor()};

    io_ctx.run();
}