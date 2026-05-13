#include "plugin.h"

#include <iostream>

int main()
{
    using namespace orc::plugins::nn_ntsc_chroma_sink;

    if (std::string(kStageName) != "nn_ntsc_chroma_sink") {
        std::cerr << "Unexpected stage name constant\n";
        return 1;
    }

    if (kStageNodeType != orc::NodeType::SINK) {
        std::cerr << "Expected SINK node type\n";
        return 1;
    }

    if (kStageMinInputs != 1 || kStageMaxInputs != 1 ||
        kStageMinOutputs != 0 || kStageMaxOutputs != 0) {
        std::cerr << "Expected one input and zero outputs\n";
        return 1;
    }

    return 0;
}
