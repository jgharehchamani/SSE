#include <iostream>
#include <string.h>
#include "../src/json/DBParserJSON.h"


void func1(const string& keyword, const list<unsigned> &documents)
{
    std::cout << keyword << ':';
    for (auto const& x: documents){
        std::cout << x << " ";
    }
    std::cout << std::endl;
}



int main(int argc, const char * argv[]) {
   
    const char *filename;
    
    
    if(argc != 2) {
        filename = "test/inverted_index_docs.txt";
        //std::cout << "usage: " << argv[0] << " <filename> \n";
    } else {
        filename = argv[1];
    }

    
    /* We prepare the functions to added into the scheduler */
        
    void (*f1)(const string& keyword, const list<unsigned> &documents);
    f1 = &func1;
        
        
    sse::dbparser::DBParserJSON parser(filename);
    /*
    * We add the functions into the scheduler
    * The first parameter is an identifier (name) for the scheduler executor,
    * in case that if we need to execute some functions.
    *
    */

    parser.addCallbackList(func1);

    std::cout << "starting the parse \n";
    parser.parse();
    std::cout << "parse finished \n";
    
    return 0;
}
