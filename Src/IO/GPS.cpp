#include <Stuff/IO/GPS.hpp>

#include <Stuff/Util/Hacks/Try.hpp>

#include <tl/expected.hpp>

namespace Stf::GPS::Detail::NMEA {

constexpr uint8_t checksum(std::string_view data) {
    uint8_t res = 0;

    for (const auto c : data)
        res ^= static_cast<uint8_t>(c);

    return res;
}

struct Message {
    std::string_view payload;
    size_t n_segments;
    uint8_t given_cksum;
    uint8_t calculated_cksum;

    static tl::expected<Message, std::string_view> create_from_sv(std::string_view sentence) {
        if (sentence.ends_with("\r\n"))
            sentence.remove_suffix(2);

        if (sentence.size() < 4)
            return tl::unexpected { "Sentence is too small" };

        if (sentence[0] != '$')
            return tl::unexpected { "Sentence does not start with an adequate prefix" };
        sentence.remove_prefix(1);

        if (sentence[sentence.size() - 3] != '*')
            return tl::unexpected { "Sentence does not have a prefix delimiter" };

        const auto cksum_str = sentence.substr(sentence.size() - 2);
        const auto cksum_opt = Stf::fast_hex_sv_to_int<uint8_t, true>(cksum_str);

        if (!cksum_opt)
            return tl::unexpected { "Sentence checksum mismatch" };

        sentence.remove_suffix(3);

        Message message {
            .payload = sentence,
            .n_segments = 0,
            .given_cksum = *cksum_opt,
            .calculated_cksum = 0,
        };

        message.n_segments
            = std::count(message.payload.begin(), message.payload.end(), ',') + (message.payload.empty() ? 0ull : 1ull);

        for (const auto c : message.payload)
            message.calculated_cksum ^= static_cast<uint8_t>(c);

        return message;
    }

    template<typename It> void extract_segments(It it) const {
        for (std::string_view payload_copy = payload;;) {
            const auto index = payload_copy.find(',');
            const auto segment = payload_copy.substr(0, index);

            *(it++) = segment;

            if (index == std::string_view::npos)
                break;

            payload_copy = payload_copy.substr(index + 1);
        }
    }
};

}

