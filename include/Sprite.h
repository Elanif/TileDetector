#ifndef SPRITE_H
#define SPRITE_H
#include<SFML/Graphics.hpp>

class Sprite {
public:
    Sprite() ;
    Sprite(const sf::Image& img) ;
    void print();
    bool color_initialized=false;
    unsigned char arr[8][8];
    std::pair<size_t,size_t> colors[4];
};
#endif // SPRITE_H
