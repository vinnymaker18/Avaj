#ifndef _SYNTACTIC_UNITS_H_
#define _SYNTACTIC_UNITS_H_

/**
 * Syntactic units are components of the source code. Expressions, statements,
 * literals and their sub varieties etc.. make up the code. If tokens are like
 * the words of a programming language, these units are the sentences, phrases
 * and paragraphs. The job of the parser then is to process a stream of tokens
 * and group them into these syntactic units.
 */


/* Import statements at the top of the source file. */
class Import
{
public:
    /**
     * Name of the module being imported. Module name may be nested with
     * periods as separators.
     */
    const std::string modname;

    Import(const std::string& _modname): modname(_modname) {}
};


/* Parameter list in a method definition.*/
class MethodParamList
{
public:
    /* Type and name of a single parameter.*/
    typedef std::pair<std::string, std::string> TypeAndNamePair;

    const std::vector<TypeAndNamePair> paramList;

    MethodParamList(const std::vector<TypeAndNamePair>& _paramList):
        paramList(_paramList) {}
};

/**
 * A statement in an imperative programming language denotes an action to be
 * performed. Statements can and frequently do affect the program state.
 * Typically, statements are used to modify the program state, interact with
 * outside world etc.. Statements can be assignments, variable declarations,
 * method calls etc..
 */
class Statement
{
public:
    virtual ~Statement() = 0;
};

#endif