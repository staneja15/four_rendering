#include "sample_application.h"

#include <cstdlib>

int main() {
    auto app = SampleApplication();
    app.init();
    app.run();


    return EXIT_SUCCESS;
}
