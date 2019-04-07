//TODO choose palette
//TODO ImgToSprite equality function for printmap


#include <iostream>
#include<fstream>
#include<string>
#include<SFML/Graphics.hpp>
#include<unordered_map>
#include<array>
#include<iomanip>
#include <algorithm>
constexpr std::size_t maxcolor=4;
constexpr int spritemode=1;//0 no size opt
                            //1 doesnt put duplicates in vector
struct TooManyColors{};
struct TooManyColorsSpriteEqual{};
const sf::Vector2u tilesize={8,8};
sf::Int64 mincolordiff=0;

sf::RenderWindow window(sf::VideoMode(768, 672), "Tile Detector");
constexpr unsigned char nes_colors[10][4]={
    {0x0D ,0x30 ,0x21 ,0x12},
    {0x0D ,0x30 ,0x29 ,0x1A},
    {0x0D ,0x30 ,0x24 ,0x14},
    {0x0D ,0x30 ,0x2A ,0x12},
    {0x0D ,0x30 ,0x2B ,0x15},
    {0x0D ,0x30 ,0x22 ,0x2B},
    {0x0D ,0x30 ,0x00 ,0x16},
    {0x0D ,0x30 ,0x05 ,0x13},
    {0x0D ,0x30 ,0x16 ,0x12},
    {0x0D ,0x30 ,0x27 ,0x16}
};
/*sf::Uint32 palette[4][16]= { //RGB PALLETE
	{0x6d6d6dff,0x2492ff,0xdbff,0x6d49dbff,0x92006dff,0xb6006dff,0xb62400ff,0x924900ff,0x6d4900ff,0x244900ff,0x6d24ff,0x9200ff,0x4949ff,0xff,0xff,0xff},
	{0xb6b6b6ff,0x6ddbff,0x49ffff,0x9200ffff,0xb600ffff,0xff0092ff,0xff0000ff,0xdb6d00ff,0x926d00ff,0x249200ff,0x9200ff,0xb66dff,0x9292ff,0x242424ff,0xff,0xff},
	{0xffffffff,0x6db6ffff,0x9292ffff,0xdb6dffff,0xff00ffff,0xff6dffff,0xff9200ff,0xffb600ff,0xdbdb00ff,0x6ddb00ff,0xff00ff,0x49ffdbff,0xffffff,0x494949ff,0xff,0xff},
	{0xffffffff,0xb6dbffff,0xdbb6ffff,0xffb6ffff,0xff92ffff,0xffb6b6ff,0xffdb92ff,0xffff49ff,0xffff6dff,0xb6ff49ff,0x92ff6dff,0x49ffdbff,0x92dbffff,0x929292ff,0xff,0xff}
};*/


sf::Uint32 palette[4][16]= { //http://www.thealmightyguru.com/Games/Hacking/Wiki/index.php/NES_Palette  YPbPr palette
    {0x7C7C7CFF ,0x0000FCFF ,0x0000BCFF ,0x4428BCFF ,0x940084FF ,0xA80020FF ,0xA81000FF ,0x881400FF ,0x503000FF ,0x007800FF ,0x006800FF ,0x005800FF ,0x004058FF ,0x000000FF ,0x000000FF ,0x000000FF},
    {0xBCBCBCFF ,0x0078F8FF ,0x0058F8FF ,0x6844FCFF ,0xD800CCFF ,0xE40058FF ,0xF83800FF ,0xE45C10FF ,0xAC7C00FF ,0x00B800FF ,0x00A800FF ,0x00A844FF ,0x008888FF ,0x000000ff ,0x000000FF ,0x000000FF},
    {0xF8F8F8FF ,0x3CBCFCFF ,0x6888FCFF ,0x9878F8FF ,0xF878F8FF ,0xF85898FF ,0xF87858FF ,0xFCA044FF ,0xF8B800FF ,0xB8F818FF ,0x58D854FF ,0x58F898FF ,0x00E8D8FF ,0x787878FF ,0x000000FF ,0x000000FF},
    {0xFCFCFCFF ,0xA4E4FCFF ,0xB8B8F8FF ,0xD8B8F8FF ,0xF8B8F8FF ,0xF8A4C0FF ,0xF0D0B0FF ,0xFCE0A8FF ,0xF8D878FF ,0xD8F878FF ,0xB8F8B8FF ,0xB8F8D8FF ,0x00FCFCFF ,0xF8D8F8FF ,0x000000FF ,0x000000FF}
};

