
#include <stdio.h>
#include <iostream>
#include "DBParser.h"

using namespace std;

namespace sse
{
namespace dbparser
{

/* The list of the functions to be called*/
FPairQueue DBParser::pairqueue_;
FListQueue DBParser::listqueue_;

DBParser::DBParser(const char *filename): file_(filename)  //same as file_ = filename
{
	pairqueue_.clear();
	listqueue_.clear();
}

const char * DBParser::get_file()
{
    return file_;
}


/* TODO 
 * change the name for registerCallback 
 */

/* std::function<void(int)> callback */
void DBParser::addCallbackList(funcList funcCall){
    listqueue_.push_back(funcCall);
}

void DBParser::addCallbackPair(funcPair funcCall){
    pairqueue_.push_back(funcCall);
}


/* Execute functions */

/* Execute all functions in the list of task */

void DBParser::callFunctionsList(const string &keyword, const list<unsigned> &documents){
    for(auto const& f: listqueue_)
    {
        f(keyword, documents);
    }
}

void DBParser::callFunctionsPair(const string &keyword, const unsigned &doc){
    for(auto const& f: pairqueue_)
    {
        f(keyword, doc);
    }
}

}
}