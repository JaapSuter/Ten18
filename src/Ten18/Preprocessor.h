#pragma once

#define Ten18_STRINGIFY_2(x) #x
#define Ten18_STRINGIFY(x) Ten18_STRINGIFY_2(x)
#define Ten18_FILE_AND_LINE __FILE__ "(" Ten18_STRINGIFY(__LINE__) "): "