void palette_loader(const std::string& path) {
    std::ifstream hex_pal(path.c_str(),std::ios::in| std::ios::binary);
    std::size_t counter=0;
    sf::Uint8 r;
    sf::Uint8 g;
    sf::Uint8 b; //can't use << operators for binary input because it interprets some chars as whitespaces
    while(hex_pal.read(reinterpret_cast<char*>(&r), 1)&&hex_pal.read(reinterpret_cast<char*>(&g), 1)&&hex_pal.read(reinterpret_cast<char*>(&b), 1)) {
        sf::Uint32 final_color=0;
        final_color+=r<<24;
        final_color+=g<<16;
        final_color+=b<<8;
        final_color+=0xff;
        palette[counter/16][counter%16]=final_color;
        ++counter;
        if (counter>=16*4) break;
    }
    if (counter<64) throw "Palette is too small";
    FILE * palette_temp=fopen("Palette Hex.txt","w");
    fprintf(palette_temp,"\n\nsf::Uint32 palette[4][16]= {\n");
    for (std::size_t i=0; i<4; ++i) {
        fprintf(palette_temp,"\t{");
        for (std::size_t j=0; j<16;++j) {
            fprintf(palette_temp,"0x%08x",palette[i][j]);
            if (j!=15) fprintf(palette_temp,",");
        }
        fprintf(palette_temp,"}\n");
    }
    fprintf(palette_temp,"};\n\n");
    fclose(palette_temp);
    hex_pal.close();
}

sf::Int64 color_difference(sf::Uint32 cr1, sf::Uint32 cr2) {
    sf::Uint8 a=cr1;
    cr1>>=8;
    sf::Uint8 b=cr1;
    cr1>>=8;
    sf::Uint8 g=cr1;
    cr1>>=8;
    sf::Uint8 r=cr1;

    sf::Uint8 a2=cr2;
    cr2>>=8;
    sf::Uint8 b2=cr2;
    cr2>>=8;
    sf::Uint8 g2=cr2;
    cr2>>=8;
    sf::Uint8 r2=cr2;

    sf::Int64 colordiff=(a2-a)*(a2-a)+(b2-b)*(b2-b)+(g2-g)*(g2-g)+(r2-r)*(r2-r);
    return colordiff;
}

sf::Int64 color_difference(sf::Color cr1, sf::Color cr2) {
    return color_difference(cr1.toInteger(),cr2.toInteger());
}

std::pair<std::size_t,std::size_t> find_closest_color(sf::Color cr1, bool non_equal=false) {
    sf::Int64 mincolor=255*255+255*255+255*255+255*255;
    std::pair<std::size_t,std::size_t> result(0,0);
    for (std::size_t i=0; i<4;++i) {
        for (std::size_t j=0; j<14; ++j) {
            sf::Int64 colordiff=color_difference(palette[i][j],cr1.toInteger());
            if (colordiff<mincolor&&!(non_equal&&colordiff==0)) {
                mincolor=colordiff;
                result.first=i;
                result.second=j;
            }
        }
    }
    return result;
}

