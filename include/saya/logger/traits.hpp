#ifndef SAYA_LOGGER_TRAITS_HPP_
#define SAYA_LOGGER_TRAITS_HPP_

#include "saya/logger_fwd.hpp"
#include "saya/logger/level_traits.hpp"

#include "saya/string_config.hpp"
#include "saya/stream_object.hpp"

#include <string>
#include <ostream>
#include <streambuf>
#include <algorithm> // max

namespace saya {

#define SAYA_DEF(CHAR_TYPE, BINDER) \
    template<class Color> \
    struct basic_logger_traits<CHAR_TYPE, Color> \
    { \
        using char_type = CHAR_TYPE; \
        using color_type = Color; \
        using string_type = std::basic_string<char_type>; \
        using ostream_type = std::basic_ostream<char_type>; \
        using streambuf_type = std::basic_streambuf<char_type>; \
        using stream_object_type = stream_object<char_type>; \
        \
        template<class Level> \
        using level_traits_type = basic_logger_level_traits<char_type, Level>; \
        \
        static constexpr std::size_t max_label_width() noexcept { \
            return std::max(std::max(level_traits_type<logger_level::INFO>::label_len(), level_traits_type<logger_level::WARN>::label_len()), level_traits_type<logger_level::ERROR>::label_len()); \
        } \
        static std::size_t indent_magic(string_type const& prompt) noexcept { \
            return prompt.empty() ? (max_label_width() - level_traits_type<logger_level::NOTE>::label_len()) : (1 + prompt.size() + 1 + 1 + max_label_width() - level_traits_type<logger_level::NOTE>::label_len()); \
        } \
        \
        static constexpr char_type const* prompt_format() noexcept { return BINDER("%s[%s]%s %s%s%s %s"); } \
        template<class Level> \
        static constexpr char_type const* prompt_no_color_format() noexcept { return level_traits_type<Level>::need_paren() ? BINDER("[%s] (%s) %s") : BINDER("[%s] %s %s"); } \
        \
        static constexpr char_type const* noprompt_format() noexcept { return BINDER("%s%s%s %s"); } \
        template<class Level> \
        static constexpr char_type const* noprompt_no_color_format() noexcept { return level_traits_type<Level>::need_paren() ? BINDER("(%s) %s") : BINDER("%s %s"); } \
        \
        static constexpr char_type const* note_format() noexcept { return BINDER("%s%s%s%s %s"); } \
        static constexpr char_type const* note_no_color_format() noexcept { return BINDER("%s%s %s"); } \
    };

SAYA_STRING_CONFIG_DEFINE(SAYA_DEF)
#undef SAYA_DEF

} // saya

#endif