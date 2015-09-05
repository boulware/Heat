#include <cstdint>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <ctime>
#include <iomanip>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

static const uint8_t CellOffset = 1; // [m]
static const uint16_t WindowWidth = 1280;
static const uint16_t WindowHeight = 720;

static const float c = 20;

struct timer
{
public:
    timer(int n, std::string Note) : n(n), TimeCount(0), TimeSum(0), OptimizationLog("opt.txt", std::ios::app)
    {
        if(OptimizationLog.is_open())
        {   
            std::time_t t = std::time(nullptr);
            
            OptimizationLog << "\n---------- " << std::put_time(std::localtime(&t), "%c %Z") << " ----------\n";
            OptimizationLog << "Note: " << Note << "\n\n";
        }
    }
    ~timer()
    {
        OptimizationLog.close();
    }
    
    uint64_t TimeCount;
    uint64_t TimeSum;
    sf::Clock Clock;

    uint64_t n;
    uint64_t N;
    std::ofstream OptimizationLog;
   
    float Average()
    {
        return float(TimeSum) / float(TimeCount);
    }
    
    void Begin()
    {   
        Clock.restart();
    }
    
    void End()
    {
        TimeCount++;
        TimeSum += Clock.getElapsedTime().asMicroseconds();

       if(TimeCount >= n)
        {   
            std::cout << "Avg: " << Average() << "us [" << TimeCount << "]\n";

            if(OptimizationLog.is_open())
            {
                OptimizationLog << "Avg: " << Average() << "us [" << TimeCount << "]\n";
            }
          
            TimeCount = 0;
            TimeSum = 0;
        }
    }
};

//static timer Timer(1000000, "Single draw call with individual setFillColor call + draw(Rects[]) call (T on cell POD)");

sf::Color hsv(int hue, float sat, float val)
{
  hue %= 360;
  while(hue<0) hue += 360;

  if(sat<0.f) sat = 0.f;
  if(sat>1.f) sat = 1.f;

  if(val<0.f) val = 0.f;
  if(val>1.f) val = 1.f;

  int h = hue/60;
  float f = float(hue)/60-h;
  float p = val*(1.f-sat);
  float q = val*(1.f-sat*f);
  float t = val*(1.f-sat*(1-f));

  switch(h)
  {
    default:
    case 0:
    case 6: return sf::Color(val*255, t*255, p*255);
    case 1: return sf::Color(q*255, val*255, p*255);
    case 2: return sf::Color(p*255, val*255, t*255);
    case 3: return sf::Color(p*255, q*255, val*255);
    case 4: return sf::Color(t*255, p*255, val*255);
    case 5: return sf::Color(val*255, p*255, q*255);
  }
}

struct cell
{
    float Temperature; // [K]
    float Resistance; // [(m*K)/W]
};

class grid : public sf::Drawable, public sf::Transformable
{
public:
    uint16_t Width, Height;

    sf::Vector2u CellSize;
    
    std::vector<cell>* CurrentBuffer;
    std::vector<cell>* NextBuffer;
    std::vector<cell> Buffers[2];

    sf::Shader CellTemperatureShader;

    void Randomize()
    {
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_real_distribution<float> RandomSingle(0.0, 1.0);
        
        for(auto& Cell : *CurrentBuffer)
        {
            Cell.Temperature = RandomSingle(mt);
//        Cell.Resistance = RandomUInt8(mt);
        }
    }
    
    unsigned int GetCellIndex(unsigned int x, unsigned int y)
    {
        unsigned int Index = (y * Width) + x;

        // TODO(tyler): Clean this up.
        if(Index < Width * Height)
        {
            return (y * Width) + x;
        }
        else
        {
            return 0;
        }
    }
    
//    std::vector<sf::RectangleShape> Rects;
    sf::VertexArray m_vertices;
    
    static const uint16_t MaxSize = 1000;
    
    grid(unsigned int Width, unsigned int Height, sf::Vector2u CellSize);

    void SetCellTemperatureArea(int CellX, int CellY, float Temperature)
    {
        unsigned int HalfSize = 5;
        
        for(unsigned int y = CellY - HalfSize; y < CellY + HalfSize; y++)
        {
            for(unsigned int x = CellX - HalfSize; x < CellX + HalfSize; x++)
            {
                CurrentBuffer->at(GetCellIndex(x, y)).Temperature = Temperature;
            }
        }
    }
    
