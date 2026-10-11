#include <SFML/Graphics.hpp>
#include <iostream>
#include <stack>
#include <fstream>
#include <bitset>

using namespace std;

void printBinary(uint64_t value) {
    cout << "values:   "<< bitset<64>(value) << endl; // Print the binary representation
}

int main() {
    sf::ContextSettings settings;
    settings.majorVersion = 2;
    settings.minorVersion = 1;

    sf::RenderWindow window(sf::VideoMode(640, 320), "CHIP-8-EMU!",sf::Style::Default,settings);

    uint8_t memory[4096]{0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
                         0x20, 0x60, 0x20, 0x20, 0x70, // 1
                         0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
                         0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
                         0x90, 0x90, 0xF0, 0x10, 0x10, // 4
                         0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
                         0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
                         0xF0, 0x10, 0x20, 0x40, 0x40, // 7
                         0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
                         0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
                         0xF0, 0x90, 0xF0, 0x90, 0x90, // A
                         0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
                         0xF0, 0x80, 0x80, 0x80, 0xF0, // C
                         0xE0, 0x90, 0x90, 0x90, 0xE0, // D
                         0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
                         0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
    uint8_t registers[16]{};




    stack<char16_t> stack;
    uint8_t soundTimer{};
    uint8_t delayTimer{};

    uint16_t IR{0};
    uint16_t PC{0x200};
    uint16_t instruct{};

    uint64_t display[32]{};


    sf::RectangleShape pixel{sf::Vector2f(-1, 0)};
    pixel.setSize(sf::Vector2f(10.f, 10.f));


    ifstream fileReader;
    fileReader.open("corax.ch8", ios_base::binary|ios::ate);

    uint8_t* readMem = &memory[0x200];

    streampos size = fileReader.tellg();
    fileReader.seekg(0,ios::beg);
    fileReader.read(reinterpret_cast<char *>(readMem), size);
    fileReader.close();


    while (window.isOpen())
    {
        sf::Event event{};
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        //fetch
        instruct = ((uint16_t)memory[PC]<<8)|memory[PC+1];
        PC+=2;
        //decode
        switch (instruct & 0xF000) {
            case(0x0000):{
                switch(instruct) {
                    case (0x00E0): {  //clear screen  WORKS
                        for (uint64_t &i: display) {
                            i = 0;
                        }
                        pixel.setPosition(-1, 0);
                        break;
                    }
                    case(0x00EE): {  //return from subroutine
                        PC = stack.top();
                        stack.pop();
                        break;
                    }

                }
                break;
            }

            case(0x1000):{ //jump  WORKS
                PC = instruct&0x0FFF;
                break;
            }
            case(0x2000):{  // Subroutines
                stack.push(PC);
                PC = instruct&0x0FFF;
                break;
            }

            case(0x3000):{  //skip if vx is equal to 0x00FF
                if (registers[(instruct&0x0F00) >>8] == (instruct&0x00FF)){
                    PC+=2;
                }
                break;
            }

            case(0x4000):{  //skip if vx is not equal to 0x00FF
                if (registers[(instruct&0x0F00) >>8] != (instruct&0x00FF)){
                    PC+=2;
                }
                break;
            }

            case(0x5000):{  //skip if vx is equal to vy
                if (registers[(instruct&0x0F00) >>8] == registers[(instruct&0x00F0) >>4]){
                    PC+=2;
                }
                break;
            }

            case(0x6000):{ //set reg   WORKS
                registers[(instruct&0x0F00) >>8] = instruct&0x00FF;
                break;
            }
            case(0x7000):{  //add reg   WORKS
                registers[(instruct&0x0F00) >>8]+=instruct&0x00FF;
                break;
            }

            case(0x8000):{  //logical instructions
                switch(instruct&0x000F){
                    case(0x0000):{  //vx =vy
                        registers[(instruct&0x0F00) >>8]=registers[(instruct&0x00F0) >>4];
                        break;
                    }
                    case(0x0001):{  //OR
                        registers[(instruct&0x0F00) >>8]|=registers[(instruct&0x00F0) >>4];
                        break;
                    }
                    case(0x0002):{  //AND
                        registers[(instruct&0x0F00) >>8]&=registers[(instruct&0x00F0) >>4];
                        break;
                    }
                    case(0x0003):{  //XOR
                        registers[(instruct&0x0F00) >>8]^=registers[(instruct&0x00F0) >>4];
                        break;
                    }
                    case(0x0004):{  //ADD with carry flag
                        int carryTest = registers[(instruct&0x0F00) >>8]+registers[(instruct&0x00F0) >>4];
                        registers[(instruct&0x0F00) >>8]+=registers[(instruct&0x00F0) >>4];
                        if (carryTest>255){registers[0xF]=1;}
                        else{registers[0xF]=0;}
                        break;
                    }
                    case(0x0005):{  //Subtract x= x-y
                        bool subTest = (registers[(instruct&0x0F00) >>8] > registers[(instruct&0x00F0) >>4]);
                        registers[(instruct&0x0F00) >>8]-=registers[(instruct&0x00F0) >>4];
                        if (subTest){{registers[0xF]=1;}}
                        else{registers[0xF]=0;}
                        break;
                    }
                    case(0x0006):{  //Ambiguous instruction!  shift x right 1
                        if (registers[(instruct&0x0F00) >>8]&1){registers[0xF]=1;}
                        else{registers[0xF]=0;}
                        registers[(instruct&0x0F00) >>8]=registers[(instruct&0x0F00) >>8]>>1;
                        break;
                    }
                    case(0x0007):{  //Subtract x= y-x
                        if (registers[(instruct&0x0F00) >>8] < registers[(instruct&0x00F0) >>4]){registers[0xF]=1;}
                        else{registers[0xF]=0;}
                        registers[(instruct&0x0F00) >>8]=registers[(instruct&0x00F0) >>4]-registers[(instruct&0x0F00) >>8];
                        break;
                    }
                    case(0x000E):{  //Ambiguous instruction!  shift x left 1
                        if ((registers[(instruct&0x0F00) >>8]&0x8000)==0x8000){registers[0xF]=1;}
                        else{registers[0xF]=0;}
                        registers[(instruct&0x0F00) >>8]=registers[(instruct&0x0F00) >>8]<<1;
                        break;
                    }

                }

                break;
            }


            case(0x9000):{  //skip if vx is not equal to vy
                if (registers[(instruct&0x0F00) >>8] != registers[(instruct&0x00F0) >>4]){
                    PC+=2;
                }
                break;
            }

            case(0xA000):{ //set IR   WORKS
                IR = instruct&0x0FFF;
                break;

            }

            case(0xB000):{ //Ambiguous instruction! Jump with offset TODO
                break;

            }

            case(0xC000):{ // Random TODO
                break;

            }

            case(0xD000): { //draw  WORKS
                    //DXYN
                    // get x from vx
                    uint8_t x = registers[(instruct&0x0F00)>>8]&63;
                    //get y from vy
                    uint8_t y = registers[(instruct&0x00F0)>>4]&31;
                    registers[0xF]=0;
                    uint8_t height = instruct&0x000F;
                    uint8_t spriteData;
                    cout << (unsigned int) y << endl;

                    for (int i=0;i<height;i++){
                        spriteData=memory[IR+i];
                        if (x<57) {
                            if ((display[y]) ==
                                (display[y] ^ ((uint64_t) spriteData << (56 - x)))) { registers[0xF] = 1; }
                            display[y] ^= ((uint64_t) spriteData << (56 - x));  //bit xor
                        }
                        else{
                            spriteData>>=(x-56);
                            if ((display[y]) ==
                                (display[y] ^ ((uint64_t) spriteData))) { registers[0xF] = 1; }
                            display[y] ^= ((uint64_t) spriteData);  //bit xor
                        }
                        y+=1;
                    }

            }

            case(0xE000):{ //Skip if key TODO
                break;

            }
            case(0xF000):{
                switch(instruct&0x00FF){ // fix masking
                    case(0x0007):{  //Timers TODO FX07
                        break;
                    }
                    case(0x0015):{ //Timers TODO FX15
                        break;
                    }
                    case(0x0018):{ //Timers TODO FX18
                        break;
                    }
                    case(0x001E):{ //Add to index TODO FX1E
                        break;
                    }
                    case(0x000A):{ //Get key Blocking TODO FX0A
                        break;
                    }
                    case(0x0029):{ //set index to Font char in vx FX29
                        IR=(registers[(instruct&0x0F00)>>8]&0x0F)*5;
                        break;
                    }

                    case(0x0033):{ // Binary-coded decimal conversion TODO FX33
                        break;
                    }

                    case(0x0055):{ // Binary-coded decimal conversion TODO FX55
                        break;
                    }
                    case(0x0065):{ // Binary-coded decimal conversion TODO FX65
                        break;
                    }

                }

                break;
            }

        }
        window.clear(sf::Color::Black);

        for (int y=0;y<32;y++){  //main draw loop
            for(int x=0;x<64;x++){
                if ((display[y]>>(63-x)) & 1ull){
                    pixel.setPosition(x*10, y*10);
                    window.draw(pixel);
                }
            }
        }

        window.display();


    }

    return 0;
}


