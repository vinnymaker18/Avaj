#pragma once

#include "headers.h"
#include "SyntaxError.h"
#include "ParseResult.h"
#include "TokenBuffer.h"
#include "Class.h"

/* Import statements at the beginning of the file. */
class Import 
{
public:
    Import(const std::string& _importStr):
        importString(_importStr) {}

    std::string importString;

    static ParseResult<Import*>* tryParse(TokenBuffer& tokenBuffer);
};

/**
 * Abstract Syntax Tree of a source file.
 */
class AST
{
public:
    const std::vector<Import*> imports;
    const std::vector<Class*> classes;

    AST(const std::vector<Import*>& _imports,
        const std::vector<Class*>& _classes):
        imports(_imports), classes(_classes) {}

    static ParseResult<AST*>* tryParse(TokenBuffer& tokenBuffer);
};
