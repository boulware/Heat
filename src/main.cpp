//#include <vector>
#include <thread>
#include <iostream>
#include <mutex>
#include <string>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

#include "constants.hpp"

#include "cell.hpp"
#include "cell_grid.hpp"

static void PixelsToCellCoords(int X, int Y, int& CellX, int& CellY, cell_grid& Grid)
{
    CellX = X / (constants::WindowWidth / Grid.mWidth);
    CellY = Y / (constants::WindowHeight / Grid.mHeight);
}

static std::mutex CountMutex;

static void LoopStepSubCollection(bool* Quit, unsigned int* ExecutionCount, cell_grid* Grid, unsigned int MinIndex, unsigned int MaxIndex)
{   
    while(!(*Quit))
    {
        Grid->StepSubCollection(MinIndex, MaxIndex);

        CountMutex.lock();
        (*ExecutionCount)++;
        CountMutex.unlock();
    }
}

class temperature_scale : public sf::Drawable, public sf::Transformable
{
private:
    sf::FloatRect mRect;
    float mMinTemp, mMaxTemp;
    sf::Text mMinText, mMaxText;
    sf::Texture mTexture;
    sf::Sprite mSprite;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {       
        states.transform *= getTransform();

        target.draw(mSprite, states);
        target.draw(mMinText, states);
        target.draw(mMaxText, states);
    }
 
public:
    temperature_scale(float MinTemp, float MaxTemp, sf::FloatRect Rect)
            :
            mMinTemp(MinTemp), mMaxTemp(MaxTemp),
            mMinText(std::to_string(static_cast<int>(MinTemp)), constants::fontCourierNew, 18),
            mMaxText(std::to_string(static_cast<int>(MaxTemp)), constants::fontCourierNew, 18),
            mRect(Rect)
    {
        setPosition(mRect.left, mRect.top);
        mSprite.setPosition(0, 0);
        mSprite.setScale(mRect.width / static_cast<float>(constants::Colors.size() / 4), mRect.height);
//        
        
        sf::Image Image;
        Image.create(constants::Colors.size() / 4, 1, constants::Colors.data());
        
        if(!mTexture.loadFromImage(Image))
        {
            std::cout << "\nFailed to load texture";
        }

        mSprite.setTexture(mTexture);

        float TextYOffset = 5.0;
        float TextXOffset = 5.0;
        
        mMinText.setOrigin(0.f, 18.f);
        mMinText.setPosition(TextXOffset, -TextYOffset);

        mMaxText.setOrigin(mMinText.getLocalBounds().width, 18.f);
        mMaxText.setPosition(mRect.width - TextXOffset, -TextYOffset);
    }
   
};

enum class phase
{
    Invalid,
    Solid,
    Liquid,
    Gas,
};

static std::string PhaseToString(phase Phase)
{
    std::string PhaseString;
    
    switch(Phase)
    {
        case(phase::Invalid): PhaseString = "Invalid"; break;
        case(phase::Solid): PhaseString = "Solid"; break;
        case(phase::Liquid): PhaseString = "Liquid"; break;
        case(phase::Gas): PhaseString = "Gas"; break;
    }

    return PhaseString;
}

struct material
{
    float Resistance;
    float MeltingPoint;
    float BoilingPoint;
};

static phase GetMaterialPhase(material& Material, float Temperature, float Pressure = 0)
{
    if(Temperature < Material.MeltingPoint) return phase::Solid;
    else if(Temperature < Material.BoilingPoint) return phase::Liquid;
    else return phase::Gas;
}

class component
{
private:
    phase Phase;
public:
    std::vector<sf::Vector2u> Positions;
    float Temperature;
    material Material;

    phase GetPhase() const {return Phase;}
    void UpdatePhase()
    {
        Phase = GetMaterialPhase(Material, Temperature);
    }
};

struct environment
{
    float Temperature;
    float Resistance;
};

