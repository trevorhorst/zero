#ifndef COMMON_CONTROL_TEMPLATE_H
#define COMMON_CONTROL_TEMPLATE_H

#include <vector>

#include "common/control/control.h"

template<typename T>
class ControlTemplate :
        public Control
{
public:
    ControlTemplate() :
        Control()
    {
        objects.push_back(static_cast<T*>(this));
    }

    ~ControlTemplate()
    {
    }

private:
    static std::vector<T*> objects;
};

template<typename T>
std::vector<T*> ControlTemplate<T>::objects = {};

#endif // RP2040_CONTROL_TEMPLATE_H
