#include "loadplugin.h"

#include <ostream>
#include <streambuf>

#include <slag/Graph.h>

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

static slag::Graph* graph = nullptr;

extern "C" PLUGIN_VISIBILITY int fgraph(UrlHandlerParam* hp)
{
    ostreambuf<char> outputbuffer(hp->pucBuffer, hp->dataBytes);
    std::ostream output(&outputbuffer);

    output << "<!DOCTYPE html><html><head><meta charset = \"UTF-8\"><title>SLAG</title></head><body>";
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

extern "C" PLUGIN_VISIBILITY int handler(MW_EVENT msg, int argi, void* argp)
{
    // printf("%s, argi: %d, argp: %p\n", msg == MW_INIT ? "MW_INIT" : (msg == MW_UNINIT ? "MW_UNINIT" : (msg == MW_PARSE_ARGS ? "MW_PARSE_ARGS" : "?")), argi, argp);
    switch (msg)
    {
    case MW_INIT:
        try {
            if (graph)
                delete graph;
            graph = new slag::Graph();
        }
        catch (std::exception& e)
        {
            fprintf(stderr, "Exception during MW_INIT: \"%s\"\n", e.what());
            graph = nullptr;
        }
        break;
    case MW_UNINIT:
        try {
            if (graph)
                delete graph;
        }
        catch (std::exception& e)
        {
            fprintf(stderr, "Exception during MW_UNINIT: \"%s\"\n", e.what());
        }
        break;
    }
    return 0;	//0 on success, -1 on failure
}
