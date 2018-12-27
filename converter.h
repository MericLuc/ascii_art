#ifndef CONVERTER_H
#define CONVERTER_H

#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>
#include <list>

enum State
{
    UNINITIALIZED = -1,
    INITIALIZED = 1,
    ERROR_OUTPUT_FILE = -2,
    ERROR_LOADING_IMAGE_FILE = -4,
    ERROR_LOADING_CONFIG_FILE = -8
};

typedef struct
{
    unsigned    m_x;
    unsigned    m_y;
    unsigned    m_levelNumber;
    std::string m_inputFile;
    std::string m_outputFile;

    void reset();

    const State computeState() const;
} conversionParams;

class converter
{
    public:
        converter(const std::string& p_inputFile,
                  const std::string& p_outputFile,
                  const unsigned     p_levelNumber = 0,
                  const unsigned     p_x = 0,
                  const unsigned     p_y = 0);

        ~converter();

        const State init();

        const State initFromFile(const std::string& p_configFile);

        bool processImage();

        void resizeImage(sf::Image&         p_resizedImage,
                         const unsigned     p_xFactor,
                         const unsigned     p_yFactor);

        void setDebugInfo(const bool p_info) { m_debugInfos = p_info; }
        const bool hasDebugInfo() const { return m_debugInfos; }

        void keepAspectRatio(const bool p_keepAspectRatio) { m_keepRatio = p_keepAspectRatio; }

    private:
        bool                           m_debugInfos;
        bool                           m_keepRatio;
        sf::Image                      m_image;
        std::ofstream                  m_output;
        conversionParams               m_params;
        static const std::vector<char> m_symbols;


};

#endif // CONVERTER_H
