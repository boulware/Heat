#include <thread>
#include <iostream>
#include <mutex>
#include <string>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

#include "constants.hpp"

#include "material.hpp"
#include "cell.hpp"
#include "cell_grid.hpp"
#include "item.hpp"
#include "temperature_scale.hpp"

#include "item.cpp"
#include "temperature_scale.cpp"
#include "component.cpp"
#include "component_grid.cpp"
#include "surface.cpp"
#include "grid_texture.cpp"

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

//    sf::Text BottleTemps("", constants::fontCourierNew, 20);
//    BottleTemps.setPosition(0, 40);

    sf::Text GameTimeText("-", constants::fontCourierNew, 20);
    sf::Text FPSText("FPS: 00 ", constants::fontCourierNew, 20);
    FPSText.setPosition(constants::WindowWidth - FPSText.getLocalBounds().width, 0);

    temperature_scale TemperatureScale(273.f, 373.f, {0.f, static_cast<float>(constants::WindowHeight) - 10.f, static_cast<float>(constants::WindowWidth), 10.f});
    
    environment ColdRoom = {250.0, constants::Materials["air"]};
    environment HotRoom = {500.0, constants::Materials["air"]};
    item Bottle("assets/waterbottle.itm", ColdRoom);
    Bottle.MoveToEnvironment(HotRoom);
    Bottle.setScale(2.0, 2.0);

    sf::Text BottleStateText("", constants::fontCourierNew, 20);
    BottleStateText.setPosition(0.f, 100.f);
    
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

        GameTimeText.setString(std::string("runtime: ") + std::to_string(GameTime.getElapsedTime().asSeconds()) + std::string("s"));
        BottleStateText.setString(Bottle.GetStateString());
        
        Window.draw(Bottle);

        Window.draw(TemperatureScale);
        
        Window.draw(BottleStateText);
        Window.draw(GameTimeText);
        Window.draw(FPSText);
        
        Window.display();
        FrameCount++;
    }
}
