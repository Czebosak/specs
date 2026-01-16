export module specs.system;

import specs.component_storage;

namespace specs {
    export struct System {
        void (*func)(ComponentStorage&);
        bool disabled;
    };
}
