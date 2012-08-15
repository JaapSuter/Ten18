#include "Ten18/Format.h"
#include "Ten18/Expect.h"

 Ten18::Format& Ten18::Format::DebugOut()
 {
     Ten18::DebugOut(this->c_str());
     return *this;
 }

 const Ten18::Format& Ten18::Format::DebugOut() const
 {
     Ten18::DebugOut(this->c_str());
     return *this;
 }