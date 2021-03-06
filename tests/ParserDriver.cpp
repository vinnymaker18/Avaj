#include "headers.h"
#include "SyntaxError.h"
#include "Parser.h"

using namespace std;

int main(int argc, char **argv)
{
    Parser parser(argv[1]);

    try {

        ParseResult<AST*>* ast = parser.constructAST();
        for (auto import : ast->result()->imports)
            cout << import->importString << endl;

        for (auto cls : ast->result()->classes) {
            for (auto fld: cls->dataFields)
                cout << fld->name << " " << fld->typeName << endl;
        }

    } catch (SyntaxError& err) {
        cout << err.what() << endl;
    }

    return 0;
}
