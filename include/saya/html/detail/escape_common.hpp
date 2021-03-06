#ifndef SAYA_HTML_DETAIL_ESCAPE_COMMON_HPP_
#define SAYA_HTML_DETAIL_ESCAPE_COMMON_HPP_

namespace saya { namespace html {

enum EscapeFlags
{
    UNESCAPE_NAMED = (1<<0),
    ESCAPE_NAMED = UNESCAPE_NAMED,

    UNESCAPE_DECIMAL = (1<<2),
    ESCAPE_DECIMAL = UNESCAPE_DECIMAL,

    UNESCAPE_DEFAULT = UNESCAPE_NAMED | UNESCAPE_DECIMAL,
    ESCAPE_DEFAULT = ESCAPE_NAMED,
};

}} // saya

#endif
