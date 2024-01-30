#include <iostream>
#include <chrono>
#include <Windows.h>
#include <vector>
#include <algorithm>


using namespace std;

int nScreenWidth = 120; // 120 columns
int nScreenHeight = 40; // 40 rows 

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;


float ffield_of_view = 3.14195 / 4.0;

float fDepth = 16.0f;

// Function to generate a random maze pattern
wstring GenerateRandomMaze(int width, int height) {
    wstring maze;

    // Generate boundaries
    for (int x = 0; x < width; x++) {
        maze += L'#';
    }
    maze += L'\n';

    for (int y = 1; y < height - 1; y++) {
        maze += L'#';
        for (int x = 1; x < width - 1; x++) {
            // Randomly decide if it's a wall or an empty space
            if (rand() % 3 == 0) {
                maze += L'#'; // Wall
            }
            else {
                maze += L'.'; // Empty space
            }
        }
        maze += L'#'; // Right boundary
        maze += L'\n';
    }

    // Generate boundaries
    for (int x = 0; x < width; x++) {
        maze += L'#';
    }

    return maze;
}





int main()
{
    // Creating Screen Buffer
    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    /*wstring map;

    map += L"################";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#...######.....#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#.......#####..#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"################";*/

    // Generate a random maze
    /*wstring map = GenerateRandomMaze(nMapWidth, nMapHeight);*/


    // Generate challenging maze
    wstring map;
    for (int y = 0; y < nMapHeight; y++) {
        for (int x = 0; x < nMapWidth; x++) {
            if (y == 0 || y == nMapHeight - 1 || x == 0 || x == nMapWidth - 1) {
                map += L'#'; // add walls to the boundaries
            }
            else if ((y % 2 == 0) && (x % 2 == 0)) {
                map += L'#'; // add walls at even coordinates
            }
            else {
                map += L'.'; // add open spaces
            }
        }
    }

    // Randomize open spaces
    vector<int> openSpaces;
    for (int i = 0; i < nMapHeight * nMapWidth; i++) {
        if (map[i] == L'.') {
            openSpaces.push_back(i);
        }
    }

    random_shuffle(openSpaces.begin(), openSpaces.end());

    for (int i = 0; i < openSpaces.size(); i++) {
        map[openSpaces[i]] = L'.';
    }

    // Intial Timepoints (later used for calculation of frames)
    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();
    




    // Main Game loop
    while (1)
    {
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();



        // Controls
        // Handle Counter-Clockwise Rotation
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
            fPlayerA -= (0.8f) * fElapsedTime;

        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
            fPlayerA += (0.8f) * fElapsedTime;

        if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
        {
            fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;

            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
            {
                fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }

        if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
        {
            fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;

            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
            {
                fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }





        for (int x = 0; x < nScreenWidth; x++)
        {
            // For each column, calculate the projected ray angle into the world space
            float fRay_Angle = (fPlayerA - ffield_of_view / 2.0f) + ((float)x / (float)nScreenWidth) * ffield_of_view;

            float fDistance_to_Wall = 0;
            bool hit_wall = false;
            bool boundary = false;




            float fEye_x = sinf(fRay_Angle); // Unit vector for ray in player space
            float fEye_Y = cosf(fRay_Angle);

            while (!hit_wall && fDistance_to_Wall < fDepth)
            {
                fDistance_to_Wall += 0.1f;

                int nTestx = (int)(fPlayerX + fEye_x * fDistance_to_Wall);
                int nTesty = (int)(fPlayerY + fEye_Y * fDistance_to_Wall);


                // Test if ray is out of bounds
                if (nTestx < 0 || nTestx >= nMapWidth || nTesty < 0 || nTesty >= nMapHeight)
                {
                    hit_wall = true;
                    fDistance_to_Wall = fDepth;

                }
                else
                {
                    // Ray is within the bounds so test to check if the ray cell is a wall block
                    if (map[nTesty * nMapWidth + nTestx] == '#')
                    {
                        hit_wall = true;

                        vector<pair<float, float>> p; // distance, dot

                        for (int tx = 0; tx < 2; tx++)
                        {
                            for (int ty = 0; ty < 2; ty++)
                            {
                                float vy = (float)nTesty + ty - fPlayerY;
                                float vx = (float)nTestx + tx - fPlayerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (fEye_x * vx / d) + (fEye_Y * vy / d);
                                p.push_back(make_pair(d, dot));
                            }

                            // Sorting Pairs from closest to the farthest
                            sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first; });

                            float bound = 0.01;
                            if (acos(p.at(0).second) < bound) boundary = true;
                            /*if (acos(p.at(1).second) < bound) boundary = true;*/

                        }

                    }
                }


            }


            // Calculating the distance to ceiling and floor
            int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistance_to_Wall);
            int nFloor = nScreenHeight - nCeiling;

            short nShade = ' ';

            if (fDistance_to_Wall <= fDepth / 4.0f)
                nShade = 0x2588; // Very Close
            else if (fDistance_to_Wall < fDepth / 3.0f)
                nShade = 0x2593;
            else if (fDistance_to_Wall < fDepth / 2.0f)
                nShade = 0x2592;
            else if (fDistance_to_Wall < fDepth)
                nShade = 0x2591;
            else
                nShade = ' '; // Far Away

            if (boundary)
                nShade = ' ';


            for (int y = 0; y < nScreenHeight; y++)
            {
                if (y < nCeiling)
                    screen[y * nScreenWidth + x] = ' ';
                else if(y > nCeiling && y <= nFloor)
                    screen[y * nScreenWidth + x] = nShade;
                else
                {
                    // Shading floor based on distance
                    float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));

                    // Adjusting the characters for floor shading independently
                    if (b < 0.25)
                    {
                        // Keep the floor characters separate from nShade
                        screen[y * nScreenWidth + x] = '.';  
                    }
                    else if (b < 0.5)
                    {
                        screen[y * nScreenWidth + x] = ':';  
                    }
                    else if (b < 0.75)
                    {
                        screen[y * nScreenWidth + x] = ',';  
                    }
                    else if (b < 0.9)
                    {
                        screen[y * nScreenWidth + x] = ';';  
                    }
                    else
                    {
                        screen[y * nScreenWidth + x] = ' ';  // Far away
                    }
                }
            }



        }


        // Display Stats
        const int bufferSize = 50;
        wchar_t buffer[bufferSize];
        swprintf_s(buffer, bufferSize, L"X = %3.2f, Y = %3.2f, A=%3.2f, FPS=%3.2f", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);
        // Display Map
        for (int nx = 0; nx < nMapWidth; nx++)
        {
            for (int ny = 0; ny < nMapWidth; ny++)
            {
                screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
            }
        }

        screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = 'P';


        screen[nScreenWidth * nScreenHeight - 1] = '\0';
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);


    }

    return 0;
}

