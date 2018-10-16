#ifndef DBParserJSON_h
#define DBParserJSON_h

#include <stdio.h>
#include "DBParser.h"
#include <list>
#include <iostream>
#include <cstdio>

namespace sse
{
namespace dbparser
{

class DBParserJSON: public DBParser {
public:
    DBParserJSON(const char *filename);
    DBParserJSON(const std::string& filename);
    void parse();
    bool parsed();
    
private:
    bool parsed_ = false;

    struct MessageHandler;
};


}
}

#endif