#include <fstream>
class item : public sf::Drawable, public sf::Transformable
{
private:
    cell_grid mHeatGrid;
    environment mCurrentEnvironment;
    std::vector<sf::Vector2u> mEnvironmentCellCoordinates;
    // TODO(tyler): ComponentMaps is not really a good name.
    std::map<char, component> mComponentMaps;
public:
    item(std::string ItemFile, environment SpawnEnvironment);

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
    
    void ResetEnvironmentCells()
    {
        for(auto& env_cell_position : mEnvironmentCellCoordinates)
        {
            mHeatGrid.SetCell(env_cell_position.x, env_cell_position.y, mCurrentEnvironment.Temperature, mCurrentEnvironment.Resistance);
        }
    }
    
    void Update()
    {
        ResetEnvironmentCells();
        for(auto& component_map : mComponentMaps)
        {
            component& Component = component_map.second;

            Component.UpdatePhase();
        }
        mHeatGrid.StepSubCollection();
        mHeatGrid.UpdateCellColors();
    }

    environment& GetEnvironment()
    {
        return mCurrentEnvironment;
    }
    
    void SetEnvironment(environment TargetEnvironment)
    {
        mCurrentEnvironment = TargetEnvironment;
        ResetEnvironmentCells();
    }

    // TODO(tyler): ComponentID is intrinsically linked to a MaterialID, which is not a good thing.
    float GetComponentTemperature(char ComponentID)
    {
        if(mComponentMaps.find(ComponentID) != mComponentMaps.end())
        {
            // The component exists in this item
            float TemperatureSum = 0;

            for(auto& cell_position : mComponentMaps[ComponentID].Positions)
            {
                TemperatureSum += mHeatGrid.GetCell(cell_position.x, cell_position.y)->Temperature;
            }

            return TemperatureSum / (float)mComponentMaps[ComponentID].Positions.size();
        }
        else
        {
            // The component is not a component of this item
            std::cout << "\nTried to get an item's component's temperature, but the component was not a part of that item!";

            return -1.0;
        }
    }

    phase GetComponentPhase(char ComponentID)
    {
        if(mComponentMaps.find(ComponentID) != mComponentMaps.end())
        {
            // The component exists in this item
            return mComponentMaps[ComponentID].GetPhase();
        }
        else
        {
            // The component is not a component of this item
            std::cout << "\nTried to get an item's component's phase, but the component was not a part of that item!";

            return phase::Invalid;
        }        
    }
};

