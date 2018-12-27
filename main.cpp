#include "converter.h"

int main()
{
    std::string l_inputFile = "..\\..\\files\\test.png";
    std::string l_outputFile = "..\\..\\files\\output.txt";
    std::string l_confFile = "..\\..\\files\\config.txt";

    converter conv(l_inputFile, l_outputFile, 0, 0, 0);
    conv.keepAspectRatio(true);
    conv.setDebugInfo(true);

    conv.initFromFile(l_confFile);

    if(conv.init() == INITIALIZED)
    {
        conv.processImage();
    }

    return 0;
}
