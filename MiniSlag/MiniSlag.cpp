#include "loadplugin.h"

#include <ostream>
#include <streambuf>

template <typename char_type, typename traits = std::char_traits<char_type>>
struct ostreambuf : public std::basic_streambuf<char_type, traits >
{
    ostreambuf(char_type* buffer, std::streamsize bufferLength) {
        this->setp(buffer, buffer + bufferLength);
    }
    const char* GetPos()const
    {
        return this->pptr();
    }
};

extern "C" PLUGIN_VISIBILITY int graph(UrlHandlerParam* hp)
{
    ostreambuf<char> outputbuffer(hp->pucBuffer, hp->dataBytes);
    std::ostream output(&outputbuffer);

    output << "<!DOCTYPE html><html><head><meta charset = \"UTF-8\"><title>"
        "Title of the document"
        "</title></head><body>";
    output << "<dl>";
    for (int i = 0; i < hp->iVarCount; ++i)
    {
        output << "<dt>" << hp->pxVars[i].name << "</dt><dd>" << hp->pxVars[i].value << "</dd>";
    }
    output << "</dl>";
    output << "</body></html>";

    hp->dataBytes = outputbuffer.GetPos() - hp->pucBuffer;
    hp->fileType = HTTPFILETYPE_HTML;
    return FLAG_DATA_RAW;
}

PLUGIN_VISIBILITY int MyUrlHandlerEvent(MW_EVENT msg, int argi, void* argp)
{
    switch (msg)
    {
    case MW_INIT:
        break;
    case MW_UNINIT:
        break;
    }
    return 0;	//0 on success, -1 on failure
}