item::item(std::string ItemFilePath, environment SpawnEnvironment)
        :
        mCurrentEnvironment(SpawnEnvironment)
{

    
    // NOTE(tyler): All of this should probably be moved into a chained call to a cell_grid constructor.
    std::ifstream ItemFile(ItemFilePath);

    unsigned int ItemWidth = 0;
    unsigned int ItemHeight = 0;

    // TODO(tyler): Implement a check so that all grid cells must be connected? Maybe not?
    if(ItemFile.is_open())
    {
        std::string Line;

        // NOTE(tyler): It might be a good idea to check during this loop whether there are invalid materials.
        while(ItemFile >> Line)
        {
            ItemWidth = std::max(ItemWidth, (unsigned)Line.length());
            ItemHeight++;
        } 

        mHeatGrid = cell_grid(ItemWidth, ItemHeight);
        mHeatGrid.c = 1.0;
        mHeatGrid.MinTemp = 273.0;
        mHeatGrid.MaxTemp = 373.0;
    
        ItemFile.clear();
        ItemFile.seekg(0, ItemFile.beg);
        char CellID;
        unsigned int CellX = 0;
        unsigned int CellY = 0;
        
        while(ItemFile >> std::noskipws >> CellID)
        {
            switch(CellID)
            {
                case('\n'):
                {
                    CellY++;
                    CellX = 0;
                } break;
                case('W'):
                {
                    // NOTE(tyler): If the material has not yet been found, add it to the list of components in the item
                    if(mComponentMaps.find(CellID) == mComponentMaps.end())
                    {
                        component Contents;
                        material Water = {1.8, 273.0, 373.0};
                        Contents.Material = Water;
                        mComponentMaps.insert(std::pair<char, component>(CellID, Contents));
                    }
                    
                    mHeatGrid.SetCell(CellX, CellY, mCurrentEnvironment.Temperature, mComponentMaps[CellID].Material.Resistance);
                    mComponentMaps[CellID].Positions.push_back({CellX, CellY});
                    
                    CellX++;
                } break;
                case('S'):
                {
                    if(mComponentMaps.find(CellID) == mComponentMaps.end())
                    {
                        component Container;
                        material Styrofoam = {1000.0, 513.0, 1e3};
                        Container.Material = Styrofoam;
                        mComponentMaps.insert(std::pair<char, component>(CellID, Container));
                    }
                    
                    mHeatGrid.SetCell(CellX, CellY, mCurrentEnvironment.Temperature, mComponentMaps[CellID].Material.Resistance);
                    mComponentMaps[CellID].Positions.push_back({CellX, CellY});

                    CellX++;
                } break;
                case('.'):
                {
                    mHeatGrid.SetCell(CellX, CellY, mCurrentEnvironment.Temperature, mCurrentEnvironment.Resistance);
                    mEnvironmentCellCoordinates.push_back({CellX, CellY});
                    CellX++;
                }
            }
        }
    }
    else
    {
        std::cout << "\nFailed to open " << ItemFilePath;
    }


    std::cout << "\nItem size is (" << ItemWidth << ", " << ItemHeight << ")";
}

void item::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();

    target.draw(mHeatGrid, states);
}

