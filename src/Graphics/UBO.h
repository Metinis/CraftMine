

#ifndef UBO_H
#define UBO_H

#include "glad.h"



class UBO {
private:
    unsigned int matricesUBO;
public:
    UBO();
    void bind() const;
    void unbind() const;
};



#endif //UBO_H
