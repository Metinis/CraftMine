

#ifndef UBO_H
#define UBO_H

#include "glad.h"



class UBO {
private:
    unsigned int matricesUBO{};
public:
    UBO();
    void bind() const;
    static void unbind() ;
};



#endif //UBO_H
