#include "converter.h"

template <class T> const T& MIN (const T& a, const T& b)
{
  return ( (a) > (b) )? (b):(a);
}

const std::vector<char> converter::m_symbols = {'@', 'N','%', 'Q', 'g', 'm', '$', 'B', 'D', 'R', '#', '8', 'd',
                                              'O', 'b', 'U', 'A', 'q', 'h', 'G', 'p' , 'X', 'k', 'u', 'n', 'x',
                                              'c', '/', '*', '+' , ';', '^', '=', '´' , ':', '_', '.', '`', ' '};

void conversionParams::reset()
{
    m_inputFile.clear();
    m_outputFile.clear();
    m_levelNumber = 0;
    m_x = 0;
    m_y = 0;
}

const State conversionParams::computeState() const
{
    State l_state = UNINITIALIZED;

    if(!m_inputFile.empty() && !m_outputFile.empty() &&
       m_levelNumber != 0 && m_x != 0 && m_y != 0)
    {
        l_state = INITIALIZED;
    }

    return l_state;
}

converter::converter(const std::string& p_inputFile,
                     const std::string& p_outputFile,
                     const unsigned     p_levelNumber,
                     const unsigned     p_x,
                     const unsigned     p_y):
                         m_debugInfos(false),
                         m_keepRatio(true)
{
    m_params.m_inputFile = p_inputFile;
    m_params.m_outputFile = p_outputFile;
    m_params.m_levelNumber = MIN(p_levelNumber, static_cast<unsigned>(m_symbols.size()));
    m_params.m_x = p_x;
    m_params.m_y = p_y;
}

converter::~converter()
{
    m_output.close();
}

const State converter::init()
{
    m_output.open(m_params.m_outputFile, std::ios::trunc);
    if(!m_output.is_open())
    {
        if(hasDebugInfo())
            std::cout << "Unable to open output file " << m_params.m_outputFile << ".\n";
        return ERROR_OUTPUT_FILE;
    }

    if(!m_image.loadFromFile(m_params.m_inputFile))
    {
        if(hasDebugInfo())
            std::cout << "Unable to load image file " << m_params.m_inputFile << ".\n";
        return ERROR_LOADING_IMAGE_FILE;
    }
    else
    {
        sf::Vector2u l_size = m_image.getSize();
        if((l_size.x == 0) || (l_size.y == 0))
        {
            if(hasDebugInfo())
                std::cout << "Unable to load image - size issue. \n ";
            return ERROR_OUTPUT_FILE;
        }

        if(m_params.m_x == 0)
        {
            m_params.m_x = m_image.getSize().x;
        }

        if(m_params.m_y == 0)
        {
            m_params.m_y = m_image.getSize().y;
        }

        if(m_params.m_levelNumber == 0)
        {
            m_params.m_levelNumber = m_symbols.size();
        }

        if(m_keepRatio)
        {
            float l_imageRatio = static_cast<float>(m_image.getSize().y)/m_image.getSize().x;
            m_params.m_y = static_cast<unsigned>(l_imageRatio*m_params.m_x);
        }
    }

    return INITIALIZED;
}

const State converter::initFromFile(const std::string& p_configFile)
{
    m_params.reset();

    std::ifstream inFile(p_configFile);
    if (!inFile.is_open())
    {
        if(hasDebugInfo())
            std::cout << "Unable to load config.\n";
        return ERROR_LOADING_CONFIG_FILE;
    }

    std::string line;
    while (std::getline(inFile, line))
    {
        if (line == "size_x")
        {
            inFile >> m_params.m_x;
            if(hasDebugInfo())
                std::cout << "loaded param size_x: " << m_params.m_x << "\n";
        }
        else if (line == "size_y")
        {
            inFile >> m_params.m_y;
            if(hasDebugInfo())
                std::cout << "loaded param size_y: " << m_params.m_y << "\n";
        }
        else if (line == "nb_symbols")
        {
            inFile >> m_params.m_levelNumber;
            if(hasDebugInfo())
                std::cout << "loaded param nb_symbols: " << m_params.m_levelNumber << "\n";
        }
        else if (line == "inputFile")
        {
            inFile >>  m_params.m_inputFile;
            if(hasDebugInfo())
                std::cout << "loaded param input file: " << m_params.m_inputFile << "\n";
        }
        else if (line == "outputFile")
        {
            inFile >>  m_params.m_outputFile;
            if(hasDebugInfo())
                std::cout << "loaded param output file: " << m_params.m_outputFile << "\n";
        }
        else if (line == "keepRatio")
        {
            inFile >>  m_keepRatio;
            if(hasDebugInfo())
                std::cout << "loaded param keepRatio: " << m_keepRatio << "\n";
        }
    }

    State l_state = m_params.computeState();
    if(l_state == INITIALIZED)
    {
        return init();
    }

    return l_state;
}

