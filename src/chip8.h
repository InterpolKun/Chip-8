

unsigned char chip8_font[80] = {
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
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

unsigned short opcode;
unsigned short I;
unsigned short pc;
unsigned short sp; //Stack pointer

unsigned char gfx[64*32];

unsigned char delay_timer;
unsigned char sound_timer;

unsigned short stack[16];
unsigned char memory[4096];
unsigned char V[16];
unsigned char key[16];

unsigned char drawFlag; // 1 true 0 false

void init_chip(){
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;

    for(int i = 0; i < 2048; ++i){
        gfx[i] = 0;
    }

    for(int i = 0; i < 16; ++i){
        stack[i] = 0;
    }

    for(int i = 0; i < 16; ++i){
        key[i] = V[i] = 0;
    }

    for(int i = 0; i < 4096; ++i){
        memory[i] = 0;
    }
    for(int i = 0; i < 80; ++i){
        memory[i] = chip8_font[i];
    }

    delay_timer = 0;
    sound_timer = 0;

    drawFlag = 1;

    srand(time(NULL));
}

void emulate(){
    opcode = memory[pc] << 8 | memory[pc + 1];

    switch(opcode & 0xF000)
    {
        case 0x0000:
            switch (opcode & 0x000F){
                case 0x0000:
                    for(int i = 0; i < 2048; ++i)
                        gfx[i] = 0x0;
                    drawFlag = 1;
                    pc += 2;
                break;

                case 0x000E:
                    --sp;
                    pc = stack[sp];
                    pc += 2;
                break;

                default:
                    printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
            }
        break;

        case 0x1000:
            pc = opcode & 0x0FFF;
        break;

        case 0x2000:
            stack[sp] = pc;
            ++sp;
            pc = opcode & 0x0FFF;
        break;

        case 0x3000:
            if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)){
                pc += 4;
            } else {
                pc += 2;
            }
        break;

        case 0x4000:
            if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)){
                pc += 4;
            } else {
                pc += 2;
            }
        break;

        case 0x5000:
            if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]){
                pc += 4;
            } else {
                pc += 2;
            }
        break;

        case 0x6000:
            V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            pc += 2;
        break;

        case 0x7000:
            V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            pc += 2;
        break;

        case 0x8000:
            switch(opcode & 0x000F){
                case 0x0000:
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                break;

                case 0x0001:
                    V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                break;

                case 0x0002:
                    V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                break;

                case 0x0003:
                    V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                break;

                case 0x0004:
                    if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x00F0) >> 8])){
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                break;

                case 0x0005:
                    if(V[(opcode & 0x0F00) >> 8] < V[(opcode & 0x00F0) >> 4]){
                        V[0xF] = 0;
                    } else {
                        V[0xF] = 1;
                    }
                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                break;

                case 0x0006:
                    V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
                    V[(opcode & 0x0F00) >> 8] >>= 1;
                    pc += 2;
                break;

                case 0x0007:
                    if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4]){
                        V[0xF] = 0;
                    } else {
                        V[0xF] = 1;
                    }
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                break;

                case 0x000E:
                    V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
                    V[(opcode & 0x0F00) >> 8] <<= 1;
                    pc += 2;
                break;

                default:
                    printf("Unknown opcode [0x8000]: 0x%X\n", opcode);
            }
            break;

            case 0x9000:
                if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]){
                    pc += 4;
                } else {
                    pc += 2;
                }
            break;

            case 0xA000:
                I = opcode & 0x0FFF;
                pc += 2;
            break;

            case 0xB000:
                pc = (opcode & 0x0FFF) + V[0];
            break;

            case 0xC000:
                V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
                pc += 2;
            break;

            case 0xD000:
            {
                    unsigned short x = V[(opcode & 0x0F00) >> 8];
                    unsigned short y = V[(opcode & 0x00F0) >> 4];
                    unsigned short height = opcode & 0x000F;
                    unsigned short pixel;

                    V[0xF] = 0;
                    for(int yline = 0; yline < height; yline++){
                        pixel = memory[I + yline];
                        for(int xline = 0; xline < 8; xline ++){
                            if((pixel & (0x80 >> xline))!= 0){
                                if(gfx[(x + xline + ((y + yline) * 64))] == 1){
                                    V[0xF] = 1;
                                }
                                gfx[x + xline + ((y + yline) * 64)] ^= 1;
                            }
                        }
                    }
                    drawFlag = 1;
                    pc += 2;
            }
            break;

            case 0xE000:
                switch (opcode & 0x00FF) {
                    case 0x009E:
                        if(key[V[(opcode & 0x0F00) >> 8]] != 0){
                            pc += 4;
                        } else {
                            pc += 2;
                        }
                    break;

                    case 0x00A1:
                        if(key[V[(opcode & 0x0F00) >> 8]] == 0){
                            pc += 4;
                        } else {
                            pc += 2;
                        }
                    break;

                    default:
                        printf("Unknown opcode [0xE000]: 0x%X\n", opcode);
                }
            break;

            case 0xF000:
                switch(opcode & 0x00FF){
                    case 0x0007:
                        V[(opcode & 0x0F00) >> 8] = delay_timer;
                        pc += 2;
                    break;

                    case 0x000A:
                    {
                        char keyPress = 0;

                        for(int i = 0; i < 16; i++){
                            if(key[i] != 0){
                                V[(opcode & 0x0F00) >> 8] = i;
                                keyPress = 1;
                            }
                        }
                        if(keyPress)
                            return;

                        pc += 2;
                    }
                    break;

                    case 0x0015:
                        delay_timer = V[(opcode & 0x0F00) >> 8];
                        pc += 2;
                    break;

                    case 0x0018:
                        sound_timer = V[(opcode & 0x0F00) >> 8];
                        pc += 2;
                    break;

                    case 0x001E:
                        if(I + V[(opcode & 0x0F00) >> 8] > 0xFFF){
                            V[0xF] = 1;
                        } else {
                            V[0xF] = 0;
                        }
                        I += V[(opcode & 0x0F00) >> 8];
                        pc += 2;
                    break;

                    case 0x0029:
                        I = V[(opcode & 0x0F00) >> 8] * 0x5;
                        pc += 2;
                    break;

                    case 0x0033:
                        memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
                        memory[I + 1] = (V[(opcode & 0x0F00) >> 8] /10) % 10;
                        memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) %10;
                        pc += 2;
                    break;

                    case 0x0055:
                        for(int i = 0; i < ((opcode & 0x0F00) >> 8); ++i){
                            memory[I + i] = V[i];
                        }
                        I += ((opcode & 0x0F00) >> 8) + 1;
                        pc += 2;
                    break;

                    case 0x0065:
                        for(int i = 0; i < ((opcode & 0x0F00) >> 8); ++i){
                            V[i] = memory[I + i];
                        }
                        I += ((opcode & 0x0F00) >> 8) + 1;
                        pc += 2;
                    break;

                    default:
                        printf("Unknown opcode [0xF000]: 0x%X\n", opcode);
                }
            break;

            default:
                printf("Unknown opcode: 0x%X\n", opcode);
    }
    if(delay_timer > 0){
        --delay_timer;
    }
    if(sound_timer > 0){
        if(sound_timer == 1)
            printf("BEEP!\n");
        --sound_timer;
    }

}

int loadRom(char * name){

    printf("Loading ROM: %s\n", name);

    FILE * rom = fopen(name, "rb");

    fseek(rom, 0, SEEK_END);
    long size = ftell(rom);
    rewind(rom);
    printf("ROM's size is %d\n", (int)size);
    char * buffer = (char*)malloc(sizeof(char) * size);
    if(buffer == NULL){
        fputs("Can't allocate memory\n", stderr);
        return 1;
    }
    printf("Memory allocated\n");
    size_t result = fread(buffer, 1, size, rom);
    if(result != size){
        fputs("Can't read\n", stderr);
        return 1;
    }

    init_chip();

    printf("ROM readed\n");

    if((4096-512) > size){
        for(int i = 0; i < size; i++){
            memory[i+512] = buffer[i];
        }
    }
    else{
        printf("ROM is too big. Max size is 3,5MB\n");
    }

    printf("ROM writed\n");

    fclose(rom);
    free(buffer);

}
