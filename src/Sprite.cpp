#include "Sprite.h"
constexpr size_t maxcolor=4;
const sf::Vector2u tilesize{8,8};
size_t lookup(sf::Color color, sf::Color* palette_color, size_t colors_found) {
    for (size_t i=0; i<colors_found; ++i)
        if (palette_color[i]==color) return i;
    return -1;
}

Sprite::Sprite() {
    for (size_t i=0; i<8; ++i)
        for (size_t j=0; j<8; ++j)
            arr[i][j]=0;
}
Sprite::Sprite(const sf::Image& img) {
    color_initialized=true;
    sf::Color palette_color1[maxcolor];//={0,0,0,0};
    size_t colors_found=1; //first color should always be black/transparent which is 0x0D= {0,13}
    palette_color1[0]=sf::Color::Black;
    colors[0]=colors[1]=colors[2]=colors[3]={0,0xD};
    for (size_t i=0; i<tilesize.x*tilesize.y; ++i) {
        size_t imgx=i%tilesize.x;
        size_t imgy=i/tilesize.x;
        sf::Color tempcolor=img.getPixel(imgx,imgy);
        size_t lookup1=lookup(tempcolor, palette_color1, colors_found);
        if (lookup1==(size_t)-1) {
            if (colors_found>=maxcolor) {
                sf::Texture temptex;
                temptex.loadFromImage(img);
                sf::Sprite tempsprite;
                tempsprite.setTexture(temptex);
                tempsprite.setScale({50,50});
                throw "too many colors";
            }
            palette_color1[colors_found]=tempcolor;
            arr[imgx][imgy]=colors_found;
            std::pair<size_t,size_t> temppair=find_closest_color(tempcolor);
            if (color_difference(tempcolor.toInteger(),palette[temppair.first][temppair.second])<mincolordiff) {
                colors[colors_found]=temppair;
                if (color_difference(tempcolor.toInteger(),palette[temppair.first][temppair.second])>0) printf("difference of %I64d",color_difference(tempcolor.toInteger(),palette[temppair.first][temppair.second]));
            }
            else {
                color_initialized=false;
                printf("%x\n",tempcolor.toInteger());
                printf("closest %d %d\n",temppair.first,temppair.second);
                printf("difference %I64d \n",color_difference(tempcolor.toInteger(),palette[temppair.first][temppair.second]));
                std::cin.get();
            }
            ++colors_found;
        }
        else arr[imgx][imgy]=lookup1;
    }
}
void Sprite::print() {
    for (size_t i=0; i<8; ++i) {
        for (size_t j=0; j<8; ++j)
            printf("%d ",arr[j][i]);
        printf("\n");
    }
}