bool converter::processImage()
{
    bool l_return = false;

    sf::Image l_resizedImage;
    l_resizedImage.create(m_params.m_y, m_params.m_x);

    sf::Vector2u l_originalSize = m_image.getSize();
    sf::Vector2u l_currentSize = l_resizedImage.getSize();

    unsigned l_XFactor = static_cast<unsigned>(l_originalSize.x/l_currentSize.x);
    unsigned l_YFactor = static_cast<unsigned>(l_originalSize.y/l_currentSize.y);

    resizeImage(l_resizedImage, l_XFactor, l_YFactor);

    if(hasDebugInfo())
    {
        std::cout << "create image size (" << m_params.m_y << " , " << m_params.m_x << ")\n";
        std::cout << "initial size: " << l_originalSize.y << " , " << l_originalSize.x << "\n";
        std::cout << "resized: " << l_currentSize.y << " , " << l_currentSize.x << "\n";
        std::cout << "(xFactor, yFactor) = (" << l_XFactor << " , " << l_YFactor << ")\n";
    }

    sf::Color l_currentColor;

    for(unsigned i=0; i<l_currentSize.y; ++i)
    {
        for(unsigned j=0;j<l_currentSize.x; ++j)
        {
                l_currentColor = l_resizedImage.getPixel(j,i);
                float GreyScale = (l_currentColor.r + l_currentColor.g + l_currentColor.b)/3;
                int nivelSaturacion = static_cast<unsigned>((GreyScale/255)*(m_params.m_levelNumber-1));
                m_output << m_symbols[nivelSaturacion] << "  ";
        }
            m_output << "\n";
    }

    std::cout << "Image converted!" << std::endl;
    return l_return;
}

void converter::resizeImage(sf::Image&     p_resizedImage,
                            const unsigned p_XFactor,
                            const unsigned p_YFactor)
{
    if((p_XFactor == 1) && (p_YFactor == 1))
    {
        p_resizedImage.copy(m_image, 0, 0);
        return;
    }

    sf::Vector2u l_dims = p_resizedImage.getSize();
    sf::Vector2u l_initialSize = m_image.getSize();
    sf::Color l_currentColor;

    for(unsigned i=0; i<l_dims.y; ++i)
    {
        for(unsigned j=0; j<l_dims.x; ++j)
        {
            float l_r(0), l_g(0), l_b(0);
            unsigned l_cmpt = 0;
            unsigned l_currentXEnd = (j+1)*p_XFactor;
            unsigned l_currentYEnd = (i+1)*p_YFactor;

            l_currentXEnd = MIN(l_currentXEnd, l_initialSize.x);
            l_currentYEnd = MIN(l_currentYEnd, l_initialSize.y);

            /*
            if(hasDebugInfo())
                std::cout << "Pixel (" << j << " , " << i << "): (" << j*p_XFactor <<
                             " , " << i*p_YFactor << ") => (" << l_currentXEnd <<
                             " , " << l_currentYEnd << ")\n";
            */

            for(unsigned k=i*p_YFactor; k<l_currentYEnd; ++k)
            {
                for(unsigned l=j*p_XFactor; l<l_currentXEnd; ++l)
                {
                    l_currentColor = m_image.getPixel(l,k);
                    ++l_cmpt;
                    l_r += l_currentColor.r;
                    l_g += l_currentColor.g;
                    l_b += l_currentColor.b;
                }
            }
            l_r/=l_cmpt;
            l_g/=l_cmpt;
            l_b/=l_cmpt;

            p_resizedImage.setPixel(j, i, sf::Color(l_r, l_g, l_b));
        }
    }
}
