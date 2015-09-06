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

#include <timer.hpp>
#include "constants.hpp"


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
    double Temperature; // [K]
    double InverseConductivity; // [(m*K)/W]
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
        std::uniform_real_distribution<float> RandomSingleN(0.0, 1.0);
        std::uniform_real_distribution<float> RandomSingle1000(0.0, 1000.0);
        
        for(auto& Cell : *CurrentBuffer)
        {
//            Cell.Temperature = RandomSingleN(mt);
//            Cell.InverseConductivity = RandomSingleN(mt);
            Cell.Temperature = RandomSingle1000(mt);
            Cell.InverseConductivity = RandomSingle1000(mt);
        }
    }

    cell* GetCell(uint16_t x, uint16_t y)
    {
        if(x > Width || y > Height)
        {
            return nullptr;
        }

        return &CurrentBuffer->at(y * Width + x);
    }
    
    sf::VertexArray m_vertices;
    
    static const uint16_t MaxSize = 1000;
    
    grid(unsigned int Width, unsigned int Height, sf::Vector2u CellSize);

    void SetCellTemperatureArea(int CellX, int CellY, float Temperature)
    {
        unsigned int HalfSize = 1;
        
        for(unsigned int y = CellY - HalfSize; y < CellY + HalfSize; y++)
        {
            for(unsigned int x = CellX - HalfSize; x < CellX + HalfSize; x++)
            {
                cell* CurrentCell = GetCell(x, y);
                if(CurrentCell)
                {
                    CurrentCell->Temperature = Temperature;
                }
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
    std::uniform_real_distribution<float> RandomSingleN(0.0, 1.0);
    std::uniform_real_distribution<float> RandomSingleNSafe(0.5, 1.0);

        /*
    for(auto& Cell : *CurrentBuffer)
    {
        Cell.Temperature = RandomSingleN(mt);
        Cell.InverseConductivity = RandomSingleNSafe(mt);
    }
        */

    for(int x = 0; x < Width/2; x++)
    {
        for(int y = 0; y < Height; y++)
        {
            cell* CurrentCell = GetCell(x, y);

            if(CurrentCell != nullptr)
            {
                CurrentCell->Temperature = 1.0;
                CurrentCell->InverseConductivity = 0.5;
            }
        }
    }

    for(int x = Width/2; x < Width; x++)
    {
        for(int y = 0; y < Height; y++)
        {
            cell* CurrentCell = GetCell(x, y);

            if(CurrentCell)
            {
                CurrentCell->Temperature = 0.0;
                CurrentCell->InverseConductivity = 0.5;
            }
        }
    }
    /*    
    int x = Width/2;
    for(int y = 0; y < Height; y++)
    {
        cell* CurrentCell = GetCell(x, y);

        
        if(CurrentCell != nullptr && (y < 2 || y > 2))
        {
            CurrentCell->Temperature = 2.0;
            CurrentCell->InverseConductivity = 100000;
        }
        else
        {
            CurrentCell->InverseConductivity = 0.5;
        }
    }
    */
    m_vertices.setPrimitiveType(sf::Quads);

    SyncCellSize();
    
    *NextBuffer = *CurrentBuffer;
}

void grid::TransferCellHeat(int Source, int Target)
{
    double dT = CurrentBuffer->at(Target).Temperature - CurrentBuffer->at(Source).Temperature;
    double R = CurrentBuffer->at(Target).InverseConductivity + CurrentBuffer->at(Source).InverseConductivity;
    double Q = c * (dT / R);

    float MinimumHeatExchange = 0.01;
    
    if(Q >= MinimumHeatExchange)
    {
        if(abs(Q) < 0.5 * abs(dT))
        {
            NextBuffer->at(Target).Temperature -= Q;
            NextBuffer->at(Source).Temperature += Q;
        }
        else
        {
            float AverageT = (CurrentBuffer->at(Target).Temperature + CurrentBuffer->at(Source).Temperature) / 2.0;
                        
            NextBuffer->at(Target).Temperature = AverageT;
            NextBuffer->at(Source).Temperature = AverageT;
        }
    }
    else
    {
        
    }
}
/*
void grid::UpdateTemperature(int x, int y)
{
    uint8_t NeighborCount;
    float TemperatureSum = 0.0;

    // If the cell is not on the left border, it has a left neighbor.
    std::vector<cell*> Neighbors(4, nullptr);

    Neighbors.push_back(GetCell(x - 1, y));
    Neighbors.push_back(GetCell(x + 1, y));
    Neighbors.push_back(GetCell(x, y - 1));
    Neighbors.push_back(GetCell(x, y + 1));

    for(auto& neighbor : Neighbors)
    {
        if(neighbor != nullptr)
        {
            TemperatureSum += neighbor.Temperature;
            NeighborCount++;
        }
    }
    

    float TemperatureSum = c * Neighbors.size() * NextBuffer->at(GetCellIndex(x, y)).Temperature;

    for(auto& Cell : Neighbors)
    {
        TemperatureSum += Cell->Temperature;
    }

    NextBuffer->at(GetCellIndex(x, y)).Temperature = TemperatureSum / (c * Neighbors.size() + Neighbors.size());
}
*/
void grid::ConductHeat()
{
    
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
    
#endif
    
#if 0
    for(int y = 0; y < Height; y++)
    {
        for(int x = 0; x < Width; x++)
        {
            UpdateTemperature(x, y);
        }
    }

#endif

    UpdateCellColors();
    
        *CurrentBuffer = *NextBuffer;
}

int main()
{   
    sf::RenderWindow Window(sf::VideoMode(WindowWidth, WindowHeight), "Window");
    
    Window.setFramerateLimit(60);

    // GD = 2
    // TransferCellHeat() -> 42FPS
    // UpdateTemperature() -> 12FPS
    
    unsigned int GridDivider = 10;
    grid MainGrid(WindowWidth / GridDivider, WindowHeight / GridDivider, {GridDivider, GridDivider});

    bool start = false;

    uint64_t TimeSum = 0;
    uint64_t TimeCount = 0;

    Window.draw(MainGrid);
    Window.display();
    
    xish::timer Timer;
    Timer.AddDescriptor("opt_level", "O2");
    Timer.AddDescriptor("build", "r");
    Timer.AddDescriptor("function", "ConductHeat");
    Timer.AddDescriptor("pixel_count", WindowWidth * WindowHeight);
    Timer.AddDescriptor("cell_count", MainGrid.CurrentBuffer->size());
    
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
        
        if(start == true)
        {
            MainGrid.ConductHeat();
        }
        
        Window.clear();

        Window.draw(MainGrid);
        
        Window.display();

    }
}