namespace Stf::GPS {

namespace Detail {

/// Parses time in the format: hhmmss.sss\n
/// The character at the position of the dot does not matter
tl::expected<Time, std::string_view> parse_utc_time(std::string_view segment) {
    if (segment.size() != 9)
        return tl::unexpected { "String too small for a UTC time value" };

    Time time {};

    static const std::array<std::tuple<size_t, size_t, int Time::*>, 4> fields { { { 0, 2, &Time::hh },
        { 2, 2, &Time::mm }, { 4, 2, &Time::ss }, { 7, 3, &Time::sss } } };

    for (auto const& [start, len, ptr] : fields) {
        if (auto temp = Stf::fast_sv_to_int<int, false, true>(segment.substr(start, len)); temp != std::nullopt)
            time.*ptr = *temp;
        else
            return tl::unexpected { "Failure parsing UTC time segment as an integer" };
    }

    return time;
}

std::optional<Date> parse_utc_date(std::string_view segment) {
    if (segment.size() != 6)
        return std::nullopt;

    Date date {};

    static const std::array<std::tuple<size_t, size_t, int Date::*>, 4> fields { { { 0, 2, &Date::dd },
        { 2, 2, &Date::mm }, { 4, 2, &Date::yy } } };

    for (auto const& [start, len, ptr] : fields) {
        if (auto temp = Stf::fast_sv_to_int<int, false, true>(segment.substr(start, len)); temp != std::nullopt)
            date.*ptr = *temp;
        else
            return std::nullopt;
    }

    return date;
}

std::optional<Angle> parse_angle(std::string_view segment, size_t degree_digits = 2) {
    auto dot_loc = segment.find('.');

    if (dot_loc == std::string_view::npos)
        return std::nullopt;

    const auto lhs = segment.substr(0, dot_loc);
    const auto minutes_decimal_s = segment.substr(dot_loc + 1);

    if (lhs.size() <= degree_digits)
        return std::nullopt;

    const auto degrees_s = lhs.substr(0, degree_digits);
    const auto minutes_s = lhs.substr(degree_digits);

    int degrees;
    int minutes;
    int minutes_decimal;

    if (Stf::from_chars(degrees_s, degrees).ec != std::errc() || Stf::from_chars(minutes_s, minutes).ec != std::errc()
        || Stf::from_chars(minutes_decimal_s, minutes_decimal).ec != std::errc())
        return std::nullopt;

    return Angle {
        .degrees = degrees,
        .minutes = minutes,
        .minutes_decimal_digits = static_cast<int>(minutes_decimal_s.size()),
        .minutes_decimal = minutes_decimal,
    };
}

std::optional<std::pair<Direction, Direction>> parse_direction_indicators(
    std::string_view ns_indicator_segment, std::string_view ew_indicator_segment) {
    std::pair<Direction, Direction> ret;
    auto& [ns, ew] = ret;

    if (ns_indicator_segment.empty() || ew_indicator_segment.empty())
        return std::nullopt;

    const auto ns_indicator = ns_indicator_segment[0];
    const auto ew_indicator = ew_indicator_segment[0];

    if (ns_indicator == 'N' || ns_indicator == 'n')
        ns = Direction::N;
    else if (ns_indicator == 'S' || ns_indicator == 's')
        ns = Direction::S;
    else
        return std::nullopt;

    if (ew_indicator == 'E' || ew_indicator == 'e')
        ns = Direction::E;
    else if (ew_indicator == 'W' || ew_indicator == 'w')
        ns = Direction::W;
    else
        return std::nullopt;

    return ret;
}

}

/* GP sentences
AAM	Waypoint arrival alarm
APA	Autopilot format A
APB	Autopilot format B
BOD	Bearing, origin to destination
BWC	Bearing and distance to waypoint, great circle
BWR	Bearing and distance to waypoint, rhumb line (overridden by BWC if available)
DBT	Depth below transducer
DPT	Depth of water
GGA	Global Positioning System Fix Data
GLL	Geographic position, latitude and longitude (and time)
GSA	GPS DOP and active satellites
GSV	Satellites in view
HDM	Heading, magnetic north
HDT	Heading, true north
HSC	Steer to heading
MTW	Mean water temperature
RMB	Recommended minimum navigation info when dest. waypoint is active
RMC	Recommended minimum specific GPS/Transit data
VTG	Track made good and ground speed
WCV	Waypoint closure velocity
WPL	Waypoint location
XTE	Cross-track error
XTR	Cross-track error, dead reckoning (overriden by XTE if available)
 */

void GPSState::update_things(UpdateData const& data) {
    if (const auto parsed_time = Detail::parse_utc_time(data.time); parsed_time)
        time = *parsed_time;

    if (const auto parsed_date = Detail::parse_utc_date(data.date); parsed_date)
        date = *parsed_date;

    if (const auto parsed_latitude = Detail::parse_angle(data.latitude, 2),
        parsed_longitude = Detail::parse_angle(data.longitude, 3);
        parsed_latitude && parsed_longitude)
        location = { *parsed_latitude, *parsed_longitude };

    if (const auto parsed_direction = Detail::parse_direction_indicators(data.ns_direction, data.ew_direction);
        parsed_direction)
        direction = *parsed_direction;

    if (int parsed_satellites; Stf::from_chars(data.connected_satellites, parsed_satellites).ec == std::errc())
        connected_satellites = parsed_satellites;
}

void GPSState::feed_line(std::string_view line) {
    const auto message_opt = Detail::NMEA::Message::create_from_sv(line);
    if (!message_opt)
        return;

    if (message_opt->calculated_cksum != message_opt->given_cksum)
        return;

    auto message = *message_opt;

    std::array<std::string_view, 32> segments;

    if (message.n_segments > segments.size() || message.n_segments == 0)
        return;

    message.extract_segments(segments.begin());

    auto const& word = segments[0];

    UpdateData update_data {};

    if (word == "GPGGA") { // Fix data
        // TODO: incomplete

        /*
         * 0: Sentence ID
         * 1: UTC Time     hhmmss.sss
         * 2: Latitude     ddmm.[m]+
         * 3: N/S Inddic.  (N|S)
         * 4: Longitude    dddm.[m]+
         * 5: E/W Indic.   (E|W)
         * 6: Position Fix [0-9]. -> (0, 1, 2, 3) = (Invalid, Valid SPS, Valid DGPS, Valid PPS)
         * 7: Satellites   [0-9]{2}
         * 8: HDOP         float
         * 9: Altitude     float
         * 10: Alt. Units  M
         */

        if (segments[6] != "0") {
            update_data.latitude = segments[2];
            update_data.longitude = segments[4];
            update_data.ns_direction = segments[3];
            update_data.ew_direction = segments[5];
        }

        update_data.time = segments[1];
        update_data.connected_satellites = segments[7];
    } else if (word == "GPGLL") { // Position
        /*
         * 0: Sentence   ID
         * 1: Latitude   ddmm.[m]+
         * 2: N/S indic. (N|S)
         * 3: Longitude  dddmm.[m]+
         * 4: E/W indic. (E|W)
         * 5: UTC time   hhmmss.sss
         * 6: Status     (A|V) -> (A V) -> (Valid Invalid)
         */

        if (segments[6] != "A")
            return;

        update_data.latitude = segments[1];
        update_data.longitude = segments[3];
        update_data.ns_direction = segments[2];
        update_data.ew_direction = segments[4];
        update_data.time = segments[5];
    } else if (word == "GPRMC") { // Position and time
        // TODO: incomplete
        /*
         * 0: Sentence ID
         * 1: UTC time
         * 2: Status
         * 3: Latitude
         * 4: N/S indic.
         * 5: Longitude
         * 6: E/W indic.
         * 7: Speed over ground
         * 8: Course over ground
         * 9: UTC date
         * 10: Magnetic variation degrees
         * 11: Magnetic variation E/W
         */

        if (segments[2] != "A")
            return;

        update_data.time = segments[1];
        update_data.latitude = segments[3];
        update_data.longitude = segments[5];
        update_data.ns_direction = segments[4];
        update_data.ew_direction = segments[6];

        // segments[7] -> speed over ground
        // segments[8] -> course over ground
    }

    update_things(update_data);
}

/// @return
/// The distance between two coordinates in kilometers
float Angle::distance(Angle lat_0, Angle long_0, Angle lat_1, Angle long_1) {
    const float earth_radius = 6371.f;

    const auto d_lat = lat_1.as_radians() - lat_0.as_radians();
    const auto d_lon = long_1.as_radians() - long_0.as_radians();

    const auto sin_d_lat_2 = std::sin(d_lat / 2.f);
    const auto sin_d_lon_2 = std::sin(d_lon / 2.f);

    const auto a = sin_d_lat_2 * sin_d_lat_2
        + sin_d_lon_2 * sin_d_lon_2 * std::cos(lat_0.as_radians()) * std::cos(lat_1.as_radians());
    const auto c = 2.f * std::atan2(std::sqrt(a), std::sqrt(1.f - a));

    return earth_radius * c;
}

}
