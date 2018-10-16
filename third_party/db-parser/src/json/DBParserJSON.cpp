#include "DBParserJSON.h"

#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include <iostream>
#include <cstdio>

using namespace rapidjson;
using namespace std;

namespace sse
{
namespace dbparser
{

struct DBParserJSON::MessageHandler : public BaseReaderHandler<UTF8<>, MessageHandler>
{
    MessageHandler(): state_(kExpectObjectStart) {
    }
    
    bool StartObject() {
        switch (state_) {
            case kExpectObjectStart:
                state_ = kExpectKeyOrObjectEnd;
                return true;
            default:
                return false;
        }
    }
    
    bool Key(const char* str, SizeType length, bool) {
        switch (state_) {
            case kExpectKeyOrObjectEnd:
                keyword_ = string(str, length);
                state_ = kExpectStartList;
                return true;
            default:
                return false;
        }
    }
    
    bool StartArray() {
        switch(state_) {
            case kExpectStartList:
                state_ = kExpectFirstValue;
                return true;
            default:
                return false;
                
        }
    }
    
    bool Uint(unsigned i) {
        switch (state_) {
            case kExpectFirstValue:
                ids_.push_back(i);
                state_ = kExpectValueOrEndArray;
                callFunctionsPair(keyword_, ids_.back());
                return true;
            case kExpectValueOrEndArray:
                ids_.push_back(i);
                callFunctionsPair(keyword_, ids_.back());
                return true;
            default:
                return false;
        }
    }
    
    bool EndArray(SizeType elementCount) {
        switch(state_){
            case kExpectValueOrEndArray:
                state_ = kExpectKeyOrObjectEnd;
                
                callFunctionsList(keyword_, ids_);
                
                keyword_.clear();
                ids_.clear();
                return true;
            default:
                return false;
        }
    }
    
    bool EndObject(SizeType) {
        return state_ == kExpectKeyOrObjectEnd;
    }
    
    bool Default() {
        return false;
    } // All other events are invalid.
    
    enum State {
        kExpectObjectStart,
        kExpectKeyOrObjectEnd,
        kExpectStartList,
        kExpectFirstValue,
        kExpectValueOrEndArray,
    } state_;
    
    string keyword_;
    list<unsigned> ids_;
};

    DBParserJSON::DBParserJSON(const char *filename)
    : DBParser(filename)
    {
        
    }
    
    DBParserJSON::DBParserJSON(const std::string& filename)
    : DBParser(filename.c_str())
    {
        
    }


void DBParserJSON::parse(){
    if(!parsed_) {
        FILE* fp = fopen(get_file(), "r");
        char readBuffer[65536];
        FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        
        MessageHandler handler;
        Reader reader;

        reader.Parse(is, handler);
        
        fclose(fp);
        parsed_ = true;
    }
}

bool DBParserJSON::parsed() {
    return parsed_;
}

}
}