    void SyncCellSize()
    {
        m_vertices.resize(Width * Height * 4);
        
        for(unsigned int y = 0; y < Height; y++)
        {
            for(unsigned int x = 0; x < Width; x++)
            {
                sf::Vertex* Quad = &m_vertices[4 * (y * Width + x)];

                Quad[0].position = sf::Vector2f(x * CellSize.x, y * CellSize.y);
                Quad[1].position = sf::Vector2f((x + 1) * CellSize.x, y * CellSize.y);
                Quad[2].position = sf::Vector2f((x + 1) * CellSize.x, (y + 1) * CellSize.y);
                Quad[3].position = sf::Vector2f(x * CellSize.x, (y + 1) * CellSize.y);

                float T = CurrentBuffer->at(y * Width + x).Temperature;
                sf::Color TemperatureColor = hsv(T * 100.0, 0.9, 0.9);
                Quad[0].color = TemperatureColor;
                Quad[1].color = TemperatureColor;
                Quad[2].color = TemperatureColor;
                Quad[3].color = TemperatureColor;
            }
        }   
    }
    
    void OutputState();
    void TransferCellHeat(int Source, int Target);
    void ConductHeat();
    void UpdateTemperature(int x, int y);

    void SwapBuffers()
    {
        std::vector<cell>* Temp = CurrentBuffer;
        CurrentBuffer = NextBuffer;
        NextBuffer = Temp;
    }

    void UpdateCellColors()
    {
        
        for(unsigned int y = 0; y < Height; y++)
        {
            for(unsigned int x = 0; x < Width; x++)
            {
                sf::Vertex* Quad = &m_vertices[4 * (y * Width + x)];

                float T = CurrentBuffer->at(y * Width + x).Temperature;
                sf::Color TemperatureColor = hsv(T * 100.0, 0.9, 0.9);
                Quad[0].color = TemperatureColor;
                Quad[1].color = TemperatureColor;
                Quad[2].color = TemperatureColor;
                Quad[3].color = TemperatureColor;
            }
        }
    }
    
private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        states.transform *= getTransform();
        
        target.draw(m_vertices, states);
    }

};

grid::grid(unsigned int Width, unsigned int Height, sf::Vector2u CellSize)
        :
        Width(Width), Height(Height), CellSize(CellSize),
        CurrentBuffer(&Buffers[0]), NextBuffer(&Buffers[1])
{
    CurrentBuffer->resize(Width * Height);
    NextBuffer->resize(Width * Height);
    
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> RandomSingle(0.0, 1.0);

    for(auto& Cell : *CurrentBuffer)
    {
        Cell.Temperature = RandomSingle(mt);
//        Cell.Resistance = RandomUInt8(mt);
    }

    m_vertices.setPrimitiveType(sf::Quads);

    SyncCellSize();
    
    *NextBuffer = *CurrentBuffer;
}

void grid::TransferCellHeat(int Source, int Target)
{
//    Timer.Begin();
    float dT = CurrentBuffer->at(Target).Temperature - CurrentBuffer->at(Source).Temperature;
//    float R = CurrentCells[Target].Resistance + CurrentCells[Source].Resistance;
    float R = 256;
    float q = c * dT / R;
    
    if(abs(q) < 0.4 * abs(dT))
    {
        NextBuffer->at(Target).Temperature -= q;
        NextBuffer->at(Source).Temperature += q;
    }
    else
    {
        float AverageT = (CurrentBuffer->at(Target).Temperature + CurrentBuffer->at(Source).Temperature) / 2.0;
                        
        NextBuffer->at(Target).Temperature = AverageT;
        NextBuffer->at(Source).Temperature = AverageT;
    }
//    Timer.End();
}

void grid::UpdateTemperature(int x, int y)
{
//    Timer.Begin();
    std::vector<cell*> Neighbors;

    // If the cell is not on the left border, it has a left neighbor.
    if(x != 0) Neighbors.push_back(&NextBuffer->at(GetCellIndex(x - 1, y)));
        if(x != Width - 1) Neighbors.push_back(&NextBuffer->at(GetCellIndex(x + 1, y)));
    // If the cell is not on the up border, it has an up neighbor.
    if(y != 0) Neighbors.push_back(&NextBuffer->at(GetCellIndex(x, y - 1)));
    // ...
    if(y != Height - 1) Neighbors.push_back(&NextBuffer->at(GetCellIndex(x, y + 1)));

    float TemperatureSum = c * Neighbors.size() * NextBuffer->at(GetCellIndex(x, y)).Temperature;

    for(auto& Cell : Neighbors)
    {
        TemperatureSum += Cell->Temperature;
    }

    NextBuffer->at(GetCellIndex(x, y)).Temperature = TemperatureSum / (c * Neighbors.size() + Neighbors.size());
//    Timer.End();
}

