#pragma once

#define SPECS_COMPONENT(name) struct name; template<> struct IsComponent<name> : std::true_type {}; struct name
