/***
 * Abstract class to operate with different kind of providers.
 ***/

#ifndef DBParser_h
#define DBParser_h

#include <string>
#include <map>
#include <utility>
#include <list>
#include <functional>
#include <list>

using namespace std;


namespace sse
{
namespace dbparser
{
/* std::function<void(int)> callback */

typedef std::function<void(const string& keyword, const list<unsigned> &documents)> funcList;
typedef std::function<void(const string& keyword, const unsigned &doc)> funcPair;

typedef std::list<funcPair> FPairQueue;
typedef std::list<funcList> FListQueue;

class DBParser
{
public:
    DBParser(const char *filename);
//    virtual ~DBParser();
    virtual void parse() = 0;
    const char *get_file();

    bool parsed();
    
    void addCallbackPair(funcPair);
    void addCallbackList(funcList);

    static void callFunctionsPair(const string& keyword, const unsigned &doc);
    static void callFunctionsList(const string& keyword, const list<unsigned> &documents);
    
    static FPairQueue pairqueue_;
    static FListQueue listqueue_;
    
private:
    const char *file_;

};

}
}

#endif /* DBParser_h */
