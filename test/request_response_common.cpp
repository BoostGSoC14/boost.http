#ifdef NDEBUG
#undef NDEBUG
#endif

#define CATCH_CONFIG_MAIN
#include "common.hpp"
#include <boost/http/reader/detail/transfer_encoding.hpp>

namespace asio = boost::asio;
namespace http = boost::http;
namespace reader = http::reader;

TEST_CASE("decode_transfer_encoding", "[detail]")
{
    // REMAINDER: there can never be beginning or trailing OWS in header fields

    using http::reader::detail::CHUNKED_NOT_FOUND;
    using http::reader::detail::CHUNKED_AT_END;
    using http::reader::detail::CHUNKED_INVALID;
    using http::reader::detail::decode_transfer_encoding;

    // CHUNKED_NOT_FOUND

    CHECK(decode_transfer_encoding("") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(", ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(", ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",,   ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(", , ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,   ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",,,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(", ,,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",,, ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",,,   ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",, ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",,   ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(", ,, ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,   ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",, ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",,   ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(", , ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(", ,   ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   , ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,   ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",, , ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",,   , ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",, ,   ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",,   ,   ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(", , , ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,   ,   ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chunked chunked") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chunked a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a chunked") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a chunked a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a#chunked#a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chunked#chunked") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chuNKed cHUnked") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chUNked a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a CHunked") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a chunkED a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a#cHunKed#a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chuNDeD#CHunkeD") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,chunked chunked")
          == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,chunked a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,a chunked") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,a chunked a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,a#chunked#a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,chunked#chunked")
          == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,chuNKed cHUnked")
          == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,chUNked a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,a CHunked") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,a chunkED a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,a#cHunKed#a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,chuNDeD#CHunkeD")
          == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chunked chunked,  ,,")
          == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chunked a,  ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a chunked,  ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a chunked a,  ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a#chunked#a,  ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chunked#chunked,  ,,")
          == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chuNKed cHUnked,  ,,")
          == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chUNked a,  ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a CHunked,  ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a chunkED a,  ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a#cHunKed#a,  ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chuNDeD#CHunkeD,  ,,")
          == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chunked chunked  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chunked a  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a chunked  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a chunked a  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a#chunked#a  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chunked#chunked  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chuNKed cHUnked  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chUNked a  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a CHunked  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a chunkED a  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a#cHunKed#a  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chuNDeD#CHunkeD  ,") == CHUNKED_NOT_FOUND);

    // CHUNKED_INVALID

    CHECK(decode_transfer_encoding("chunked,chunked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chUNked,CHunked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked ,CHUNKED") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNKED   ,chunked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNKed, cHUnked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("ChUNKed,   cHUnKed") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,say what") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked, say what") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,   say what") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked ,say what") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked   ,say what") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,chunked,chunked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,chUNked,CHunked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,chunked ,CHUNKED")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,CHUNKED   ,chunked")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,CHUNKed, cHUnked")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,ChUNKed,   cHUnKed")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,chunKed,say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,CHUNKED, say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,chunked,   say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,CHUNKED ,say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   chunked   ,say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   chunKED,chunked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   chUNked,CHunked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   chunked ,CHUNKED")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   CHUNKED   ,chunked")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   CHUNKed, cHUnked")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   ChUNKed,   cHUnKed")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   chunked,say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   CHUNKED, say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   chunked,   say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   CHUNKED ,say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   chunked   ,say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,chunked,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chUNked,CHunked,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked ,CHUNKED,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNKED   ,chunked,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNKed, cHUnked,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("ChUNKed,   cHUnKed,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,say what,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked, say what,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,   say what,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked ,say what,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked   ,say what,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,chunked ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chUNked,CHunked ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked ,CHUNKED ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNKED   ,chunked ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNKed, cHUnked ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("ChUNKed,   cHUnKed ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,say what ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked, say what ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,   say what ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked ,say what ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked   ,say what ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,chunked   ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chUNked,CHunked   ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked ,CHUNKED   ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNKED   ,chunked   ,")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNKed, cHUnked   ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("ChUNKed,   cHUnKed   ,")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,say what   ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked, say what   ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,   say what   ,")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked ,say what   ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked   ,say what   ,")
          == CHUNKED_INVALID);

    // CHUNKED_AT_END

    CHECK(decode_transfer_encoding("chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("Chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunkeD") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("CHUNKED") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("cHunKed") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding(",chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding(", chUnked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding(",   chunkEd") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding(",,   chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding(", ,chunKed") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding(",   ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding(",  ,,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked,") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunkEd ,") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked   ,") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunKed   ,,") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("cHunked, ,") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunkeD,   ,") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("cHunked,,   ,") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("and then there was light,chunked")
          == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("let there be rock  ,chunked")
          == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("punk is dead,  chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("punk is not dead,,chunked")
          == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a, chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("#chunked#,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("#chunked,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked#,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("#chunked# ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("#chunked ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked# ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("#chunked#   ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked#   ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("#chunked   ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("#chunked#, chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("#chunked, chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked#, chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("#chunked#,   chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked#,   chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("#chunked,   chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a chunked b,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a chunked,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked b,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a chunked b ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a chunked ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked b ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a chunked b   ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked b   ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a chunked   ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a chunked b, chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a chunked, chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked b, chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a chunked b,   chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked b,   chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a chunked,   chunked") == CHUNKED_AT_END);
}