class Sprite {
public:
    Sprite() {
        for (std::size_t i=0; i<8; ++i)
            for (std::size_t j=0; j<8; ++j)
                arr[i][j]=0;
    }
    std::size_t lookup(sf::Color color, sf::Color* palette_color, std::size_t colors_found) const{
        for (std::size_t i=0; i<colors_found; ++i)
            if (palette_color[i]==color) return i;
        return -1;
    }
    Sprite(const sf::Image& img) {
        color_initialized=true;
        sf::Color palette_color1[maxcolor];//={0,0,0,0};
        std::size_t colors_found=1; //first color should always be black/transparent which is 0x0F= {0,15}
        palette_color1[0]=sf::Color(0x000000FF); //or palette[0][0xf];
        colors[0]=colors[1]=colors[2]=colors[3]={0,0xD};
        for (std::size_t i=0; i<tilesize.x*tilesize.y; ++i) {
            std::size_t imgx=i%tilesize.x;
            std::size_t imgy=i/tilesize.x;
            sf::Color tempcolor=img.getPixel(imgx,imgy);
            std::size_t lookup1=lookup(tempcolor, palette_color1, colors_found);
            if (lookup1==(std::size_t)-1) {
                if (colors_found>=maxcolor) {
                    sf::Texture temptex;
                    temptex.loadFromImage(img);
                    sf::Sprite tempsprite;
                    tempsprite.setTexture(temptex);
                    tempsprite.setScale({50,50});
                    throw TooManyColors();
                }
                palette_color1[colors_found]=tempcolor;
                arr[imgx][imgy]=colors_found;
                std::pair<std::size_t,std::size_t> temppair=find_closest_color(tempcolor);
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
        total_colors=colors_found;
    }
    void print() {
        for (std::size_t i=0; i<8; ++i) {
            for (std::size_t j=0; j<8; ++j)
                printf("%d ",arr[j][i]);
            printf("\n");
        }
    }
    bool color_initialized=false;
    unsigned char arr[8][8];
    unsigned char first_color_occ[maxcolor];
    std::size_t total_colors;
    std::pair<std::size_t,std::size_t> colors[maxcolor];
};
template <typename INT>
constexpr INT rol(INT val, unsigned int moves) {
    static_assert(std::is_unsigned<INT>::value,
                  "Rotate Left only makes sense for unsigned types");
    return (val << (moves%(sizeof(INT)*CHAR_BIT))) | (val >> (sizeof(INT)*CHAR_BIT-(moves%(sizeof(INT)*CHAR_BIT))));
}

namespace std
{
    template<>
    struct hash<sf::Image> {
        std::size_t operator()(const sf::Image& t) const noexcept
        {
            sf::Vector2u imagesize=t.getSize();
            unsigned long long temphash=0;
            for (std::size_t x=0; x<imagesize.x; ++x)
                for (std::size_t y=0; y<imagesize.y; ++y)
                    temphash+=t.getPixel(x,y).toInteger();
            return std::hash<unsigned long long>()(temphash);
        }
    };

    template<>
    struct hash<Sprite> {
        std::size_t operator()(const Sprite& t) const noexcept
        {
            struct funcholder {
                static std::size_t lookup(unsigned char color, unsigned char* palette_color, std::size_t colors_found) {
                    for (std::size_t i=0; i<colors_found; ++i)
                        if (palette_color[i]==color) return i;
                    return (std::size_t)-1;
                }
            };
            std::size_t colors_found=0;
            unsigned char palette_color1[maxcolor];
            unsigned long long temphash=0;
            unsigned long long rotations=0;
            for (std::size_t x=0; x<tilesize.x; ++x) {
                for (std::size_t y=0; y<tilesize.y; ++y) {
                    std::size_t lookup1=funcholder::lookup(t.arr[x][y],palette_color1,colors_found);
                    if (lookup1==(std::size_t)-1) {
                        if (colors_found>=maxcolor) throw "Too many colors in sprite hash"; //ERROR
                        palette_color1[colors_found]=t.arr[x][y];
                        ++colors_found;
                    }
                    temphash+=rol<unsigned long long>(lookup1,rotations++); //lookup1 intentional
                }
            }
            return std::hash<unsigned long long>()(temphash);
        }
    };
}
class ImageEqual
{
public:
    bool operator()(const sf::Image& img1, const sf::Image& img2) const
    {
        if (img1.getSize()!=img2.getSize())
            return false;
        for (std::size_t x=0; x<img1.getSize().x; ++x)
            for (std::size_t y=0; y<img1.getSize().y; ++y)
                if (img1.getPixel(x,y)!=img2.getPixel(x,y))
                    return false;
        return true;
    }

};

struct SpriteEqual {
    std::size_t lookup(unsigned char color, unsigned char* palette_color, std::size_t colors_found) const{
        for (std::size_t i=0; i<colors_found; ++i)
            if (palette_color[i]==color) return i;
        return (std::size_t)-1;
    }
    bool operator()(const Sprite& img1, const Sprite& img2) const{
        std::size_t colors_found=0;
        unsigned char palette_color1[maxcolor];//={0,0,0,0};
        unsigned char palette_color2[maxcolor];//={0,0,0,0};
        for (std::size_t x=0; x<tilesize.x; ++x) {
            for (std::size_t y=0; y<tilesize.y; ++y) {
                std::size_t lookup1=lookup(img1.arr[x][y],palette_color1,colors_found);
                std::size_t lookup2=lookup(img2.arr[x][y],palette_color2,colors_found);
                if (lookup1!=lookup2) return false;
                if (lookup1==(std::size_t)-1) {
                    if (colors_found>=maxcolor) throw "Too many colors in sprite equal"; //ERROR
                    palette_color1[colors_found]=img1.arr[x][y];
                    palette_color2[colors_found]=img2.arr[x][y];
                    ++colors_found;
                }
            }
        }
        return true;
    }
};
struct ImgToSpriteEqual {
    std::size_t lookup(unsigned char color, unsigned char* palette_color, std::size_t colors_found) const{
        for (std::size_t i=0; i<colors_found; ++i)
            if (palette_color[i]==color) return i;
        return (std::size_t)-1;
    }
    bool operator()(const Sprite& img1, const Sprite& img2) const{
        if (img1.total_colors!=img2.total_colors) return false;
        for (std::size_t i=0; i<img1.total_colors; ++i)
            if (img1.colors[i]!=img2.colors[i]) return false;
        std::size_t colors_found=0;
        unsigned char palette_color1[maxcolor];//={0,0,0,0};
        unsigned char palette_color2[maxcolor];//={0,0,0,0};
        for (std::size_t x=0; x<tilesize.x; ++x) {
            for (std::size_t y=0; y<tilesize.y; ++y) {
                std::size_t lookup1=lookup(img1.arr[x][y],palette_color1,colors_found);
                std::size_t lookup2=lookup(img2.arr[x][y],palette_color2,colors_found);
                if (lookup1!=lookup2) return false;
                if (lookup1==(std::size_t)-1) {
                    if (colors_found>=maxcolor) throw "Too many colors in sprite equal"; //ERROR
                    palette_color1[colors_found]=img1.arr[x][y];
                    palette_color2[colors_found]=img2.arr[x][y];
                    ++colors_found;
                }
            }
        }
        return true;
    }
};
struct ImgToSpriteHash {
    std::size_t operator()(const Sprite& t) const noexcept
    {
        struct funcholder {
            static std::size_t lookup(unsigned char color, unsigned char* palette_color, std::size_t colors_found) {
                for (std::size_t i=0; i<colors_found; ++i)
                    if (palette_color[i]==color) return i;
                return (std::size_t)-1;
            }
        };
        std::size_t colors_found=0;
        unsigned char palette_color1[maxcolor];
        unsigned long long temphash=0;
        temphash+=std::hash<unsigned long long>()(t.total_colors);
        for (std::size_t i=0; i<t.total_colors; ++i) {
            temphash+=std::hash<std::size_t>()(t.colors[i].first);
            temphash+=std::hash<std::size_t>()(t.colors[i].second);
        }
        unsigned long long rotations=0;
        for (std::size_t x=0; x<tilesize.x; ++x) {
            for (std::size_t y=0; y<tilesize.y; ++y) {
                std::size_t lookup1=funcholder::lookup(t.arr[x][y],palette_color1,colors_found);
                if (lookup1==(std::size_t)-1) {
                    if (colors_found>=maxcolor) throw "Too many colors in sprite hash"; //ERROR
                    palette_color1[colors_found]=t.arr[x][y];
                    ++colors_found;
                }
                temphash+=rol<unsigned long long>(lookup1,rotations++);
            }
        }
        return std::hash<unsigned long long>()(temphash);
    }
};
std::vector<Sprite> spritevector;
std::unordered_map<Sprite,std::size_t,std::hash<Sprite>,SpriteEqual> spritemap;
bool load(const std::string& tilefile){ //TODO remake with c++
    std::ifstream spritefile(tilefile.c_str(),std::ios::in);
    std::size_t sprite_count=spritevector.size();
    while (!spritefile.eof()) {
        //printf("%d\n",spritevector.size());
        std::size_t characters=0;
        Sprite newsprite;
        for (characters=0; characters<tilesize.y*2; ++characters) {
            sf::Uint32 hextmp;
            if (!(spritefile>>std::hex>>hextmp)) {
                characters = tilesize.y*2;
                break;
            }
            sf::Uint8 hex=hextmp;
            if (characters<tilesize.y) {
                for (std::size_t i=0; i<tilesize.x; ++i)
                newsprite.arr[tilesize.x-i-1][characters]=(hex >> i) & 1U;
            }
            else {
                for (std::size_t i=0; i<tilesize.x; ++i)
                newsprite.arr[tilesize.x-i-1][characters-8]+=((hex >> i) & 1U)<<1;
            }
        }
        if (characters>=tilesize.y*2) {
            bool colors_found_arr[maxcolor];
            colors_found_arr[0]=true; //black is always first
            newsprite.first_color_occ[0]=0;
            for (std::size_t i=1; i<maxcolor;++i)
                colors_found_arr[i]=false;
            std::size_t colors_found=1;
            for (std::size_t i=0; i<tilesize.x*tilesize.y; ++i) {
                std::size_t imgx=i%tilesize.x;
                std::size_t imgy=i/tilesize.x;
                if (!colors_found_arr[newsprite.arr[imgx][imgy]]) {
                    newsprite.first_color_occ[colors_found]=newsprite.arr[imgx][imgy];
                    colors_found_arr[newsprite.arr[imgx][imgy]]=true;
                    colors_found++; //out of bounds excp if sprite file is ill-formed
                }
            }
            newsprite.total_colors=colors_found;
            if (spritemode==1) {
                if (spritemap.find(newsprite)==spritemap.end()) {
                    spritevector.push_back(newsprite);
                    spritemap[newsprite]=sprite_count;
                    sprite_count++;
                }
            }
            else if (spritemode==0) {
                spritevector.push_back(newsprite);
                if (spritemap.find(newsprite)==spritemap.end()) {
                    spritemap[newsprite]=sprite_count;
                }
                sprite_count++;
            }
        }
    }
    spritefile.close();
    return true;
}

std::size_t create_sprite(FILE* output_sprite, const Sprite& _newsprite) {
    Sprite newsprite=_newsprite;
    bool colors_found_arr[maxcolor];
    colors_found_arr[0]=true; //black is always first
    newsprite.first_color_occ[0]=0;
    for (std::size_t i=1; i<maxcolor;++i)
        colors_found_arr[i]=false;
    std::size_t colors_found=1;
    for (std::size_t i=0; i<tilesize.x*tilesize.y; ++i) {
        std::size_t imgx=i%tilesize.x;
        std::size_t imgy=i/tilesize.x;
        if (!colors_found_arr[newsprite.arr[imgx][imgy]]) {
            newsprite.first_color_occ[colors_found]=newsprite.arr[imgx][imgy];
            colors_found_arr[newsprite.arr[imgx][imgy]]=true;
            colors_found++; //out of bounds excp if sprite file is ill-formed
        }
    }
    newsprite.total_colors=colors_found;
    for (std::size_t y=0; y<tilesize.y;++y) {
        sf::Uint32 hex=0;
        for (std::size_t x=0; x<tilesize.x;++x) {
            hex+=(newsprite.arr[tilesize.x-x-1][y]%2)<<x;
        }
        hex&=0xff;
        fprintf(output_sprite,"%02x ",hex);
    }

    for (std::size_t y=0; y<tilesize.y;++y) {
        sf::Uint8 hex=0;
        for (std::size_t x=0; x<tilesize.x;++x) {
            hex+=(newsprite.arr[tilesize.x-x-1][y]/2)<<x;
        }
        hex&=0xff;
        fprintf(output_sprite,"%02x ",hex);
    }
    std::size_t vectorsize=spritevector.size();
    spritevector.push_back(newsprite);
    spritemap[newsprite]=vectorsize;
    return vectorsize;
}
bool allblack(const Sprite& imgtosprite) {
    /*const float color_exp=log(3)/log(2);
    for (std::size_t i=0; i<imgtosprite.total_colors; ++i) {
        unsigned int colf=palette[imgtosprite.colors[i].first][imgtosprite.colors[i].second];
        const unsigned char af=colf&0xff;
        colf>>=8;
        const float bf=(colf&0xff)*af/255.;
        colf>>=8;
        const float gf=(colf&0xff)*af/255.;
        colf>>=8;
        const float rf=(colf&0xff)*af/255.;
        const float black_diff=pow(rf,color_exp)+pow(gf,color_exp)+pow(bf,color_exp);
        if (black_diff>mincolordiff) return false;
    }*/


    for (std::size_t i=0; i<imgtosprite.total_colors; ++i) {
        unsigned int colf=palette[imgtosprite.colors[i].first][imgtosprite.colors[i].second];
        if (color_difference(colf,sf::Color::Black.toInteger())>0) return false;
    }
    return true;
}


//previous print stuff
std::vector<std::vector<std::pair<std::size_t, std::size_t> > > printvector;
std::unordered_map<Sprite,std::size_t,ImgToSpriteHash,ImgToSpriteEqual> printmap;
void print_sprite_tilecont(std::ofstream& tile_output, const Sprite& imgtosprite, const std::size_t& x, const std::size_t& y) {
    if (!allblack(imgtosprite)) {
        if ( printmap.find(imgtosprite)==printmap.end() ) {
            printvector.push_back(std::vector<std::pair<std::size_t, std::size_t> >{std::make_pair(x,y)} );
            if (printvector.size()==0) throw "push_back failed";
            printmap[imgtosprite]=printvector.size()-1;
        }
        else {
            printvector[printmap[imgtosprite]].push_back(std::make_pair(x,y));
        }
    }
}
void print_last_sprite_tilecont(std::ofstream& tile_output) {
    for (const auto& i: printmap) {
        for (const auto& j: printvector[i.second]) {
            tile_output<<"tilecont->at("<<j.first<<","<<j.second<<")=";
        }
        tile_output<<"tiletype("<<spritemap[i.first]<<",";
        for (std::size_t j=0; j<maxcolor; ++j) {
            tile_output<<"0x";
            tile_output<<i.first.colors[j].first;
            tile_output<<std::hex<<i.first.colors[j].second<<std::dec;
            if (j!=maxcolor-1) tile_output<<",";
            else tile_output<<");\n";
        }
    }
}
int main()
{
    palette_loader("YPbPr.pal");
    mincolordiff=255*255+255*255+255*255+255*255; //finds the highest difference between 2 colors and halves it, if when detecting a tile the color is farther than this value from any color it throws an exception
    for (std::size_t i=0; i<4;++i) {
        for (std::size_t j=0; j<14; ++j) {
            std::pair<std::size_t,std::size_t> closestcolor=find_closest_color(sf::Color(palette[i][j]),true);
            sf::Int64 colordiff=color_difference(palette[i][j],palette[closestcolor.first][closestcolor.second]);
            if (colordiff!=0) {
                if (colordiff<6) {
                    colordiff=0;
                }
                else colordiff=colordiff/2-3;
                if (colordiff<mincolordiff) mincolordiff=colordiff;
                if (colordiff==21) {
                    printf("%d,%d vs %d,%d",i,j,closestcolor.first,closestcolor.second);
                }
            }
        }
    }
    printf("colordiff=%I64d\n",mincolordiff);
    std::string spritetxt="sprites.txt";
    load(spritetxt);
    load(spritetxt+"updated");
    printf("spritemap size%d, spritevector size%d\n",spritemap.size(),spritevector.size());
    sf::Image blink;
    std::string imagesrc="statistic-pieces.png";
    blink.loadFromFile(imagesrc);
    std::size_t width=blink.getSize().x/tilesize.x;
    std::size_t height=blink.getSize().y/tilesize.y;
    std::unordered_map<sf::Image,std::size_t,std::hash<sf::Image>,ImageEqual> imagemap;
    std::size_t tile=0;

    FILE*output_sprite=fopen(std::string(spritetxt+"updated").c_str(),"a");
    std::ofstream tile_output(imagesrc+".tilecont");

    for (std::size_t i=0; i<width*height;++i) {
        sf::Image tempimg;
        sf::Sprite tempsprite;
        sf::Texture temptex;
        tempimg.create(tilesize.x,tilesize.y);
        temptex.create(tilesize.x,tilesize.y);
        std::size_t imgx=(i%width)*tilesize.x;
        std::size_t imgy=(i/width)*tilesize.y;
        sf::IntRect section(imgx,imgy,tilesize.x,tilesize.y);
        tempimg.copy(blink,0,0,section);
        temptex.loadFromImage(tempimg);
        tempsprite.setTexture(temptex,true);
        tempsprite.setPosition(imgx*3,imgy*3);
        tempsprite.setScale(3,3);
        window.clear();
        window.draw(tempsprite);
        window.display();
        //sf::sleep(sf::milliseconds(1));
        if (imagemap.find(tempimg)==imagemap.end()) {
            imagemap[tempimg]=tile++;
        }
        Sprite imgtosprite(tempimg);
        if (spritemap.find(imgtosprite)==spritemap.end()) {
            printf("Creating new sprite\n");
            imgtosprite.print();
            create_sprite(output_sprite,imgtosprite); //TODO
        }
        else {
            //riordinare i colori di sprite
            const Sprite& correct_order=spritevector[spritemap[imgtosprite]];
            //imgtosprite always has 0 1 2 3 order
            std::pair<std::size_t,std::size_t> newcolors[maxcolor];
            for (std::size_t j=0; j<correct_order.total_colors;++j) {
                newcolors[correct_order.first_color_occ[j]]=imgtosprite.colors[j];
            }
            for (std::size_t j=0; j<maxcolor;++j) {
                imgtosprite.colors[j]=newcolors[j];
            }
        }
        print_sprite_tilecont(tile_output,imgtosprite,(i%width),(i/width));
        std::cout<<"I= "<<i<<"imagemap="<<imagemap[tempimg]<<", spritemap="<< spritemap[imgtosprite]<<"\n";
    }
    print_last_sprite_tilecont(tile_output);

    tile_output.close();
    fclose(output_sprite);
}