int main()
{
    sf::RenderWindow Window(sf::VideoMode(constants::WindowWidth, constants::WindowHeight), "Window");//, sf::Style::Fullscreen);
    Window.setFramerateLimit(60);

        if(!constants::fontCourierNew.loadFromFile("assets/cour.ttf"))
        {
        std::cout << "\nFailed to load font.";
        }
    
//    cell_grid Grid(constants::WindowWidth / Divider, constants::WindowHeight / Divider);
//    Grid.c = 0.0;

    bool Quit = false;
    
    // On O2 + release (100, 100)
    //1200FPS on 1 thread
    //575FPS on main thread only
    //About 2700 grid updates/s + 1200FPS with 3 cores

    // O2 + r ( 1280, 720)
    // 16FPS and 25 grid updates/s
    // Versus 6-7 grid updates/s + 6-7 FPS in old heat program

    // O2 + r (640, 360)
    // 60-65FPS and ~100-105 grid updates/s
    // Versus 30 grid updates/s + 30FPS in old heat program

    // O2 + r (1280, 720) when cell members changed to doubles from floats
    // 11FPS and 18 grid updates/s
    
   
/*
    unsigned int ThreadCount = std::thread::hardware_concurrency() - 1;
    unsigned int CellsPerThread = (Grid.mWidth * Grid.mHeight) / ThreadCount;

    std::vector<std::thread> Threads;
    
    for(unsigned int thread_index = 0; thread_index < ThreadCount; thread_index++)
    {
        unsigned int MinIndex = CellsPerThread * thread_index;
        unsigned int MaxIndex = CellsPerThread * (thread_index + 1) - 1;

        if(thread_index == ThreadCount - 1)
        {
            MaxIndex = Grid.mWidth * Grid.mHeight;
        }

        Threads.push_back(std::thread(std::bind(LoopStepSubCollection, &Quit, &LoopCount, &Grid, MinIndex, MaxIndex)));
        std::cout << "\nSplitting thread " << thread_index << " on [" << MinIndex << ", " << MaxIndex << "]";
    }
*/
//    sf::Text TestText("sdklfjasldfjskdf", constants::fontCourierNew, 10);
//    std::cout << "\nlocal top: " << TestText.getLocalBounds().height - 

    sf::Text BottleTemps("", constants::fontCourierNew, 20);
    BottleTemps.setPosition(0, 40);
    sf::Text GameTimeText("-", constants::fontCourierNew, 20);
    sf::Text FPSText("FPS: 00 ", constants::fontCourierNew, 20);
    FPSText.setPosition(constants::WindowWidth - FPSText.getLocalBounds().width, 0);

    temperature_scale TemperatureScale(273.f, 373.f, {0.f, static_cast<float>(constants::WindowHeight) - 10.f, static_cast<float>(constants::WindowWidth), 10.f});
    
    environment ColdRoom = {333.0, 42.0};
    environment HotRoom = {358.0, 42.0};
    item Bottle("assets/waterbottle.itm", ColdRoom);
    Bottle.SetEnvironment(HotRoom);
    Bottle.setScale(2.0, 2.0);
    
    sf::Time LoopTime = sf::seconds(1.0);
    unsigned int LoopCount = 0;
    unsigned int FrameCount = 0;
    
    sf::Clock GameTime;
    sf::Clock Timer;
    
    while(Window.isOpen())
    {
        sf::Event Event;
        while(Window.pollEvent(Event))
        {
            if(Event.type == sf::Event::Closed)
            {
                Window.close();
                Quit = true;
/*
                for(int thread_index = 0; thread_index < Threads.size(); thread_index++)
                {
                    Threads[thread_index].join();
                    std::cout << "\nJoining thread " << thread_index;
                }
*/
            }
        }

        /*
        if(Window.hasFocus())
        {
            if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                sf::Vector2i MousePosition = sf::Mouse::getPosition(Window);

                int Radius = 2;
                int CellX, CellY;
                PixelsToCellCoords(MousePosition.x, MousePosition.y, CellX, CellY, Grid);
                Grid.SetCell(CellX, CellY, 0.1, 1.8);
            }
        
            if(sf::Mouse::isButtonPressed(sf::Mouse::Right))
            {
                sf::Vector2i MousePosition = sf::Mouse::getPosition(Window);

                int Radius = 2;
                int CellX, CellY;
                PixelsToCellCoords(MousePosition.x, MousePosition.y, CellX, CellY, Grid);
                Grid.SetCell(CellX, CellY, 0.2, 30.0);
            }

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            {
                Grid.c = 0.01;
            }
            
        }
        */

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        {
        }
        
        if(Timer.getElapsedTime() >= LoopTime)
        {
            CountMutex.lock();
            LoopCount = 0;
            CountMutex.unlock();
            std::cout << "\nFPS:" << FrameCount;
            FPSText.setString(std::string("FPS: ") + std::to_string(FrameCount));
            FrameCount = 0;
            Timer.restart();
        }
        
        Window.clear();

        sf::Vector2i MousePosition = sf::Mouse::getPosition(Window);
        Bottle.setPosition(MousePosition.x, MousePosition.y);
        
        Bottle.Update();
        
        BottleTemps.setString(std::string("environment T: ") + std::to_string(Bottle.GetEnvironment().Temperature));
        BottleTemps.setString(BottleTemps.getString() + std::string("\nwater T [") + PhaseToString(Bottle.GetComponentPhase('W')) + std::string("]: ") + std::to_string(Bottle.GetComponentTemperature('W')));
        BottleTemps.setString(BottleTemps.getString() + std::string("\nstyrofoam T [") + PhaseToString(Bottle.GetComponentPhase('S')) + std::string("]: ") + std::to_string(Bottle.GetComponentTemperature('S')));

        GameTimeText.setString(std::string("runtime: ") + std::to_string(GameTime.getElapsedTime().asSeconds()) + std::string("s"));
        
        Window.draw(Bottle);

        Window.draw(TemperatureScale);
        
        Window.draw(BottleTemps);
        Window.draw(GameTimeText);
        Window.draw(FPSText);
        
        Window.display();
        FrameCount++;
    }
}
