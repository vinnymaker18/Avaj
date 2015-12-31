#ifndef _TYPE_H_
#define _TYPE_H_

#include "headers.h"
#include "Expression.h"
#include "Method.h"

class SymbolTable;

/* An abstract class to represent Avaj types. Builtin types and user defined
 * types are instances of its subclasses. In future, this class will include
 * the method code as well.
 */
class Type
{
public:
    /* Name of the type.*/
    const std::string name;

    /* Total size of all data fields in this type.*/
    const int size;

    /* List of the methods supported by this type.*/
    const std::vector<Method> methods;
public:
    Type(const std::string _name, const int _size,
         const std::vector<Method>& methods);
    /* Get the total size of the data fields in this type. This may be less
     * than the actual size occupied by the objects of this type in RAM because
     * of, say, alignment restrictions.
     */
    virtual int getSize() const;

    /* Return the list of all methods supported by this type.*/
    virtual std::vector<Method> getMethods() const = 0;

    /* Compute the type of an Avaj expression, within the context of the given
     * symbol table.
     */
    static Type *getExpressionType(const Expression *expr,
                                   const SymbolTable& symTab);
};
#endif