void grid::ConductHeat()
{
    // SwapBuffers();
    *NextBuffer = *CurrentBuffer;

#if 1
    for(int y = 0; y < Height; y++)
    {
        for(int x = 0; x < Width; x++)
        {
            int i = (y * Width) + x;
            
            if(x == Width - 1 || y == Height - 1)
            {
                // Cells on right, but not bottom-right
                if(x == Width - 1 && y != Height - 1)
                {
                    // Down neighbor
                    TransferCellHeat(i, i + Width);
                }
                // Cells on bottom, but not bottom-right
                if(y == Height - 1 && x != Width - 1)
                {
                    // Right neighbor 
                    TransferCellHeat(i, i + 1);
                }
            }
            else
            {                
                // Right neighbor
                TransferCellHeat(i, i + 1);
                // Down neighbor
                TransferCellHeat(i, i + Width);
            }
        }
    }

    UpdateCellColors();
    
#endif
    
    // Cells not touching a border
#if 0
//    Timer.Begin();
    for(int y = 0; y < Height; y++)
    {
        for(int x = 0; x < Width; x++)
        {
            UpdateTemperature(x, y);
        }
    }

    UpdateCellColors();
//    Timer.End();
    //
#endif

        *CurrentBuffer = *NextBuffer;
}

int main()
{
    sf::RenderWindow Window(sf::VideoMode(WindowWidth, WindowHeight), "Window");
    Window.setFramerateLimit(60);

    grid MainGrid(1280, 720, {1, 1});

    bool start = false;

    uint64_t TimeSum = 0;
    uint64_t TimeCount = 0;

    Window.draw(MainGrid);
    Window.display();
    
    while(Window.isOpen())
    {        
        sf::Event Event;
        while(Window.pollEvent(Event))
        {
            if(Event.type == sf::Event::Closed) Window.close();
        }

        if(Window.hasFocus())
        {
            if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                sf::Vector2i MousePosition = sf::Mouse::getPosition(Window);
                MousePosition.x = std::max(0, std::min(MousePosition.x, WindowWidth - 1));
                MousePosition.y = std::max(0, std::min(MousePosition.y, WindowHeight - 1));
            
//            MainGrid.CurrentBuffer->at(MainGrid.GetCellIndex(MousePosition.x / (WindowWidth / MainGrid.Width), MousePosition.y / (WindowHeight / MainGrid.Height))).Temperature = 0;

                int CellX = MousePosition.x / (WindowWidth / MainGrid.Width);
                int CellY = MousePosition.y / (WindowHeight / MainGrid.Height);
                MainGrid.SetCellTemperatureArea(CellX, CellY, 1.0);
            }
        
            if(sf::Mouse::isButtonPressed(sf::Mouse::Right))
            {
                sf::Vector2i MousePosition = sf::Mouse::getPosition(Window);
                MousePosition.x = std::max(0, std::min(MousePosition.x, WindowWidth - 1));
                MousePosition.y = std::max(0, std::min(MousePosition.y, WindowHeight - 1));

                int CellX = MousePosition.x / (WindowWidth / MainGrid.Width);
                int CellY = MousePosition.y / (WindowHeight / MainGrid.Height);
                MainGrid.SetCellTemperatureArea(CellX, CellY, 0.0);
//                MainGrid.CurrentBuffer->at(MainGrid.GetCellIndex(MousePosition.x / (WindowWidth / MainGrid.Width), MousePosition.y / (WindowHeight / MainGrid.Height))).Temperature = 255;
            }

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            {
                start = true;
            }

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::R))
            {
                MainGrid.Randomize();
            }
        }
        if(start == true) MainGrid.ConductHeat();
        
        Window.clear();

//        MainGrid.UpdateCellColors();
        Window.draw(MainGrid);
        
        Window.display();
    }

//    std::cout << "Timer: " << ( << "us [n = " << TimeCount << "]\n> ";

    std::string Comment;
//    std::getline(std::cin, Comment);
}
