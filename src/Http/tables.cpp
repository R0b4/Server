#include "tables.hpp"

std::map<const string_view, const standard_header, cmp_str_case_insensitive> str_to_header_t = {
    {string_view("Accept"), haccept},
    {string_view("Accept-Encoding"), hacceptenc},
    {string_view("Accept-Charset"), hacceptcharset},
    {string_view("Cookie"), hcookie},
    {string_view("Set-Cookie"), hsetcookie},
    {string_view("Content-Type"), hcontenttype},
    {string_view("Content-Length"), hcontentlength},
    {string_view("Content-Encoding"), hcontentenc},
    {string_view("Date"), hdate},
    {string_view("Server"), hserver},
    {string_view("Host"), hhost}
};

std::map<const standard_header, const string_view> header_to_str = {
    {haccept, string_view("Accept")},
    {hacceptenc, string_view("Accept-Encoding")},
    {hacceptcharset, string_view("Accept-Charset")},
    {hcookie, string_view("Cookie")},
    {hsetcookie, string_view("Set-Cookie")},
    {hcontenttype, string_view("Content-Type")},
    {hcontentlength, string_view("Content-Length")},
    {hcontentenc, string_view("Content-Encoding")},
    {hdate, string_view("Date")},
    {hserver, string_view("Server")},
    {hhost, string_view("Host")}
};

std::map<const string_view, const request_t, cmp_str> str_to_request_t = {
    {string_view("GET"), rget},
    {string_view("POST"), rpost},
    {string_view("PUT"), rput},
    {string_view("HEAD"), rhead},
    {string_view("DELETE"), rdelete},
    {string_view("TRACE"), rtrace},
    {string_view("OPTIONS"), roptions},
    {string_view("CONNECT"), rconnect},
    {string_view("PATCH"), rpatch}
};

std::map<const string_view, const httpversion, cmp_str> str_to_httpversion = {
    {string_view("HTTP/1.0"), http10},
    {string_view("HTTP/1.1"), http11},
    {string_view("HTTP/2"), http2}
};

std::map<const httpversion, const string_view> httpversion_to_str = {
    {http10, string_view("HTTP/1.0")},
    {http11, string_view("HTTP/1.1")},
    {http2, string_view("HTTP/2")}
};

std::map<string_view, string_view, cmp_str> ext_to_mime {
    {string_view(".aac"), string_view("audio/aac")},
    {string_view(".abw"), string_view("application/x-abiword")},
    {string_view(".arc"), string_view("application/x-freearc")},
    {string_view(".avi"), string_view("video/x-msvideo")},
    {string_view(".azw"), string_view("application/vnd.amazon.ebook")},
    {string_view(".bin"), string_view("application/octet-stream")},
    {string_view(".bmp"), string_view("image/bmp")},
    {string_view(".bz"), string_view("application/x-bzip")},
    {string_view(".bz2"), string_view("application/x-bzip2")},
    {string_view(".cda"), string_view("application/x-cdf")},
    {string_view(".csh"), string_view("application/x-csh")},
    {string_view(".css"), string_view("text/css")},
    {string_view(".csv"), string_view("text/csv")},
    {string_view(".doc"), string_view("application/msword")},
    {string_view(".docx"), string_view("application/vnd.openxmlformats-officedocument.wordprocessingml.document")},
    {string_view(".eot"), string_view("application/vnd.ms-fontobject")},
    {string_view(".epub"), string_view("application/epub+zip")},
    {string_view(".gz"), string_view("application/gzip")},
    {string_view(".gif"), string_view("image/gif")},
    {string_view(".htm"), string_view("text/html")},
    {string_view(".html"), string_view("text/html")},
    {string_view(".ico"), string_view("image/vnd.microsoft.icon")},
    {string_view(".ics"), string_view("text/calendar")},
    {string_view(".jar"), string_view("application/java-archive")},
    {string_view(".jpeg"), string_view("image/jpeg")},
    {string_view(".jpg"), string_view("image/jpeg")},
    {string_view(".js"), string_view("text/javascript")},
    {string_view(".json"), string_view("application/json")},
    {string_view(".jsonld"), string_view("application/ld+json")},
    {string_view(".mid"), string_view("audio/mid")},
    {string_view(".midi"), string_view("audio/midi")},
    {string_view(".mjs"), string_view("text/javascript")},
    {string_view(".mp3"), string_view("audio/mpeg")},
    {string_view(".mp4"), string_view("video/mp4")},
    {string_view(".mpeg"), string_view("video/mpeg")},
    {string_view(".mpkg"), string_view("application/vnd.apple.installer+xml")},
    {string_view(".odp"), string_view("application/vnd.oasis.opendocument.presentation")},
    {string_view(".ods"), string_view("application/vnd.oasis.opendocument.spreadsheet")},
    {string_view(".odt"), string_view("application/vnd.oasis.opendocument.text")},
    {string_view(".oga"), string_view("audio/ogg")},
    {string_view(".ogv"), string_view("video/ogg")},
    {string_view(".ogx"), string_view("application/ogg")},
    {string_view(".opus"), string_view("audio/opus")},
    {string_view(".otf"), string_view("font/otf")},
    {string_view(".png"), string_view("image/png")},
    {string_view(".pdf"), string_view("application/pdf")},
    {string_view(".php"), string_view("application/x-httpd-php")},
    {string_view(".ppt"), string_view("application/vnd.ms-powerpoint")},
    {string_view(".pptx"), string_view("application/vnd.openxmlformats-officedocument.presentationml.presentation")},
    {string_view(".rar"), string_view("application/vnd.rar")},
    {string_view(".rtf"), string_view("application/rtf")},
    {string_view(".sh"), string_view("application/x-sh")},
    {string_view(".svg"), string_view("image/svg+xml")},
    {string_view(".swf"), string_view("application/x-shockwave-flash")},
    {string_view(".tar"), string_view("application/x-tar")},
    {string_view(".tif .tiff"), string_view("image/tiff")},
    {string_view(".ts"), string_view("video/mp2t")},
    {string_view(".ttf"), string_view("font/ttf")},
    {string_view(".txt"), string_view("text/plain")},
    {string_view(".vsd"), string_view("application/vnd.visio")},
    {string_view(".wav"), string_view("audio/wav")},
    {string_view(".weba"), string_view("audio/webm")},
    {string_view(".webm"), string_view("video/webm")},
    {string_view(".webp"), string_view("image/webp")},
    {string_view(".woff"), string_view("font/woff")},
    {string_view(".woff2"), string_view("font/woff2")},
    {string_view(".xhtml"), string_view("application/xhtml+xml")},
    {string_view(".xls"), string_view("application/vnd.ms-excel")},
    {string_view(".xlsx"), string_view("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet")},
    {string_view(".xml"), string_view("application/xml")},
    {string_view(".xul"), string_view("application/vnd.mozilla.xul+xml")},
    {string_view(".zip"), string_view("application/zip")},
    {string_view(".7z"), string_view("application/x-7z-compresse")},
};

string_view get_mime(string_view ext) {
    auto it = ext_to_mime.find(ext);
    if (it == ext_to_mime.end()) return string_view("text/plain");
    return it->second;
}