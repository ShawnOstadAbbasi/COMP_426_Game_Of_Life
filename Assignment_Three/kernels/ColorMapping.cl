typedef struct {
    float r, g, b;
} Pixel;
__constant Pixel colorMapping[10] = {
    {1.0f, 0.0f, 0.0f},     // 0 = Red
    {0.0f, 1.0f, 0.0f},     // 1 = Green
    {0.0f, 0.0f, 1.0f},     // 2 = Blue
    {1.0f, 1.0f, 0.0f},     // 3 = Yellow
    {0.0f, 1.0f, 1.0f},     // 4 = Cyan
    {1.0f, 0.0f, 1.0f},     // 5 = Magenta
    {1.0f, 0.647f, 0.0f},   // 6 = Orange
    {0.501f, 0.0f, 0.501f}, // 7 = Purple
    {1.0f, 0.752f, 0.796f}, // 8 = Pink
    {1.0f, 1.0f, 1.0f}      // 9 = White
};
__constant Pixel black = {0.0f,0.0f,0.0f};

__kernel void ColorMapping(
    __global char* background,
    __global Pixel* display,
    const int numCols
)
{
    // Get the row and column this work-item will compute
    int row = get_global_id(0);
    int col = get_global_id(1);

    if ((int)background[row * numCols + col] == -1)
        display[row * numCols + col] = black;
    else
        display[row * numCols + col] = colorMapping[(int)background[row * numCols + col]];
}